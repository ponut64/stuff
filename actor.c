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
Next steps:
Guiding the actor by a path

What we need to do:
Per frame, iterate a pathing step towards (target)

When starting:
Check pathing (target)
Find out if a straight, unobstructed, floor path exists to the target
If so, just walk towards target.
Also check if the (pathing condition) is met. If so, stop pathing.
"Pathing Conditions" might be:
Within min/max distance, in line of sight, without line of sight, etc.

If a straight path does not exist, we need to start building a path table.

However, I'd like to get to building a path table, but I have bigger problems.

The first problem is that I need to determine *if* i need a path table, and this is a complex problem.

I have to be careful about the placement of the target. If it is directly on the floor, there could be issues.

Anyway, first is checking what floor sector the target is at.
Then you check if you have line of sight to the target, and if you can path to that floor sector without going off/in a wall.

The second question (of whether you can even reach it) is simplest to be calculated procedurally.
Just walk towards it, and see if **on the NEXT frame** you will hit a wall or walk off of a floor.

It is probably easier to raycast for walls, but checking if walking off a floor is harder.
A unified solution is to just check a single point in the direction towards the target, and then see
if the segment between that point and the actor crosses a wall, and if the floor height beneath that point is good.

check target
walk towards target
if about to hit wall or walk off floor, STOP, path back to current floor center
start building path trees (while still moving)
path tree is built by right-hand rule, very simple for now
go to first step in path tree 

if about to hit wall or walk off floor, STOP, path back to current floor center
path to guidance point towards that floor
once destination is reached, path to next floor center
if about to hit wall or walk off floor, STOP, exception - maybe don't even test for this and keep walking

once next floor center is reached, this step is passed, next step

needed functions:
actor line of sight
actor check movement state OK

path table 1 step
maintain path table
etc?

1 ->
Do we proceed with building global path tables, or not?
Is it only important to navigate TO a sector, rather than within a sector?
-> Navigation can be simple, as long as the level adheres to rules.
-> Sectors can have pathGuides which actors can follow to enter/exit the sector.
-> Within a sector, an actor can:
	a. Move to destination normally, if it is within the sector
	b. Move to a pathGuide of the sector, which might be the center of the sector, or points which guide to other sectors
	
This idea fails on L or U shaped sectors; basically, any situation where the actor might walk off the floor into the abyss or into a wall.
I know it generates a shitload of data, but the old system which simply checked adjacent floors seems objectively superior.
The issue might be the amount of data it generates.

Moreover, a best solution is one which sectors have pathGuides in and out, for faster actor navigation; this is yet MORE data,
because in this scenario the sector and every floor has guidance.

I see that arguing over this is pointless right now. I need to establish a functional pathing system, then understand its failings,
and work from there.

Okay. I should start with a simple function that checks the navigation point for the actor.
It will check if it is on the same floor as the actor, is the first check. If it is, go straight to it.
If it is not, well, we can start attempting navigation.
*/

int		actorLineOfSight(_actor * act, int * pos)
{
	//Goal:
	//Check line-of-sight from (actor) to (pos)
	//This involves all collision-enabled proxies
	
	static int vector_to_pos[3] = {0,0,0};
	static int normal_to_pos[3] = {0,0,0};
	static int vector_to_hit[3] = {0,0,0};
	static int hit[3] = {0,0,0};
	static int nHit[3];
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
	
	for(int c = 0; c < MAX_PHYS_PROXY; c++)
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
	}
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
	static int floorNorm[3];
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
	for(int c = 0; c < MAX_PHYS_PROXY; c++)
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
			//possibleFloor += hitscan_vector_from_position_box(towardsFloor, actorPathProxy, floorProxy, floorNorm, &RBBs[c]);
			break;
			case(ITEM | OBJPOP):
			break;
			case(BUILD | OBJPOP):
			possibleFloor += hitscan_vector_from_position_building(towardsFloor, actorPathProxy, floorProxy, &hitFloorPly, &entities[dWorldObjects[activeObjects[c]].type.entity_ID], RBBs[c].pos, NULL);
			floorNorm[X] = entities[dWorldObjects[activeObjects[c]].type.entity_ID].pol->nmtbl[hitFloorPly][X];
			floorNorm[Y] = entities[dWorldObjects[activeObjects[c]].type.entity_ID].pol->nmtbl[hitFloorPly][Y];
			floorNorm[Z] = entities[dWorldObjects[activeObjects[c]].type.entity_ID].pol->nmtbl[hitFloorPly][Z];
			break;
			default:
			break;
		}
	}
	
	//Check sectors for LOS
	_sector * sct = &sectors[act->curSector];
	//Rather than check everything in the sector's PVS for collision,
	//we will only check the sector itself + primary adjacents.
	for(int s = 0; s < (sct->nbAdjacent+1); s++)
	{
		possibleFloor += hitscan_vector_from_position_building(towardsFloor, actorPathProxy, floorProxy, &hitFloorPly, sct->ent, levelPos, &sectors[sct->pvs[s]]);
		floorNorm[X] = sct->ent->pol->nmtbl[hitFloorPly][X];
		floorNorm[Y] = sct->ent->pol->nmtbl[hitFloorPly][Y];
		floorNorm[Z] = sct->ent->pol->nmtbl[hitFloorPly][Z];
	}
	
	//If there was no possible floor, path is not OK.
	if(!possibleFloor) return 0;
	
	//If the floor height difference is outside the tolerance, path is not OK.
	if(JO_ABS((act->pos[Y] + act->box->radius[Y]) - floorProxy[Y]) > (act->box->radius[Y]>>1)) return 0;
	
	//If this wasn't actually a floor at all, path is not OK.
	if(floorNorm[Y] > (-32768)) return 0;
	
	//Otherwise, path should be OK.
	return 1;
	
}


void	buildAdjacentFloorList(int entity_id)
{
	//Step 1: Is this a valid entity type?
	if(entities[entity_id].type != MODEL_TYPE_SECTORED) return;
	if(!entities[entity_id].file_done) return;
	
	GVPLY * mesh = entities[entity_id].pol;
	entity_t * ent = &entities[entity_id];
	//The concept is to build an adjacent plane list for each polygon of the mesh.
	//The first problem is finding RAM suitable to allocate for this purpose.

	//Gotta be carefuL: hopefully this doesn't get misaligned.
	//we start over max, stop
	if(adjPolyStackPtr > adjPolyStackMax) return;
	//
	ent->pathGuides = (_pathGuide*)adjPolyStackPtr;
	adjPolyStackPtr += (mesh->nbPolygon * sizeof(_pathGuide));
	//we over max, stop
	if(adjPolyStackPtr > adjPolyStackMax) return;
	//Initialize the path guides
	for(unsigned int i = 0; i < mesh->nbPolygon; i++)
	{
		ent->pathGuides[i].adjacent_plane_id[0] = INVALID_PLANE;
		ent->pathGuides[i].adjacent_plane_id[1] = INVALID_PLANE;
		ent->pathGuides[i].adjacent_plane_id[2] = INVALID_PLANE;
		ent->pathGuides[i].adjacent_plane_id[3] = INVALID_PLANE;
	}
	int t_plane[4][3];
	int t_center[3] = {0, 0, 0};
	int c_plane[4][3];
	int c_center[3] = {0, 0, 0};
	int vector_to_tc[3];
	int normal_to_tc[3];
	int y_min = 0;
	int y_max = 0;
	int within_span = 0;
	unsigned int cur_plane_num = 0;
	int within_shape_ct = 0;
	int vert_within_shape[4] = {0,0,0,0};
	int adjacent_planes = 0;
	
	//int total_adj_planes = 0;

	for(unsigned int i = 0; i < mesh->nbPolygon; i++)
	{
		if(mesh->maxtbl[i] != N_Yn) continue;
		if(!(mesh->attbl[i].render_data_flags & GV_FLAG_SINGLE)) continue;
		
		y_min = mesh->pntbl[mesh->pltbl[i].vertices[0]][Y];
		y_max = mesh->pntbl[mesh->pltbl[i].vertices[0]][Y];
		for(int k = 0; k < 4; k++)
		{
			t_plane[k][X] = mesh->pntbl[mesh->pltbl[i].vertices[k]][X];
			t_plane[k][Y] = mesh->pntbl[mesh->pltbl[i].vertices[k]][Y];
			t_plane[k][Z] = mesh->pntbl[mesh->pltbl[i].vertices[k]][Z];
			t_center[X] += t_plane[k][X];
			t_center[Y] += t_plane[k][Y];
			t_center[Z] += t_plane[k][Z];
			y_min = (t_plane[k][Y] < y_min) ? t_plane[k][Y] : y_min;
			y_max = (t_plane[k][Y] > y_max) ? t_plane[k][Y] : y_max;
		}
		t_center[X] >>=2;
		t_center[Y] >>=2;
		t_center[Z] >>=2;
		cur_plane_num = i;
		
		//Add some margin of error
		y_min -= 1<<16;
		y_max += 1<<16;
		
		adjacent_planes = 0;
		for(unsigned int p = 0; p < mesh->nbPolygon; p++)
		{
			if(p == cur_plane_num) continue;
			if(mesh->maxtbl[p] != N_Yn) continue;
			if(!(mesh->attbl[p].render_data_flags & GV_FLAG_SINGLE)) continue;
			within_span = 0;
			for(int k = 0; k < 4; k++)
			{
				c_plane[k][X] = mesh->pntbl[mesh->pltbl[p].vertices[k]][X];
				c_plane[k][Y] = mesh->pntbl[mesh->pltbl[p].vertices[k]][Y];
				c_plane[k][Z] = mesh->pntbl[mesh->pltbl[p].vertices[k]][Z];
				if((t_plane[k][Y] > y_min) && (t_plane[k][Y] < y_max)) within_span = 1;
				c_center[X] += c_plane[k][X];
				c_center[Y] += c_plane[k][Y];
				c_center[Z] += c_plane[k][Z];
			}
			if(!within_span) continue;
			
			c_center[X] >>=2;
			c_center[Y] >>=2;
			c_center[Z] >>=2;
			
			vector_to_tc[X] = t_center[X] - c_center[X];
			vector_to_tc[Y] = t_center[Y] - c_center[Y];
			vector_to_tc[Z] = t_center[Z] - c_center[Z];
			
			accurate_normalize(vector_to_tc, normal_to_tc);
			//Method for detecting adjacency:
			//Hooray, it's the edge-wind test again. We check if two vertices are within the original polygon being tested.
			//If they are, they probably share an edge.
			within_shape_ct = 0;
			for(int k = 0; k < 4; k++)
			{
				//Aggravating Necessity: Push c_plane towards t_plane slightly
				//This is so the chirality test does not fail
				//oops, doesn't work :)
				//c_plane[k][X] += normal_to_tc[X]<<1;
				//c_plane[k][Y] += normal_to_tc[Y]<<1;
				//c_plane[k][Z] += normal_to_tc[Z]<<1;
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
			
			//So we think these floor polygons are adjacent.
			//First, state that 'p' (the other polygon we are testing) is adjacent to the current plane being tested for.
			//Safety check: if the polygon we are testing adjacency to was already registered as adjacent, don't register it again.
			for(int k = 0; k < 4; k++)
			{
				if(ent->pathGuides[cur_plane_num].adjacent_plane_id[k] == p)
				{
					within_shape_ct = 0;
					break;
				}
			}
			if(within_shape_ct)
			{
				ent->pathGuides[cur_plane_num].adjacent_plane_id[adjacent_planes] = p;
				
				//Mark the guidance point of (adjacent_planes) as the center of [FK]->[SK]
				ent->pathGuides[cur_plane_num].guidance_points[adjacent_planes][X] = (c_plane[fk][X] + c_plane[sk][X])>>1;
				ent->pathGuides[cur_plane_num].guidance_points[adjacent_planes][Y] = (c_plane[fk][Y] + c_plane[sk][Y])>>1;
				ent->pathGuides[cur_plane_num].guidance_points[adjacent_planes][Z] = (c_plane[fk][Z] + c_plane[sk][Z])>>1;		
			}
			//Second, we also need to tell 'p' that it is adjacent to the current plane.
			//We don't know if we have tested it yet, so we need to look through its plane list to see if there is an empty spot.
			for(int k = 0; k < 4; k++)
			{
				if(ent->pathGuides[p].adjacent_plane_id[k] == cur_plane_num) break;
				if(ent->pathGuides[p].adjacent_plane_id[k] == INVALID_PLANE)
				{
					ent->pathGuides[p].adjacent_plane_id[k] = cur_plane_num;
					
					//Mark the guidance point of (adjacent_planes) as the center of [FK]->[SK]
					//This is a valid guidance point for both polygons, because it is the center of the gateway edge.
					ent->pathGuides[p].guidance_points[k][X] = (c_plane[fk][X] + c_plane[sk][X])>>1;
					ent->pathGuides[p].guidance_points[k][Y] = (c_plane[fk][Y] + c_plane[sk][Y])>>1;
					ent->pathGuides[p].guidance_points[k][Z] = (c_plane[fk][Z] + c_plane[sk][Z])>>1;		
					break;
					//total_adj_planes++;
				}
			}
			
			adjacent_planes++;
			//total_adj_planes++;
			if(adjacent_planes >= 4) break;
		}
	}
	//nbg_sprintf(3, 10, "tajp(%i)", total_adj_planes);
}

void	init_pathing_system(void)
{
	for(int i = 0; i < MAX_MODELS; i++)
	{
		nbg_sprintf(0,0, "Building path tables...");
		buildAdjacentFloorList(i);
	}
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
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis->yp0, mesh->maxtbl[alias], 12))
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
					
					nbg_sprintf(5, 12, "ad0(%i)", ent->pathGuides[alias].adjacent_plane_id[0]);
					nbg_sprintf(5, 13, "ad1(%i)", ent->pathGuides[alias].adjacent_plane_id[1]);
					nbg_sprintf(5, 14, "ad2(%i)", ent->pathGuides[alias].adjacent_plane_id[2]);
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

void	manage_actors(int * ppos)
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
			
			act->curSector = broad_phase_sector_finder(act->pos, levelPos, &sectors[act->curSector]);
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
			act->pathTarget[X] = you.guidePos[X];
			act->pathTarget[Y] = you.guidePos[Y];
			act->pathTarget[Z] = you.guidePos[Z];
			
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
				
					//nbg_sprintf(5, 10, "los(%i)", act->info.flags.losTarget);
				
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

