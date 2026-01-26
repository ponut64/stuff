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
#include "draw.h"
#include "physobjet.h"

_actor spawned_actors[MAX_PHYS_PROXY];

_lineTable cur_actor_line_table;
_pathStepHost pathStepHeap;

unsigned char * sectorPathHeap;

unsigned char * pathStackPtr;
unsigned char * pathStackMax;

// Design Evaluation:
// The system allocates memory for one path guide per adjacent sector, generally speaking; sometimes two.
// This is a design flaw; using sector barriers for pathing in addition to rendering will cause problems.
// However, this flaw can be mitigated with designer intervention tools of a different kind.
// The level designer must be able to manually designate path nodes.
// What is likely to be easiest and most helpful to design is a material property which
// designates the center of a polygon to be a path node.
// But what will the pathing system do with these manually placed nodes?
// Surely, some of them could represent additional borders between sectors....?
// But more importantly, they need to represent turning points, essentially; places that guide the AI through obstacles.
// Thus, the role of an intrasector path guide is to provide an alternative to normal pathing exception:
// Upon pathing exception LOS check to target failing, check LOS to an intrasector node.
// If there is LOS to this node, navigate to that node instead of the exception path.
// If there is no LOS to this node, increment node counter, and wait until next exception release check to check again.
// Perhaps the system would optimally order the check by the nodes closest to the desired sector border.
// It also needs to flag intrasector nodes it has already visited on this path so it doesn't go back to a used one...?


//Handler function which will populate the goal position and goal floor ID.
void	actorPopulateGoalInfo(_actor * act, int * goal, int target_sector)
{

	act->pathGoal[X] = goal[X];
	act->pathGoal[Y] = goal[Y];
	act->pathGoal[Z] = goal[Z];
	
	act->pathTarget[X] = goal[X];
	act->pathTarget[Y] = goal[Y];
	act->pathTarget[Z] = goal[Z];
	
	act->curPathStep = 0;
	act->pathingLatch = 0;
	act->atGoal = 0;
	
	act->goalSector = broad_phase_sector_finder(act->pathGoal, levelPos, &sectors[target_sector]);
}

void	get_floor_position_at_sector_center(int sector_number, int * given_center)
{
	//Check sectors for LOS
	_sector * sct = &sectors[sector_number];
	
	int towardsFloor[3] = {0, (1<<16), 0};
	int neg_ctr[3] = {-sct->center_pos[X], -sct->center_pos[Y], -sct->center_pos[Z]};
	int hitFloorPly = 0;
	int possibleFloor = 0;
	//We can't forget to re-set these.
	given_center[X] = -(32765<<16);
	given_center[Y] = -(32765<<16);
	given_center[Z] = -(32765<<16);
	//Rather than check everything in the sector's PVS for collision,
	//we will first check the sector itself + primary adjacents.
	for(int s = 0; s < (sct->nbAdjacent+1); s++)
	{
		possibleFloor += hitscan_vector_from_position_building(towardsFloor, neg_ctr, given_center, &hitFloorPly, sct->ent, levelPos, &sectors[sct->pvs[s]]);
	}
	if(!possibleFloor)
	{
	//	given_center[X] = -sct->center_pos[X];
	//	given_center[Y] = -sct->center_pos[Y];
	//	given_center[Z] = -sct->center_pos[Z];
	} else {
		given_center[X] = given_center[X];
		given_center[Y] = given_center[Y];
		given_center[Z] = given_center[Z];
	}
}

int		actor_per_polygon_collision(_actor * act, _lineTable * realTimeAxis, entity_t * ent, int * ent_pos)
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
	if(ent->file_done != true) return 0;
	//If the actor is not within a reasonable radius of the object, cease the test.
	int cDif[3];
	cDif[X] = (JO_ABS(ent_pos[X] - act->pos[X]) - (act->box->radius[X]<<1))>>16;
	cDif[Y] = (JO_ABS(ent_pos[Y] - act->pos[Y]) - (act->box->radius[Y]<<1))>>16;
	cDif[Z] = (JO_ABS(ent_pos[Z] - act->pos[Z]) - (act->box->radius[Z]<<1))>>16;
	if(cDif[X] > ent->radius[X] || cDif[Y] > ent->radius[Y] || cDif[Z] > ent->radius[Z]) return 0;
	
	spr_sprintf(20, 24, "inbox");
	
	GVPLY * mesh = ent->pol;
	_boundBox * box = act->box;
	static int plane_center[3];
	static int plane_points[4][3];
	static int anchor_to_plane[3];
	static int used_normal[3];
	static int potential_hit[3];
	int surfHit = 0;
	
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
				if(isPointonSegment(potential_hit, realTimeAxis->yp0, realTimeAxis->yp1, 65536))
				{	
					act->floorPos[X] = potential_hit[X];
					act->floorPos[Y] = potential_hit[Y];
					act->floorPos[Z] = potential_hit[Z];
					act->info.flags.hitFloor = 1;
					surfHit = 1;
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
	
	if(surfHit)
	{
		return 1;
	} else {
		return 0;
	}
	
}


int		actor_sector_collision(int actor_id, _lineTable * realTimeAxis, _sector * sct, int * ent_pos)
{
	
	if(!sct->nbPolygon) return 0;
	
	_actor * act = &spawned_actors[actor_id];
	
	entity_t * ent = sct->ent;
	GVPLY * mesh = ent->pol;
	_boundBox * box = act->box;
	static int plane_center[3];
	static int plane_points[4][3];
	static int anchor_to_plane[3];
	static int used_normal[3];
	static int potential_hit[3];
	int surfHit = 0;
	
	for(unsigned int i = 0; i < sct->nbPolygon; i++)
	{
		//Collsion Addendum:
		//With a simplified collision system using unrotating boxes, a collision proxy's center may interact with the wall.
		//In other words, it's possible to use any 90d convex wall edge to jackknife yourself into hammerspace. Not good.
		//The solution is barbaric: make every wall (and... floor, i guess) slightly bigger than it appears.
		//Method: Center the plane points around center, multiply by a fixed amount, add back to plane center, then add world pos.

		int alias = sct->pltbl[i];
		plane_center[X] = 0;
		plane_center[Y] = 0;
		plane_center[Z] = 0;
		for(int u = 0; u < 4; u++)
		{
		plane_points[u][X] = (mesh->pntbl[mesh->pltbl[alias].vertices[u]][X]);
		plane_points[u][Y] = (mesh->pntbl[mesh->pltbl[alias].vertices[u]][Y]); 
		plane_points[u][Z] = (mesh->pntbl[mesh->pltbl[alias].vertices[u]][Z]);
		//Add to the plane's center
		plane_center[X] += plane_points[u][X];
		plane_center[Y] += plane_points[u][Y];
		plane_center[Z] += plane_points[u][Z];
		}
		//Divide sum of plane points by 4 to average all the points
		plane_center[X] >>=2;
		plane_center[Y] >>=2;
		plane_center[Z] >>=2;
		for(int u = 0; u < 4; u++)
		{
			plane_points[u][X] -= plane_center[X];
			plane_points[u][Y] -= plane_center[Y];
			plane_points[u][Z] -= plane_center[Z];
			plane_points[u][X] = fxm(plane_points[u][X], 72089); //(1.1)
			plane_points[u][Y] = fxm(plane_points[u][Y], 72089);
			plane_points[u][Z] = fxm(plane_points[u][Z], 72089);
			plane_points[u][X] += plane_center[X] + ent_pos[X];
			plane_points[u][Y] += plane_center[Y] + ent_pos[Y];
			plane_points[u][Z] += plane_center[Z] + ent_pos[Z];
		}
		
		//Now for the world-space plane center calculation
		plane_center[X] = 0;
		plane_center[Y] = 0;
		plane_center[Z] = 0;
		
		for(int u = 0; u < 4; u++)
		{
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
				if(isPointonSegment(potential_hit, realTimeAxis->yp0, realTimeAxis->yp1, 65536))
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
					surfHit = 1;
					act->box->surfID = INVALID_SECTOR;
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
					if(!act->atGoal && act->goalSector != act->curSector)
					{
						findPathTo(act->goalSector, actor_id);
					}
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
					if(!act->atGoal && act->goalSector != act->curSector)
					{
						findPathTo(act->goalSector, actor_id);
					}
				}
			}
			break;
		}


	
	}
	
	if(surfHit)
	{
		return 1;
	} else {
		return 0;
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
	act->pathingLatch = 1;
	act->exceptionTimer = 0;
	act->curSector = spawner->curSector;
	act->curPathStep = INVALID_SECTOR;
	act->exceptionStep = INVALID_SECTOR;
	
	act->pathGoal[X] = 0;
	act->pathGoal[Y] = 0;
	act->pathGoal[Z] = 0;
	act->atGoal = 1;
	act->aggroTimer = 0;
	act->goalSector = INVALID_SECTOR;
	act->markedSector = INVALID_SECTOR;
	
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

void	actor_per_object_processing(_actor * act)
{
	//Horrid solution, but this is what we will do for now.
	//(ideally would create a linked list of all objects in sector to search though instead of just dummy look all objects)
	int position_difference[3];
	
	int printIdx = 0;
	
	for(unsigned int i = 0; i < objNEW; i++)
	{
		_declaredObject * obj = &dWorldObjects[i];
		if(obj->curSector != act->curSector) continue;
		
		if((obj->type.ext_dat & ETYPE) == LDATA)
		{
		//Most objects between actor<->object should be some kind of collision.
		//There are likely going to be gamestate managers that relate to actors, those may be here too.
		position_difference[X] = JO_ABS(act->pos[X] - obj->pos[X]) - act->box->radius[X];
		position_difference[Y] = JO_ABS(act->pos[Y] - obj->pos[Y]) - act->box->radius[Y];
		position_difference[Z] = JO_ABS(act->pos[Z] - obj->pos[Z]) - act->box->radius[Z];

		
		int ldata_type = obj->type.ext_dat & LDATA_TYPE;
		
		switch(ldata_type)
		{
		case(MOVER_TARGET):
		
		
			// spr_sprintf_decimal(8 + (printIdx * (10 * 8)), 50, position_difference[X]);                     
			// spr_sprintf_decimal(8 + (printIdx * (10 * 8)), 62, position_difference[Y]);                       
			// spr_sprintf_decimal(8 + (printIdx * (10 * 8)), 74, position_difference[Z]);
		
			if(position_difference[X] < (obj->type.radius[X]<<16)
			&& position_difference[Y] < (obj->type.radius[Y]<<16)
			&& position_difference[Z] < (obj->type.radius[Z]<<16)
			//Enabling Booleans
			&& (!(obj->type.ext_dat & OBJPOP)))
			{
				if(((obj->type.ext_dat & MOVER_TARGET_TYPE) != MOVER_TARGET_REMOTE)
				&& ((obj->type.ext_dat & MOVER_TARGET_TYPE) != MOVER_TARGET_CALLBACK))
				{
				spr_sprintf(8 + (printIdx * (10 * 8)), 38, "obj(%i)", i);
					
				unsigned int moverTriggerLink = (obj->more_data & 0xFF00)>>8;
				_declaredObject * dwa = &dWorldObjects[moverTriggerLink];
			
				//In such case where an actor has contacted a mover trigger and it is not a remote trigger,
				//the mover should be triggered.
				dwa->type.ext_dat |= OBJPOP;
				dwa->type.effectTimeCount = 0;
				obj->type.effectTimeCount = 0;
				} else {
				//If we are in the radius but the door/mover is triggerable by a button instead,
				//we have an opportunity to trigger a pathing exception towards the button.
				//The first thing we need to do is check if the actor is colliding with the door (e.g. trying to open it).
				//If it is, we can generate a pathing exception.
				
				}
			}
		break;
		default:
		
		break;
		}
	
		}
		printIdx++;
	}
	
}

void	actor_set_animation_state(_actor * act, int animation_number)
{
	if(!(act->animPriorityQueue & animation_number) && act->animPriorityQueue > animation_number)
	{
		act->animationTimer = -1;
	}
	
	act->animPriorityQueue |= animation_number;
	act->animState |= animation_number;
}

void *	adjudicate_actor_animation_queue(_actor * act)
{
	int shift_count = 0;
	int state_count = 0;
	int reg = act->animPriorityQueue;
	int state = act->animState;
	//In the act of counting out the animation priority queue, we want to clear the animations set on the actor.
	act->animPriorityQueue = 0;
	act->animState = 0;
	animationControl * used_anim = &t_idle_pose;

	if(reg == 0 && state == 0) return (void*)used_anim;
	
	if(reg != 0)
	{
		while((reg & 1) == 0)
		{
			shift_count++;
			reg>>=1;
		}
	}
	if(state != 0)
	{
		while((state & 1) == 0)
		{
			state_count++;
			state>>=1;
		}
	}

	
	switch(shift_count)
	{
	case(0):
	used_anim = &t_dead_anim;//(highest priority)
	break;
	case(1):
	used_anim =	&t_dead_pose; 
	break;
	case(2):
	used_anim =	&t_attack_anim;
	break;
	case(3):
	used_anim =	&t_move_anim;
	break;
	case(4):
	used_anim =	&t_aggro_anim;
	break;
	case(5):
	used_anim =	&t_point_anim;
	break;
	case(6):
	used_anim =	&t_point_pose;
	break;
	case(7):
	used_anim =	&t_look_anim;
	break;
	default:
	break;
	}
	
	act->animationTimer -= (act->animationTimer > 0) ? delta_time : 0;
	
	if(used_anim->reset_enable == 'Y' && act->animationTimer <= (delta_time<<1) && act->animationTimer > 0 && state_count == shift_count)
	{
		switch(shift_count)
		{
		case(0):
		//used_anim = &t_dead_anim;//(highest priority)
		break;
		case(1):
		//used_anim =	&t_dead_pose; 
		break;
		case(2):
		//used_anim =	&t_attack_anim;
		break;
		case(3):
		//used_anim =	&t_move_anim;
		break;
		case(4):
		//used_anim =	&t_aggro_anim;
		break;
		case(5):
		//used_anim =	&t_point_anim;
		act->info.flags.locked = 1;
		break;
		case(6):
		//used_anim =	&t_point_pose;
		break;
		case(7):
		//used_anim =	&t_look_anim;
		break;
		default:
		break;
		}
	}
	//If animation reset is enabled, we will treat this animation as one that must be played through.
	//When starting an animation we need to set the animation timer of the actor to the length of this animation, and then set the flag back.
	//However, we may have set this animation state from this procedure of animation maintenance.
	//We only want to start this course if we have started the animation state on this frame. If we have not, we don't want to start the animation.
	if(used_anim->reset_enable == 'Y' && act->animationTimer <= 0 && state_count == shift_count)
	{
		act->animPriorityQueue |= (1<<shift_count);
		act->animationTimer = used_anim->time;
	} else if(act->animationTimer > 0)
	{
	//In such case where the used animation had reset enabled but the timer is above zero, we only want to set it back for the next frame.
		act->animPriorityQueue |= (1<<shift_count);
	
	}
	
	nbg_sprintf(5, 9, "anim(%x)", act->animPriorityQueue);
	nbg_sprintf(15, 9, "stat(%x)", act->animState);
	nbg_sprintf(5, 10, "tim(%i)", act->animationTimer);
	
	return (void*)used_anim;
	
}

void	actor_threat_evaluation(_actor * act)
{
	//Procedurally speaking, this would ordinarily contain information about how each actor type responds to things that they (are allowed to) see.
	//In this test case, it is always assumed to be evaluating the player.
	
	int * target = you.wpos;
	
	int dist = approximate_distance(target, act->pos);
	
	if(dist < 512<<16)
	{
		act->info.flags.inCombat = 1;
	}
}

void	actor_idle_actions(int actor_id)
{
	_actor * act = &spawned_actors[actor_id];
	//Goal:
	//To manage the idle state of various actor types.
	//Primarily, this will make the actor look for the player both actively and passively.
	static int intersection_pt[3] = {0,0,0};
	static int hit_id = 0;
	int los_blocked = 0;
	//*we're going to use the player's sector, as that what we are judging LOS to/from here
	_sector * sct = &sectors[you.curSector];
	
	//we need to get a vector from actor->player
	int vec_dif[3] = {-(act->pos[X] + you.pos[X])>>4, -(act->pos[Y] + you.pos[Y])>>4, -(act->pos[Z] + you.pos[Z])>>4};
	int vec_norm[3] = {0,0,0};
	
	quick_normalize(vec_dif, vec_norm);
	
	//Must re-set this variable in order for the hitscan function to be able to properly filter the nearest hit
	intersection_pt[X] = 32766<<16;
	intersection_pt[Y] = 32766<<16;
	intersection_pt[Z] = 32766<<16;
	for(int s = 0; s < (sct->nbVisible+1); s++)
	{
		hitscan_vector_from_position_building(vec_norm, act->pos, &intersection_pt[0], &hit_id, sct->ent, levelPos, &sectors[sct->pvs[s]]);
	}
	//(We also need to do line of sight calculations with movers)
	los_blocked = isPointonSegment(intersection_pt, you.wpos, act->pos, 128);
	
	nbg_sprintf(5, 7, "los(%i)", los_blocked);
	
	short sprSpan[3] = {10, 10, 10};
	sprite_prep.info.drawMode = SPRITE_TYPE_BILLBOARD;
	sprite_prep.info.drawOnce = 1;
	sprite_prep.info.mesh = 0;
	sprite_prep.info.sorted = 0;
	add_to_sprite_list(intersection_pt, sprSpan, 'O', 5, sprite_prep, 0, 0);
	
	//So we have an intersection_pt and a determination if line of sight is blocked.
	//Now we need to derive a viewing angle difference via a simple dot product.
	//Positive numbers are co-aligned (so +1 is facing perfectly). Negative is facing away.
	//If the actor is looking, it will see the player in all positive numbers.
	//If the actor is not looking, it will only see the player in numbers > 49152.
	//Note that vertical difference is not accounted for yet. It ought to be.
	
	int vec_dot = fxdot(act->box->UVNZ, vec_norm);

	// nbg_sprintf_decimal(5, 11, vec_norm[X]);
	// nbg_sprintf_decimal(5, 12, vec_norm[Y]);
	// nbg_sprintf_decimal(5, 13, vec_norm[Z]);
	
	if(act->idleActionTimer <= 0)
	{
	act->idleActionTimer += 6<<16;	
	actor_set_animation_state(act, 1<<7);
		
	} else {
		
	act->idleActionTimer -= delta_time;	
	}
	
	//I need to program an event specific to the end of an animation. This would be a function pointer.
	//Concept:
	//AI starts looking at potential enemy target.
	//When AI is "locked" (i.e. looking at it directly), it plays the "Point" animation. "Locked"
	//This animation ends, then puts AI in a state where it will hold the "Point" pose and then runs evaluations on the threat.
	//For our test purposes, it will be a distance check. If you are close, the AI will change to "inCombat".
	if(!los_blocked)
	{
		if(vec_dot > 0)
		{
			actorMoveToPos(act, intersection_pt, 0, act->box->radius[X]>>16);
		}
		
		if(vec_dot > 49152)
		{
			if(!act->info.flags.locked)
			{
				actor_set_animation_state(act, 1<<5);
				
			}
		}
		
		if(act->info.flags.locked)
		{
			//At this point, we would do a threat evaluation.
			actor_set_animation_state(act, 1<<6);
			actor_threat_evaluation(act);
			act->aggroTimer = 10<<16;
		}
		
	} else {
			if(act->aggroTimer <= 0)
			{
			act->info.flags.locked = 0;
			act->markedSector = INVALID_SECTOR;
			}
			act->info.flags.inCombat = 0;
			act->aggroTimer -= delta_time;
	}
	

	static int nolos_timer = 0;
	
	if(act->info.flags.inCombat)
	{
		actor_set_animation_state(act, 1<<4);
		
		//If you are in the same sector as the actor, the actor will go directly to you.
		//If you are NOT in the same sector, guide the actor towards the sector generally.
		//In addition, do not repeat the guidance command unless the player changes sectors.
		if(act->curSector == you.curSector && nolos_timer < 0)
		{
		actorPopulateGoalInfo(act, you.wpos, you.curSector);
		} else if(you.curSector != act->goalSector)
		{
		get_floor_position_at_sector_center(you.curSector, intersection_pt);
			
		intersection_pt[Y] -= act->box->radius[Y];

		actorPopulateGoalInfo(act, intersection_pt, you.curSector);
		nolos_timer = 1<<16;
		}
	} else if(act->aggroTimer > 0 && act->markedSector == INVALID_SECTOR && act->atGoal)
	{
		//In case the aggroTimer is still positive, the actor is still aggro'd onto the player.
		//It should not know where the player is at this point.
		//Rather, it should pick a sector adjacent to the last sector the player was in that the actor is not currently in, and go there.
		//Later I am going to build "Wandering" or "Patrolling" logic so .. that'll be relevant to this.
		//Unlike wandering or patrolling, this will only be evaluated once.
		act->markedSector = act->goalSector;
		
		for(int i = 0; i < sct->nbAdjacent; i++)
		{
			int randal = getRandom();
			int sct_num = sct->pvs[i+1];
			if(sct_num == act->curSector) continue;
			if(i == (sct->nbAdjacent-1) || randal > 32768)
			{
				get_floor_position_at_sector_center(sct_num, intersection_pt);
					
				intersection_pt[Y] -= act->box->radius[Y];
		
				actorPopulateGoalInfo(act, intersection_pt, sct_num);
				break;
			}
		}
	}
	nolos_timer -= delta_time;
	
	// nbg_sprintf(5, 9, "loc(%i)", act->info.flags.locked);
	// nbg_sprintf(15, 9, "sctAt(%x)", act->curSector);
	// nbg_sprintf(5, 10, "sctGl(%i)", act->goalSector);
	// nbg_sprintf(15, 10, "lat(%i)", act->pathingLatch);
	// nbg_sprintf(5, 11, "at(%i)", act->atGoal);
	// nbg_sprintf(15, 11, "hi(%i)", you.timeSinceWallHit);
	// nbg_sprintf_decimal(5, 12, act->aggroTimer);
	
	//To get the player to collide back with an actor,
	//the collision ID needs to be present from the actor's box<->player collision.
	//This is currently reset as flush boxes happens after player collision, but before actor interaction.
	//We need to find a way to store this interaction specifically between the player and a box that is owned by an actor.
	
	
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
				// 7. Belongs to <actor number>
				////////////////////////////////////////////////////
				RBBs[objUP].status[0] = 'R';
				RBBs[objUP].status[1] = 'C';
				RBBs[objUP].status[2] = 'N';
				RBBs[objUP].status[6] = i;
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
				
				act->dirUV[X] = 0;
				act->dirUV[Y] = 0;
				act->dirUV[Z] = 0;
				quick_normalize(act->velocity, act->dirUV);
				//The path delta (guidance vector) uses 0 for the Y axis to represent the actor's inability to go up.
				//A more graceful way to do this would be to multiply each input by the actor's traversal limitations.
				int path_delta[3] = {(act->pathTarget[X] - act->pos[X])>>4, 0, (act->pathTarget[Z] - act->pos[Z])>>4};
				quick_normalize(path_delta, act->pathUV);
				
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
					if(actor_per_polygon_collision(act, &cur_actor_line_table, &entities[dWorldObjects[activeObjects[c]].type.entity_ID], RBBs[c].pos))
					{
						act->box->surfID = RBBs[c].boxID;
					}
					if(act->info.flags.hitWall)
					{
						RBBs[c].collisionID = act->box->boxID;
						act->box->collisionID = RBBs[c].boxID;
					}
					//In the "Build" type, special collision handling will be present for Movers. Atleast, that's intended.
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
				actor_sector_collision(i, &cur_actor_line_table, sct, levelPos);
			//}
			
			actor_per_object_processing(act);
			
			
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
				
				
				if(!act->atGoal)
				{
					act->info.flags.losTarget = actorCheckPathOK(act, act->pathUV);
					if(!act->info.flags.losTarget)
					{
						findPathTo(act->goalSector, i);
						actor_set_animation_state(act, 1);
					}
					actor_set_animation_state(act, 1<<3);
				} else {
					act->goalSector = INVALID_SECTOR;
				}
				// nbg_sprintf(20, 16, "cur(%i)", act->curSector);
				// nbg_sprintf(20, 17, "gol(%i)", act->goalSector);
				//nbg_sprintf(20, 15, "(%i)", act->atGoal);
				//nbg_sprintf(20, 15, "nodes(%i)", pathing->count[act->curSector][act->goalSector]);
				
				// nbg_sprintf_decimal(3, 10, act->pathTarget[X]);                     
				// nbg_sprintf_decimal(3, 11, act->pathTarget[Y]);                       
				// nbg_sprintf_decimal(3, 12, act->pathTarget[Z]);
				
				// nbg_sprintf(3, 10, "x(%i)", act->dirUV[X]);                     
				// nbg_sprintf(3, 11, "y(%i)", act->dirUV[Y]);                       
				// nbg_sprintf(3, 12, "z(%i)", act->dirUV[Z]);
				
				// spr_sprintf_decimal(24, 24, act->dirUV[X]);                     
				// spr_sprintf_decimal(24, 36, act->dirUV[Y]);                       
				// spr_sprintf_decimal(24, 48, act->dirUV[Z]);
				
				// nbg_sprintf_decimal(3, 13, fxdot(act->pathUV, act->pathUV));
				
				// nbg_sprintf(5, 10, "los(%i)", act->info.flags.losTarget);

				//nbg_sprintf_decimal(5, 12, act->pos[X]);
				//nbg_sprintf_decimal(5, 13, act->pos[Y]);
				//nbg_sprintf_decimal(5, 14, act->pos[Z]);
				
				act->totalFriction = 32768;
				
				if(!act->atGoal) checkInPathSteps(i);
				actor_idle_actions(i);
				//Add velocity of surface
				if(act->box->surfID != INVALID_SECTOR)
				{
					_boundBox * on_box = &RBBs[dWorldObjects[act->box->surfID].bbnum];
					act->pos[X] += on_box->velocity[X];
					act->pos[Y] += on_box->velocity[Y];
					act->pos[Z] += on_box->velocity[Z];
					
					// spr_sprintf_decimal(50, 20, on_box->velocity[X]);
					// spr_sprintf_decimal(50, 33, on_box->velocity[Y]);
					// spr_sprintf_decimal(50, 46, on_box->velocity[Z]);
					//spr_sprintf(100, 20, "abn:%i", dWorldObjects[act->box->surfID].bbnum);
				}
				
			}
			
			active_lights[0].pos[X] = -act->pos[X];
			active_lights[0].pos[Y] = -act->pos[Y];
			active_lights[0].pos[Z] = -act->pos[Z];
			
			
			act->box->animation = adjudicate_actor_animation_queue(act);
			//nbg_sprintf(5, 13, "ani(%x)", act->box->animation);
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

