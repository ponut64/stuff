//actor.c
//This file compiled with physobjet.c

#include <sl_def.h>
#include "def.h"
#include "pcmsys.h"
#include "sound.h"
#include "mymath.h"
#include "render.h"
#include "mloader.h"

#include "collision.h"
#include "particle.h"
#include "input.h"

#include "physobjet.h"

_actor spawned_actors[MAX_PHYS_PROXY];

_lineTable cur_actor_line_table;

unsigned char * adjacentPolyHeap;
unsigned char * pathTableHeap;

unsigned char * adjPolyStackPtr;
unsigned char * adjPolyStackMax;
/*

Navigation via floor polygons as the basis for pathing and guiding will probably work,
but the core computational fault is the complexity of building paths this way.
Short-hand path guides need to be made which guide an actor through a sector.

These don't have to work all of the time; it does not need to be perfect.
It only needs to work most of the time.

The more important part is understanding how to use these path nodes to have an actor follow a short, logical path towards a goal.
The primary method is to:
1 - Actor checks LOS to target point.
1a. If LOS checks out, walk towards it. No extra steps.
2 - Sector of target is checked.
2a. If in immediately adjacent sector, use guidance points to sector. When in sector, walk towards it. No extra steps.
3 - Now for the forking checking model.
Path tables are made based on the number of adjacent sectors followed through.
If in sector 6 and adjacent to sector 48 and 12, guiding to sector 7, we might:
- Add sector 48 and sector 12 to two separate possible path lists.
- Sector 48 is adjacent to two other sectors.
-- That makes two more lists; sector 48 to 47 and sector 48 to 13.
- Sector 12 is adjacent to two othr sectors.
-- Sector 12 and 13 and Sector 12 and 9.
- There are a total of four lists now.
Repeat that expanding process until one list counts sector 7 as adjacent.
The first list that counts sector 7 as adjacent (in the least sector steps) is considered the valid path, and is followed.

I think this is a much more reasonable system.
The problem is it means I have to go back and reinvent the system such that sectors make and list guidance nodes:
1 - Center
2 - ToSector for each sector

so I need to do different maths.
I need to first sweep each sector: every floor polygon in a sector must be checked if it is adjacent to another given sector or not.
Since we already use logic like this to check for adjacent sectors (used in the PVS), we can just check that part of the PVS for the sectors.

Then, the center of the intersecting edge of each intersecting floor polygon is added as a navigation point to that sector.
Handy to have more guidance points, maybe, probably not.

So we have the following in SECTOR data:

typedef struct {
	int numNodes;
	POINT * nodes;
} _pathNodes;

contained in:
_pathNodes * sectorPathNodes; //Path nodes to other sectors, of size <nbAdjacent>

Okay, we're kind of building the guide tables right now.
But there's something wrong with it. Hmm.

*/


void	buildAdjacentFloorList(unsigned int sector_id)
{
	if(sector_id > MAX_SECTORS || sector_id == INVALID_SECTOR) return;
	_sector * sct = &sectors[sector_id];
	//Step 1: Is this sector populated?
	if(!sct->nbPolygon) return;
	if(!sct->ent) return;
	//Step 2: Is it pointing to a valid entity?
	if(sct->ent->type != MODEL_TYPE_SECTORED) return;
	if(!sct->ent->file_done) return;
	
	GVPLY * mesh = sct->ent->pol;
	//The concept is to generate path nodes that segregate adjacent floors between sectors of the level,
	//such that the path nodes represent a point that can be navigated towards to go from one sector to another.
	//The first problem is finding RAM suitable to allocate for this purpose.

	//Gotta be carefuL: hopefully this doesn't get misaligned.
	//we start over max, stop
	if(adjPolyStackPtr > adjPolyStackMax) return;
	//
	sct->paths = (_pathNodes*)adjPolyStackPtr;
	//To make it simple, we are just going to guess on the number of possible paths being 6.
	adjPolyStackPtr += 6 * sizeof(_pathNodes);
	
	int pass_number = 0;

	int t_plane[4][3];
	int t_center[3] = {0, 0, 0};
	int c_plane[4][3];
	int c_center[3] = {0, 0, 0};
	int vector_to_tc[3];
	int normal_to_tc[3];
	int y_min = 0;
	int y_max = 0;
	int within_span = 0;
	int within_shape_ct = 0;
	int vert_within_shape[4] = {0,0,0,0};
	int guidance_point[3] = {0,0,0};
	
	_sector * sct2;
	//GOTO Label:
	//On the first pass of this loop, we will count up the number of adjacents, then allocate memory based on that number.
	//On the second pass, we will fill that allocated memory with valid data.

	pass_number = 0;
	PASS:
for(int s = 0; s < sct->nbAdjacent; s++)
{
	sct2 = &sectors[sct->pvs[s+1]];
	sct->paths[s].numNodes = 0;
	for(int i = 0; i < sct->nbPolygon; i++)
	{
		int alias = sct->pltbl[i];
		if(mesh->maxtbl[alias] != N_Yn) continue;
		y_min = mesh->pntbl[mesh->pltbl[alias].vertices[0]][Y];
		y_max = mesh->pntbl[mesh->pltbl[alias].vertices[0]][Y];
		t_center[X] = 0;
		t_center[Y] = 0;
		t_center[Z] = 0;
		for(int k = 0; k < 4; k++)
		{
			t_plane[k][X] = mesh->pntbl[mesh->pltbl[alias].vertices[k]][X];
			t_plane[k][Y] = mesh->pntbl[mesh->pltbl[alias].vertices[k]][Y];
			t_plane[k][Z] = mesh->pntbl[mesh->pltbl[alias].vertices[k]][Z];
			t_center[X] += t_plane[k][X];
			t_center[Y] += t_plane[k][Y];
			t_center[Z] += t_plane[k][Z];
			y_min = (t_plane[k][Y] < y_min) ? t_plane[k][Y] : y_min;
			y_max = (t_plane[k][Y] > y_max) ? t_plane[k][Y] : y_max;
		}
		t_center[X] >>=4;
		t_center[Y] >>=4;
		t_center[Z] >>=4;
		
		//Add some margin of error
		//This will be a quarter meter, or (16<<16)
		y_min -= 16<<16;
		y_max += 16<<16;
		
		for(int p = 0; p < sct2->nbPolygon; p++)
		{
			int alias2 = sct2->pltbl[p];
			if(mesh->maxtbl[alias2] != N_Yn) continue;
			
			within_span = 0;
			c_center[X] = 0;
			c_center[Y] = 0;
			c_center[Z] = 0;
			for(int k = 0; k < 4; k++)
			{
				c_plane[k][X] = mesh->pntbl[mesh->pltbl[alias2].vertices[k]][X];
				c_plane[k][Y] = mesh->pntbl[mesh->pltbl[alias2].vertices[k]][Y];
				c_plane[k][Z] = mesh->pntbl[mesh->pltbl[alias2].vertices[k]][Z];
				if((t_plane[k][Y] > y_min) && (t_plane[k][Y] < y_max)) within_span = 1;
				c_center[X] += c_plane[k][X];
				c_center[Y] += c_plane[k][Y];
				c_center[Z] += c_plane[k][Z];
			}
			if(!within_span) continue;
			
			c_center[X] >>=4;
			c_center[Y] >>=4;
			c_center[Z] >>=4;
			
			vector_to_tc[X] = c_center[X] - t_center[X];
			vector_to_tc[Y] = c_center[Y] - t_center[Y];
			vector_to_tc[Z] = c_center[Z] - t_center[Z];
			normal_to_tc[X] = 0;
			normal_to_tc[Y] = 0;
			normal_to_tc[Z] = 0;
			
			accurate_normalize(vector_to_tc, normal_to_tc);
			//Method for detecting adjacency:
			//Hooray, it's the edge-wind test again. We check if two vertices are within the original polygon being tested.
			//If they are, they probably share an edge.
			within_shape_ct = 0;
			for(int k = 0; k < 4; k++)
			{
				//Aggravating Necessity: Push c_plane towards t_plane slightly
				//This is so the chirality test does not fail
				c_plane[k][X] -= normal_to_tc[X];
				c_plane[k][Y] -= normal_to_tc[Y];
				c_plane[k][Z] -= normal_to_tc[Z];
				vert_within_shape[k] = 0;
				//I need to know *exactly* which edge is adjacent.
				//I can do this by checking every vertex, and then seeing which pair is adjacent.
				if(edge_wind_test(t_plane[0], t_plane[1], t_plane[2], t_plane[3], c_plane[k], N_Yn, 12))
				{
					within_shape_ct++;
					vert_within_shape[k] = 1;
				}
			}
			if(within_shape_ct < 2) continue;
			
			if(pass_number == 0)
			{
				sct->paths[s].numNodes++;
				continue;
			}
			
			
			//This logic chunk assigns which edge was adjacent (0->1, 1->2, 2->3, 3->0)
			int fk = 0;
			int sk = 0;
			if(vert_within_shape[0] && vert_within_shape[1])
			{
				fk = 0;
				sk = 1;
			} else if(vert_within_shape[1] && vert_within_shape[2])
			{
				fk = 1;
				sk = 2;
			} else if(vert_within_shape[2] && vert_within_shape[3])
			{
				fk = 2;
				sk = 3;
			} else if(vert_within_shape[3] && vert_within_shape[0])
			{
				fk = 3;
				sk = 0;
			}
			

			if(within_shape_ct)
			{
				//Mark the guidance point of (adjacent_planes) as the center of [FK]->[SK]
				guidance_point[X] = (c_plane[fk][X] + c_plane[sk][X])>>1;
				guidance_point[Y] = (c_plane[fk][Y] + c_plane[sk][Y])>>1;
				guidance_point[Z] = (c_plane[fk][Z] + c_plane[sk][Z])>>1;		
				sct->paths[s].nodes[sct->paths[s].numNodes][X] = guidance_point[X];
				sct->paths[s].nodes[sct->paths[s].numNodes][Y] = guidance_point[Y];
				sct->paths[s].nodes[sct->paths[s].numNodes][Z] = guidance_point[Z];
				//add a direction to the path node - first calculate a vector from guidance pt to center
				vector_to_tc[X] = JO_ABS((t_center[X] - (guidance_point[X]>>2))>>4);
				vector_to_tc[Y] = JO_ABS((t_center[Y] - (guidance_point[Y]>>2))>>4);
				vector_to_tc[Z] = JO_ABS((t_center[Z] - (guidance_point[Z]>>2))>>4);

				accurate_normalize(vector_to_tc, sct->paths[s].dir[sct->paths[s].numNodes]);
				
				sct->paths[s].fromSector = sector_id;
				sct->paths[s].toSector = sct->pvs[s+1];
				
				sct->paths[s].numNodes++;
			}
			
		}
	}

}	

	//We will reach this point on first and second pass.
	//After the first pass (pass_number == 0), we need to allocate memory:
	//according to the number of adjacent floors in each adjacent sector to the sector being tested
	//On second pass (pass_number == 1), we should just be able to exit the function.
	if(pass_number == 0)
	{
		for(int i = 0; i < sct->nbAdjacent; i++)
		{
			sct->paths[i].fromSector = INVALID_SECTOR;
			sct->paths[i].toSector = INVALID_SECTOR;
			sct->paths[i].nodes = (POINT *)adjPolyStackPtr;
			adjPolyStackPtr += sct->paths[i].numNodes * sizeof(POINT);
			sct->paths[i].dir = (VECTOR *)adjPolyStackPtr;
			adjPolyStackPtr += sct->paths[i].numNodes * sizeof(VECTOR);
			//If we've exceeded the memory limit, abort.
			if(adjPolyStackPtr > adjPolyStackMax)
			{
				nbg_sprintf(0,0, "Error: path tables out of memory");
				return;
			}
		}
		pass_number = 1;
		goto PASS;
	}
	
	//nbg_sprintf(3, 10, "tajp(%i)", total_adj_planes);
}

//What do I want here?
//I want a list which contains a sequence of pointers to all path nodes between two given sectors.
//i.e. if you are in <A> going to <B>, there is a pointer at paths[a][b]
// which points to another list of pointers which makes an inclusive list of all path nodes between A and B.
void	reconcile_pathing_lists(void)
{
	_sector * sct;
	
	if(adjPolyStackPtr > adjPolyStackMax)
	{
		nbg_sprintf(0,0, "Error: path tables out of memory");
		return;
	}
	
	//Initialize table
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		for(int k = 0; k < MAX_SECTORS; k++)
		{
			pathing->count[i][k] = 0;
		}
	}
	
	//Count up intersecting paths
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		sct = &sectors[i];
		if(sct->nbAdjacent == 0 || sct->nbVisible == 0) continue;
		
		for(int j = 0; j < sct->nbAdjacent; j++)
		{
			int sctA = sct->paths[j].fromSector;
			int sctB = sct->paths[j].toSector;
			pathing->count[sctA][sctB]+=1;
		}
	}
	
	//Allocate memory for each intersection
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		for(int k = 0; k < MAX_SECTORS; k++)
		{
			if(pathing->count[i][k] != 0)
			{
				if(adjPolyStackPtr > adjPolyStackMax)
				{
					nbg_sprintf(0,0, "Error: path tables out of memory");
					return;
				}
				pathing->guides[i][k] = (_pathNodes**)adjPolyStackPtr;
				adjPolyStackPtr += pathing->count[i][k] * sizeof(void *);
				//Initialize
				for(int f = 0; f < pathing->count[i][k]; f++)
				{
					pathing->guides[i][k][f] = sectors[INVALID_SECTOR].paths;
				}
			}
			pathing->count[i][k] = 0;
		}
	}
	
	//Populate the allocated memory
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		sct = &sectors[i];
		if(sct->nbAdjacent == 0 || sct->nbVisible == 0) continue;
		
		for(int j = 0; j < sct->nbAdjacent; j++)
		{
			int sctA = sct->paths[j].fromSector;
			int sctB = sct->paths[j].toSector;
			pathing->guides[sctA][sctB][pathing->count[sctA][sctB]] = &sct->paths[j];
			pathing->count[sctA][sctB]++;
		}
	}
	
	
}

void	init_pathing_system(void)
{
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		nbg_sprintf(0,0, "Building path tables...");
		buildAdjacentFloorList(i);
	}
	
	reconcile_pathing_lists();
}

int		actorLineOfSight(_actor * act, int * pos)
{
	//Goal:
	//Check line-of-sight from (actor) to (pos)
	//This involves all collision-enabled proxies
	
	static int vector_to_pos[3] = {0,0,0};
	static int normal_to_pos[3] = {0,0,0};
	static int vector_to_hit[3] = {0,0,0};
	static int hit[3] = {0,0,0};
	//static int nHit[3];
	static int hitPly = 0;
	int possibleObstruction = 0;
	
	hit[X] = 0;
	hit[Y] = 0;
	hit[Z] = 0;
	
	vector_to_pos[X] = (pos[X] - act->pos[X])>>4;
	vector_to_pos[Y] = (pos[Y] - act->pos[Y])>>4;
	vector_to_pos[Z] = (pos[Z] - act->pos[Z])>>4;
	
	accurate_normalize(vector_to_pos, normal_to_pos);
	
	//Methods needed:
	//well i have them
	
/* 	for(int c = 0; c < MAX_PHYS_PROXY; c++)
	{
		//nbg_sprintf(0, 0, "(PHYS)"); //Debug ONLY
		if(RBBs[c].status[1] != 'C') continue;
		if(RBBs[c].boxID == act->box->boxID) continue;
		unsigned short edata = dWorldObjects[activeObjects[c]].type.ext_dat;
		unsigned short boxType = edata & (0xF000);
		//Check if object # is a collision-approved type
		switch(boxType)
		{
			case(OBJPOP):
			case(SPAWNER):
			possibleObstruction += hitscan_vector_from_position_box(normal_to_pos, act->pos, hit, nHit, &RBBs[c]);
			break;
			case(ITEM | OBJPOP):
			break;
			case(BUILD | OBJPOP):
			possibleObstruction += hitscan_vector_from_position_building(normal_to_pos, act->pos, hit, &hitPly, &entities[dWorldObjects[activeObjects[c]].type.entity_ID], RBBs[c].pos, NULL);
			break;
			default:
			break;
		}
	} */
	

	//Check sectors for LOS
	_sector * sct = &sectors[act->curSector];
	//Rather than check everything in the sector's PVS for collision,
	//we will only check the sector itself + primary adjacents.
	for(int s = 0; s < (sct->nbAdjacent+1); s++)
	{
		possibleObstruction += hitscan_vector_from_position_building(normal_to_pos, act->pos, hit, &hitPly, sct->ent, levelPos, &sectors[sct->pvs[s]]);
		if(possibleObstruction) break;
		hit[X] = 0;
		hit[Y] = 0;
		hit[Z] = 0;
	}
	
	// nbg_sprintf(5, 10, "this(%i)", hitFloorPly);
	
	// for(int f = 0; f < sct->ent->numFloor; f++)
	// {
		// if(sct->ent->paths[f].id == hitFloorPly)
		// {
			// nbg_sprintf(5, 11, "ad_ct(%i)", sct->ent->paths[f].numGuides);
			
			// for(int g = 0; g < sct->ent->paths[f].numGuides; g++)
			// {
				// nbg_sprintf(5, 13+g, "id(%i)", sct->ent->paths[f].guides[g].floor_id);
			// }
		// }
	// }
	
	
	//What we should have returned now is the closest hit point to the actor (hit).
	//We need to know if (hit) is between (actor) and (pos).
	//What we will do is see if the hit is on the right side of pos using the normal to it.
	//Of course, if there was no hit, there is no obstruction to line-of-sight.
	if(!possibleObstruction)
	{
		return 1;
	}
	
	vector_to_hit[X] = (pos[X] - hit[X]);
	vector_to_hit[Y] = (pos[Y] - hit[Y]);
	vector_to_hit[Z] = (pos[Z] - hit[Z]);
	
	//(1<<16 being used as some tolerance in case the target position is exactly the hit position, as may sometimes happen)
	if(fxdot(vector_to_hit, normal_to_pos) > (1<<16))
	{
		return 0;
	}
	//No obstruction conditions were met; line-of-sight is achieved.
	return 1;
	
}

int	actorCheckPathOK(_actor * act)
{
	//Step 1: Create an arbitrary point some distance in the direction the actor is moving, scaled by velocity.
	static int actorPathProxy[3];
	static int towardsFloor[3] = {0, (1<<16), 0};
	static int floorProxy[3];
	//static int floorNorm[3];
	static int hitFloorPly = 0;
	static int losToProxy = 0;
	
	//(we add the Z axis because that's forward)
	actorPathProxy[X] = act->pos[X] + fxm(act->velocity[X]<<1, time_fixed_scale) + fxm(act->box->radius[Z]<<1, act->pathUV[X]);
	actorPathProxy[Z] = act->pos[Z] + fxm(act->velocity[Z]<<1, time_fixed_scale) + fxm(act->box->radius[Z]<<1, act->pathUV[Z]);
	
	//We are going to do something that in many circumstances we would not want to do.
	//We are going to ignore the Y axis of the path proxy; it will retain the Y axis of the actor.
	//This is a simplification of representing the allowable movement of the actor.
	//You don't want them to think they can path to a point up a vertical wall; the wall should block them,
	//and make them find another path.
	actorPathProxy[Y] = act->pos[Y];

	sprite_prep.info.drawMode = SPRITE_TYPE_BILLBOARD;
	sprite_prep.info.drawOnce = 1;
	sprite_prep.info.mesh = 0;
	sprite_prep.info.sorted = 0;
	static short sprSpan[3] = {10,10,10};
	add_to_sprite_list(actorPathProxy, sprSpan, 'A', 5, sprite_prep, 0, 0);
	
	//Step 2: Check line-of-sight to this point.
	losToProxy = actorLineOfSight(act, actorPathProxy);
	
	//If no line of sight, path is not ok.
	if(!losToProxy) return 0;
	
	//Step 3: Get a floor position/normal.
	int possibleFloor = 0;

	//Check sectors for LOS
	_sector * sct = &sectors[act->curSector];
	//Rather than check everything in the sector's PVS for collision,
	//we will only check the sector itself + primary adjacents.
	for(int s = 0; s < (sct->nbAdjacent+1); s++)
	{
		possibleFloor += hitscan_vector_from_position_building(towardsFloor, actorPathProxy, floorProxy, &hitFloorPly, sct->ent, levelPos, &sectors[sct->pvs[s]]);
		//floorNorm[X] = sct->ent->pol->nmtbl[hitFloorPly][X];
		//floorNorm[Y] = sct->ent->pol->nmtbl[hitFloorPly][Y];
		//floorNorm[Z] = sct->ent->pol->nmtbl[hitFloorPly][Z];
	}
	
	//If there was no possible floor, path is not OK.
	//if(!possibleFloor) return 0;
	

	
	//If the floor height difference is outside the tolerance, path is not OK.
	// I'll have to use some other method to validate whether or not the guidance point is on a floor or not.
	//if(JO_ABS((act->pos[Y] + act->box->radius[Y]) - floorProxy[Y]) > (act->box->radius[Y]>>1)) return 0;
	
	//If this wasn't actually a floor at all, path is not OK.
	//if(floorNorm[Y] > (-32768)) return 0;
	
	//Otherwise, path should be OK.
	return 1;
	
}

//Handler function which will populate the goal position and goal floor ID.
void	actorPopulateGoalInfo(_actor * act, int * goal, int target_sector)
{

	act->pathGoal[X] = goal[X];
	act->pathGoal[Y] = goal[Y];
	act->pathGoal[Z] = goal[Z];
	
	act->pathTarget[X] = goal[X];
	act->pathTarget[Y] = goal[Y];
	act->pathTarget[Z] = goal[Z];
	
	act->goalSector = broad_phase_sector_finder(act->pathGoal, levelPos, &sectors[target_sector]);
}


void	actorMoveToPos(_actor * act, int * target, int rate, int gap)
{
	static int target_dif[3];
	static int dif_norm[3];
	
	target_dif[X] = (target[X] - act->pos[X])>>4;
	target_dif[Y] = (target[Y] - act->pos[Y])>>4;
	target_dif[Z] = (target[Z] - act->pos[Z])>>4;
	
	accurate_normalize(target_dif, dif_norm);
	
	target_dif[X] = JO_ABS(target_dif[X]>>12);
	target_dif[Y] = JO_ABS(target_dif[Y]>>12);
	target_dif[Z] = JO_ABS(target_dif[Z]>>12);
	
	if((target_dif[X] + target_dif[Y] + target_dif[Z]) < gap) return;

	act->dV[X] += fxm(dif_norm[X], rate);
	act->dV[Y] += fxm(dif_norm[Y], rate);
	act->dV[Z] += fxm(dif_norm[Z], rate);
	
}

void	actor_hit_wall(_actor * act, int * wall_norm)
{
	
	int deflectionFactor = -fxdot(act->velocity, wall_norm);
		
	act->dV[X] += fxm(wall_norm[X], deflectionFactor + REBOUND_ELASTICITY);// - (normal[X]>>4);
	act->dV[Y] += fxm(wall_norm[Y], deflectionFactor + REBOUND_ELASTICITY);// - (normal[Y]>>4);
	act->dV[Z] += fxm(wall_norm[Z], deflectionFactor + REBOUND_ELASTICITY);// - (normal[Z]>>4);
	//Small push to secure surface release
	act->pos[X] += wall_norm[X]>>4;
	act->pos[Y] += wall_norm[Y]>>4;
	act->pos[Z] += wall_norm[Z]>>4;
	
	act->info.flags.hitWall = 1;
	
}

void	actor_per_polygon_collision(_actor * act, _lineTable * realTimeAxis, entity_t * ent, int * ent_pos)
{
	//Plan:
	// 1. Edge wind test the center-face with the plane being tested
	// 2. Project the center-face to the plane being tested
	// 3. Test the projected point to see if it is inside the actor
	// 4. If it is inside the actor, conduct collision response.
	// I'm using an axis-aligned box, this has to be simple.
	// From the center-face projected point, I can add:
	// the polygon normal multiplied by the radius of the box
	// But what good does that do?
	// There are two separate problems I am trying to solve:
	// 1. Colliding with large surfaces as they are sloped on the edges of the box
	// 2. Colliding with small surfaces that do not cross any center-face of the box
	// With large surfaces, something like the Minkowski Difference can apply.
	// For colliding with small surfaces like that, it needs every edge projected on the box.
	// We want to project only the faces which are not being tested.
	
	//If the entity is not loaded, cease the test.
	if(ent->file_done != true) return;
	
	GVPLY * mesh = ent->pol;
	_boundBox * box = act->box;
	static int plane_center[3];
	static int plane_points[4][3];
	static int anchor_to_plane[3];
	static int used_normal[3];
	static int potential_hit[3];
	
	for(unsigned int i = 0; i < mesh->nbPolygon; i++)
	{
		
		//////////////////////////////////////////////////////////////
		// Add the position of the mesh to the position of its points
		// PDATA vector space is inverted, so we negate them
		// "Get world-space point position"
		//////////////////////////////////////////////////////////////
		plane_center[X] = 0;
		plane_center[Y] = 0;
		plane_center[Z] = 0;
		for(int u = 0; u < 4; u++)
		{
		plane_points[u][X] = (mesh->pntbl[mesh->pltbl[i].vertices[u]][X] + ent_pos[X] ); 
		plane_points[u][Y] = (mesh->pntbl[mesh->pltbl[i].vertices[u]][Y] + ent_pos[Y] ); 
		plane_points[u][Z] = (mesh->pntbl[mesh->pltbl[i].vertices[u]][Z] + ent_pos[Z] );
		//Add to the plane's center
		plane_center[X] += plane_points[u][X];
		plane_center[Y] += plane_points[u][Y];
		plane_center[Z] += plane_points[u][Z];
		}
		//Divide sum of plane points by 4 to average all the points
		plane_center[X] >>=2;
		plane_center[Y] >>=2;
		plane_center[Z] >>=2;
		
		used_normal[X] = mesh->nmtbl[i][X];
		used_normal[Y] = mesh->nmtbl[i][Y];
		used_normal[Z] = mesh->nmtbl[i][Z];
		
		//Exceptor: if the plane is not single-plane (i.e. must collide on both sides), we need to find which side we're on.
		if(!(mesh->attbl[i].render_data_flags & GV_FLAG_SINGLE))
		{
			anchor_to_plane[X] = (act->pos[X] - plane_center[X])>>16;
			anchor_to_plane[Y] = (act->pos[Y] - plane_center[Y])>>16;
			anchor_to_plane[Z] = (act->pos[Z] - plane_center[Z])>>16;
			
			int anchor_scale = (anchor_to_plane[X] * mesh->nmtbl[i][X]) + (anchor_to_plane[Y] * mesh->nmtbl[i][Y]) + (anchor_to_plane[Z] * mesh->nmtbl[i][Z]);
			
			if(anchor_scale < 0) 
			{
				used_normal[X] = -mesh->nmtbl[i][X];
				used_normal[Y] = -mesh->nmtbl[i][Y];
				used_normal[Z] = -mesh->nmtbl[i][Z];
			}
		}
		
		//Exceptor: if the plane is not physical (no collision), don't try to collide with it.
		if(!(mesh->attbl[i].render_data_flags & GV_FLAG_PHYS)) continue;
		
		switch(mesh->maxtbl[i])
		{
			case(N_Yp):
			//////////////////////////////////////////////////////////////
			// Ceiling branch (treated as wall)
			//////////////////////////////////////////////////////////////
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis->yp0, mesh->maxtbl[i], 12))
			{
				ray_to_plane(box->UVY, act->nextPos, used_normal, plane_center, potential_hit);
				if(isPointonSegment(potential_hit, realTimeAxis->yp0, realTimeAxis->yp1, 16384))
				{
					act->wallPos[X] = potential_hit[X];
					act->wallPos[Y] = potential_hit[Y];
					act->wallPos[Z] = potential_hit[Z];
					actor_hit_wall(act, used_normal);
				}
			}
			break;
			case(N_Yn):
			if(act->info.flags.hitFloor) break;
			//////////////////////////////////////////////////////////////
			// Floor branch
			//////////////////////////////////////////////////////////////
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis->yp0, mesh->maxtbl[i], 12))
			{
				ray_to_plane(box->UVY, act->nextPos, used_normal, plane_center, potential_hit);
				if(isPointonSegment(potential_hit, realTimeAxis->yp0, realTimeAxis->yp1, 16384))
				{	
					act->floorPos[X] = potential_hit[X];
					act->floorPos[Y] = potential_hit[Y];
					act->floorPos[Z] = potential_hit[Z];
					act->info.flags.hitFloor = 1;
				}
			}
			break;
			case(N_Xp):
			case(N_Xn):
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis->xp1, mesh->maxtbl[i], 12))
			{
				ray_to_plane(box->UVX, act->nextPos, used_normal, plane_center, potential_hit);
				if(isPointonSegment(potential_hit, realTimeAxis->xp0, realTimeAxis->xp1, 16384))
				{
					act->wallPos[X] = potential_hit[X];
					act->wallPos[Y] = potential_hit[Y];
					act->wallPos[Z] = potential_hit[Z];
					actor_hit_wall(act, used_normal);
				}
			} else {
				//if(edge_projection_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis, act->box, N_Zp))
				//{
				//	actor_hit_wall(act, used_normal);
				//}
				//
				//if(edge_projection_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis, act->box, N_Zn))
				//{
				//	actor_hit_wall(act, used_normal);
				//}	
			}
			break;
			case(N_Zp):
			case(N_Zn):
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis->zp1, mesh->maxtbl[i], 12))
			{
				ray_to_plane(box->UVZ, act->nextPos, used_normal, plane_center, potential_hit);
				if(isPointonSegment(potential_hit, realTimeAxis->zp0, realTimeAxis->zp1, 16384))
				{
					act->wallPos[X] = potential_hit[X];
					act->wallPos[Y] = potential_hit[Y];
					act->wallPos[Z] = potential_hit[Z];
					actor_hit_wall(act, used_normal);
				}
			} else {
				//if(edge_projection_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis, act->box, N_Xp))
				//{
				//	actor_hit_wall(act, used_normal);
				//}
				//
				//if(edge_projection_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis, act->box, N_Xn))
				//{
				//	actor_hit_wall(act, used_normal);
				//}	
			}
			break;
		}
	
	}
	
}


void	actor_sector_collision(_actor * act, _lineTable * realTimeAxis, _sector * sct, int * ent_pos)
{
	
	if(!sct->nbPolygon) return;
	
	entity_t * ent = sct->ent;
	GVPLY * mesh = ent->pol;
	_boundBox * box = act->box;
	static int plane_center[3];
	static int plane_points[4][3];
	static int anchor_to_plane[3];
	static int used_normal[3];
	static int potential_hit[3];
	
	for(unsigned int i = 0; i < sct->nbPolygon; i++)
	{
		int alias = sct->pltbl[i];
		//////////////////////////////////////////////////////////////
		// Add the position of the mesh to the position of its points
		// PDATA vector space is inverted, so we negate them
		// "Get world-space point position"
		//////////////////////////////////////////////////////////////
		plane_center[X] = 0;
		plane_center[Y] = 0;
		plane_center[Z] = 0;
		for(int u = 0; u < 4; u++)
		{
		plane_points[u][X] = (mesh->pntbl[mesh->pltbl[alias].vertices[u]][X] + ent_pos[X] ); 
		plane_points[u][Y] = (mesh->pntbl[mesh->pltbl[alias].vertices[u]][Y] + ent_pos[Y] ); 
		plane_points[u][Z] = (mesh->pntbl[mesh->pltbl[alias].vertices[u]][Z] + ent_pos[Z] );
		//Add to the plane's center
		plane_center[X] += plane_points[u][X];
		plane_center[Y] += plane_points[u][Y];
		plane_center[Z] += plane_points[u][Z];
		}
		//Divide sum of plane points by 4 to average all the points
		plane_center[X] >>=2;
		plane_center[Y] >>=2;
		plane_center[Z] >>=2;
		
		used_normal[X] = mesh->nmtbl[alias][X];
		used_normal[Y] = mesh->nmtbl[alias][Y];
		used_normal[Z] = mesh->nmtbl[alias][Z];

		
		//Exceptor: if the plane is not single-plane (i.e. must collide on both sides), we need to find which side we're on.
		if(!(mesh->attbl[alias].render_data_flags & GV_FLAG_SINGLE))
		{
			
			anchor_to_plane[X] = (act->pos[X] - plane_center[X])>>16;
			anchor_to_plane[Y] = (act->pos[Y] - plane_center[Y])>>16;
			anchor_to_plane[Z] = (act->pos[Z] - plane_center[Z])>>16;
			int anchor_scale = (anchor_to_plane[X] * used_normal[X]) + (anchor_to_plane[Y] * used_normal[Y]) + (anchor_to_plane[Z] * used_normal[Z]);
			
			if(anchor_scale < 0) 
			{
				used_normal[X] = -mesh->nmtbl[alias][X];
				used_normal[Y] = -mesh->nmtbl[alias][Y];
				used_normal[Z] = -mesh->nmtbl[alias][Z];
			}
		}
		

		
		//Exceptor: if the plane is not physical (no collision), don't try to collide with it.
		if(!(mesh->attbl[alias].render_data_flags & GV_FLAG_PHYS)) continue;
		
		switch(mesh->maxtbl[alias])
		{
			case(N_Yp):
			//////////////////////////////////////////////////////////////
			// Ceiling branch (treated as wall)
			//////////////////////////////////////////////////////////////
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis->yp1, mesh->maxtbl[alias], 12))
			{
				ray_to_plane(box->UVY, act->nextPos, used_normal, plane_center, potential_hit);
				if(isPointonSegment(potential_hit, realTimeAxis->yp0, realTimeAxis->yp1, 16384))
				{
					act->wallPos[X] = potential_hit[X];
					act->wallPos[Y] = potential_hit[Y];
					act->wallPos[Z] = potential_hit[Z];
					actor_hit_wall(act, used_normal);
				}
			}
			break;
			case(N_Yn):
			if(act->info.flags.hitFloor) break;
			//////////////////////////////////////////////////////////////
			// Floor branch
			//////////////////////////////////////////////////////////////
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis->yp0, mesh->maxtbl[alias], 12))
			{
				ray_to_plane(box->UVY, act->nextPos, used_normal, plane_center, potential_hit);
				if(isPointonSegment(potential_hit, realTimeAxis->yp0, realTimeAxis->yp1, 16384))
				{	
					act->floorPos[X] = potential_hit[X];
					act->floorPos[Y] = potential_hit[Y];
					act->floorPos[Z] = potential_hit[Z];
					act->info.flags.hitFloor = 1;
				}
			}
			break;
			case(N_Xp):
			case(N_Xn):
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis->xp1, mesh->maxtbl[alias], 12))
			{
				ray_to_plane(box->UVX, act->nextPos, used_normal, plane_center, potential_hit);
				if(isPointonSegment(potential_hit, realTimeAxis->xp0, realTimeAxis->xp1, 16384))
				{
					act->wallPos[X] = potential_hit[X];
					act->wallPos[Y] = potential_hit[Y];
					act->wallPos[Z] = potential_hit[Z];
					actor_hit_wall(act, used_normal);
				}
			}
			break;
			case(N_Zp):
			case(N_Zn):
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis->zp1, mesh->maxtbl[alias], 12))
			{
				ray_to_plane(box->UVZ, act->nextPos, used_normal, plane_center, potential_hit);
				if(isPointonSegment(potential_hit, realTimeAxis->zp0, realTimeAxis->zp1, 16384))
				{
					act->wallPos[X] = potential_hit[X];
					act->wallPos[Y] = potential_hit[Y];
					act->wallPos[Z] = potential_hit[Z];
					actor_hit_wall(act, used_normal);
				}
			}
			break;
		}
	
	}
	
}

int create_actor_from_spawner(_declaredObject * spawner, int boxID)
{
	//First, check the spawner to make sure it's actually a spawner.
	unsigned short * edata = &spawner->type.ext_dat;
	int actor_number = -1;
	
	if(GET_ETYPE(*edata) != SPAWNER) return actor_number;
	if(*edata & SPAWNER_ACTIVE) return actor_number;
	if(*edata & SPAWNER_DISABLED) return actor_number;
	//Next, check the active actor list to see if there are any open slots.
	for(int i = 0; i < MAX_PHYS_PROXY; i++)
	{
		if(spawned_actors[i].info.flags.active)
		{
			continue;
		} else {
			actor_number = i;
		}
	}
	//In the case where no open slot was found, the actor number will be -1. Exit the function.
	if(actor_number == -1) return actor_number;
	//Otherwise, we will continue setting up the new actor.
	_actor * act = &spawned_actors[actor_number];
	
	act->pos[X] = spawner->pos[X];
	act->pos[Y] = spawner->pos[Y];
	act->pos[Z] = spawner->pos[Z];
	act->nextPos[X] = spawner->pos[X];
	act->nextPos[Y] = spawner->pos[Y];
	act->nextPos[Z] = spawner->pos[Z];
	act->dV[X] = 0;
	act->dV[Y] = 0;
	act->dV[Z] = 0;
	act->velocity[X] = 0;
	act->velocity[Y] = 0;
	act->velocity[Z] = 0;
	act->dirUV[X] = 0;
	act->dirUV[Y] = -(1<<16);
	act->dirUV[Z] = 0;
	act->lifetime = 0;
	act->floorPos[X] = 0;
	act->floorPos[Y] = 0;
	act->floorPos[Z] = 0;
	act->wallPos[X] = 0;
	act->wallPos[Y] = 0;
	act->wallPos[Z] = 0;
	act->rot[X] = 0;
	act->rot[Y] = spawner->rot[Y];
	act->rot[Z] = 0;
	act->dRot[X] = 0;
	act->dRot[Y] = 0;
	act->dRot[Z] = 0;
	act->totalFriction = 0;
	act->curSector = spawner->curSector;
	
	act->pathGoal[X] = 0;
	act->pathGoal[Y] = 0;
	act->pathGoal[Z] = 0;
	act->goalSector = INVALID_SECTOR;
	
	act->spawner = spawner;
	act->entity_ID = spawner->type.entity_ID;
	act->info.flags.active = 1;
	act->info.flags.alive = 1;
	act->info.flags.hitWall = 0;
	act->info.flags.hitFloor = 0;
	act->type = GET_SPAWNED_TYPE(*edata);
	act->boxID = boxID;
	
	act->spawner->type.ext_dat |= SPAWNER_ACTIVE;
	
	switch(act->type)
	{
		case(SPAWNER_T_EXAMPLE):
		act->maxHealth = 1;
		act->health = 1;
		break;
		default:
		break;
	}
	return actor_number;
}

void	manage_actors(void)
{
	//Goal:
	//Check each actor in the actor list
	//If an actor is within a certain distance from the player, mark it active. Else, mark inactive, and continue.
	//If an actor's health is 0, mark it as not alive.
	//If an actor is not alive, mark it as not active.
	//If an actor is alive and active --
	// 1. Add the dV / dRot to velocity / rot, according to the timescale
	// 2. Add the velocity to the position, according to the timescale
	// 2a. Adjudicate the sector (grid) location of the actor
	// 3. Generate valid bounding box / rendering parameters for it
	// 4. Collision test the actor with the types that this type of actor should collide with
	// 5. Instigate the necessary collision response
	
	_actor * act;

	for(int i = 0; i < MAX_PHYS_PROXY; i++)
	{
		
		act = &spawned_actors[i];
		act->prevSector = act->curSector;
		
		if(sectorIsVisible[act->curSector])
		{
			//Mark as active
			act->info.flags.active = 1;
			if(act->health == 0) act->info.flags.alive = 0;
			if(!act->info.flags.alive)
			{
				act->info.flags.active = 0;
				act->spawner->type.ext_dat |= SPAWNER_DISABLED;
				continue;
			}
			
			
			//nbg_sprintf(5, 11, "ac_sct(%i)", act->curSector);
			///////////////////////////////////////////////
			//At this point, an actor should be alive and active.
			///////////////////////////////////////////////
			// Increase the velocity by the velocity change, multiplied by the timescale
			///////////////////////////////////////////////
			act->dV[X] -= fxm(act->velocity[X], act->totalFriction);
			act->dV[Y] -= fxm(act->velocity[Y], act->totalFriction);
			act->dV[Z] -= fxm(act->velocity[Z], act->totalFriction);
			act->velocity[X] += fxm(act->dV[X], time_fixed_scale);
			act->velocity[Y] += fxm(act->dV[Y], time_fixed_scale);
			act->velocity[Z] += fxm(act->dV[Z], time_fixed_scale);
			act->dV[X] = 0;
			act->dV[Y] = 0;
			act->dV[Z] = 0;
			///////////////////////////////////////////////
			// Position change
			///////////////////////////////////////////////
			act->pos[X] += fxm(act->velocity[X], time_fixed_scale);
			act->pos[Y] += fxm(act->velocity[Y], time_fixed_scale);
			act->pos[Z] += fxm(act->velocity[Z], time_fixed_scale);
			act->nextPos[X] = act->pos[X] + fxm(act->velocity[X], time_fixed_scale);
			act->nextPos[Y] = act->pos[Y] + fxm(act->velocity[Y], time_fixed_scale);
			act->nextPos[Z] = act->pos[Z] + fxm(act->velocity[Z], time_fixed_scale);

			///////////////////////////////////////////////
			// Rotation change
			///////////////////////////////////////////////
			act->rot[X] += act->dRot[X] * framerate;
			act->rot[Y] += act->dRot[Y] * framerate;
			act->rot[Z] += act->dRot[Z] * framerate;
			act->dRot[X] = 0;
			act->dRot[Y] = 0;
			act->dRot[Z] = 0;
			
			act->lifetime += delta_time;
			///////////////////////////////////////////////
			// (Rendered) Bound Box Generation
			///////////////////////////////////////////////
			if(objUP < MAX_PHYS_PROXY)
			{
				bound_box_starter.modified_box = &RBBs[objUP];
				bound_box_starter.x_location = act->pos[X];
				bound_box_starter.y_location = act->pos[Y];
				bound_box_starter.z_location = act->pos[Z];
				
				bound_box_starter.x_rotation = act->rot[X];
				bound_box_starter.y_rotation = act->rot[Y];
				bound_box_starter.z_rotation = act->rot[Z];
	
				bound_box_starter.x_radius = act->spawner->type.radius[X]<<16;
				bound_box_starter.y_radius = act->spawner->type.radius[Y]<<16;
				bound_box_starter.z_radius = act->spawner->type.radius[Z]<<16;
				
				RBBs[objUP].velocity[X] = act->velocity[X];
				RBBs[objUP].velocity[Y] = act->velocity[Y];
				RBBs[objUP].velocity[Z] = act->velocity[Z];
				
				makeBoundBox(&bound_box_starter, EULER_OPTION_XZY);
				RBBs[objUP].boxID = act->boxID;
				act->box = &RBBs[objUP];
				////////////////////////////////////////////////////
				//Set the box status. This branch of the logic dictates the box is:
				// 1. Render-able
				// 2. Collidable
				// 3. May or may not emit light
				////////////////////////////////////////////////////
				RBBs[objUP].status[0] = 'R';
				RBBs[objUP].status[1] = 'C';
				RBBs[objUP].status[2] = 'N';
				//This array is meant on a list where iterative searches can find the right object in the entire declared list.
				activeObjects[objUP] = act->boxID;
				//This array is meant as a list where iterative searches find the entity type drawn.
				objPREP[objUP] = act->entity_ID;
				objUP++;
			} else {
				continue;
			}
			////////////////////////////////////////////////////
			//Before we progress to the rules applied to all actors,
			//we need a part here where the behaviors of each actor is implemented.
			//this also includes the collision tests and responses to them.
			////////////////////////////////////////////////////
			// Settle current actor's line table (for collision)
			for(int s = 0; s < 3; s++)
			{
			cur_actor_line_table.xp0[s] = act->pos[s] + act->box->Xplus[s] + fxm(act->velocity[s], time_fixed_scale);
			cur_actor_line_table.xp1[s] = act->pos[s] + act->box->Xneg[s] + fxm(act->velocity[s], time_fixed_scale);
			
			cur_actor_line_table.yp0[s] = act->pos[s] + act->box->Yplus[s] + fxm(act->velocity[s], time_fixed_scale);
			cur_actor_line_table.yp1[s] = act->pos[s] + act->box->Yneg[s] + fxm(act->velocity[s], time_fixed_scale);
			
			cur_actor_line_table.zp0[s] = act->pos[s] + act->box->Zplus[s] + fxm(act->velocity[s], time_fixed_scale);
			cur_actor_line_table.zp1[s] = act->pos[s] + act->box->Zneg[s] + fxm(act->velocity[s], time_fixed_scale);
			}
			if(act->box->status[3] != 'B')
			{
				finalize_collision_proxy(act->box);
				
				accurate_normalize(act->velocity, act->dirUV);
				//The path delta (guidance vector) uses 0 for the Y axis to represent the actor's inability to go up.
				//A more graceful way to do this would be to multiply each input by the actor's traversal limitations.
				int path_delta[3] = {(act->pathTarget[X] - act->pos[X])>>4, 0, (act->pathTarget[Z] - act->pos[Z])>>4};
				accurate_normalize(path_delta, act->pathUV);
				
			}
			//Special note: The collision system is using next-frame position, so the sector system must also use next-frame position.
			act->curSector = broad_phase_sector_finder(cur_actor_line_table.yp1, levelPos, &sectors[act->curSector]);
			//this is going to get very expensive, because we must:
			//you know what, let's use this opportunity to develop the simplified collision system as axis-aligned collision
			for(int c = 0; c < MAX_PHYS_PROXY; c++)
			{
				//nbg_sprintf(0, 0, "(PHYS)"); //Debug ONLY
				if(RBBs[c].status[1] != 'C') continue;
				if(dWorldObjects[activeObjects[c]].curSector != act->curSector) continue;
				unsigned short edata = dWorldObjects[activeObjects[c]].type.ext_dat;
				unsigned short boxType = edata & (0xF000);
				//Check if object # is a collision-approved type
				switch(boxType)
				{
					case(OBJPOP):
					case(SPAWNER):

					break;
					case(ITEM | OBJPOP):

					break;
					case(BUILD | OBJPOP):
					actor_per_polygon_collision(act, &cur_actor_line_table, &entities[dWorldObjects[activeObjects[c]].type.entity_ID], RBBs[c].pos);
					break;
					default:
					break;
				}
			}
			
			//Sector Collision
			_sector * sct = &sectors[act->curSector];
			//Rather than check everything in the sector's PVS for collision,
			//we will only check the sector itself + primary adjacents.
			//for(int s = 0; s < (sct->nbAdjacent+1); s++)
			//{
				actor_sector_collision(act, &cur_actor_line_table, sct, levelPos);
			//}
			
			//Debug
			// act->pathTarget[X] = act->pathGoal[X];
			// act->pathTarget[Y] = act->pathGoal[Y];
			// act->pathTarget[Z] = act->pathGoal[Z];
			
			if(!act->info.flags.hitFloor)
			{
				act->dV[Y] += GRAVITY;
				act->totalFriction = 1024;
			} else {
				act->velocity[Y] = 0;
				
				act->pos[X] = act->floorPos[X];
				act->pos[Y] = act->floorPos[Y] - (act->box->radius[Y] - (1<<16));
				act->pos[Z] = act->floorPos[Z];
				
				act->info.flags.losTarget = actorCheckPathOK(act);
				if(!act->info.flags.losTarget)
				{
					//If we lose valid LOS to target, go to a path node, if there is one
					if(act->goalSector != act->curSector)
					{
						_pathNodes * guide = pathing->guides[act->curSector][act->goalSector][0];
						act->pathTarget[X] = levelPos[X] + guide->nodes[0][X];
						act->pathTarget[Y] = levelPos[Y] + (guide->nodes[0][Y] - act->box->radius[Y]);
						act->pathTarget[Z] = levelPos[Z] + guide->nodes[0][Z];
					}
				}
				
				nbg_sprintf_decimal(3, 10, act->pathTarget[X]);                     
				nbg_sprintf_decimal(3, 11, act->pathTarget[Y]);                       
				nbg_sprintf_decimal(3, 12, act->pathTarget[Z]);
				
				// nbg_sprintf(5, 10, "los(%i)", act->info.flags.losTarget);
		
/* 				nbg_sprintf(5, 8, "ast(%i)", act->curSector);
				nbg_sprintf(5, 9, "ast(%i)", you.curSector);
				nbg_sprintf(5, 10, "guide(%i)", pathing->count[act->curSector][you.curSector]);
				int rcnp = 0;
				for(int g = 0; g < pathing->count[act->curSector][you.curSector]; g++)
				{
					_pathNodes * guide = pathing->guides[act->curSector][you.curSector][g];
					
						// nbg_sprintf_decimal(3, 10+rcnp, node->nodes[j][X]);
						// rcnp++;                        
						// nbg_sprintf_decimal(3, 10+rcnp, node->nodes[j][Y]);
						// rcnp++;                         
						// nbg_sprintf_decimal(3, 10+rcnp, node->nodes[j][Z]);
						// rcnp++;
					nbg_sprintf(3, 12+rcnp, "Ei:(%i)", guide->numNodes);
					rcnp++;
				}	 */	
				
				
					//nbg_sprintf_decimal(5, 12, act->pos[X]);
					//nbg_sprintf_decimal(5, 13, act->pos[Y]);
					//nbg_sprintf_decimal(5, 14, act->pos[Z]);
				
				act->totalFriction = 32768;
				
				if(act->info.flags.losTarget) actorMoveToPos(act, act->pathTarget, 32768, 100);
			}
			act->info.flags.hitFloor = 0;
		} else {
			act->info.flags.active = 0;
		
			if(act->curSector == INVALID_SECTOR)
			{
				act->curSector = broad_phase_sector_finder(act->pos, levelPos, &sectors[act->curSector]);
			}
		
		}
	
	}
	
	
}

