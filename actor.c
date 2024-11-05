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
_pathStepHost * pathStepHeap;

unsigned char * sectorPathHeap;

unsigned char * pathStackPtr;
unsigned char * pathStackMax;

//Next step ideas:
// 1. "Off a cliff" avoidance
// 2. "Stuck on a Wall" avoidance 
// 3. Implementation of multi-step pathing
// (do I even need it...?)

//Design evaulation:
// Cliff avoidance / Cliff-inclusive pathing
// Need to define a "cliff" ->
// Floor height difference of greater than half the actor's height or no floor at all
// What to do when found a cliff ->
// This is more difficult; the normals involved do not give a useful direction to go.  
// Furthurmore, while we can know what polygon we are on we don't really know which edge we cross to hit the cliff.
// There are no guides present which easily direct the actor away from the cliff.
// We can try probing in a pattern with "exceptions" until we aren't going off the cliff,
// but this has a high chance of the actor accidentally going down the cliff.
// We also really don't want to be generating a bunch of exceptions in rapid succession; bad for performance.
// The only other thing that might work is supposing not only a pathing exception, but that the direction to the cliff is a wall.
// Thus, even if the actor accidentally moves towards the wall, it will "bounce" off of the last path which went towards the wall.
// This helps the actor avoid going down the cliff and might soften the neccessity of rapid path checks.

//Exception cases that need to be caught early:
//1. Pathing to an area that is significantly above the actor
//	There is not likely to be a direct path since these conditions mean a wall is probably present.
//	In such a case, the actor should immediately route by sector guides.
//	Even if the location is in the current sector, we should probably try pathing around other sectors.

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
	
	act->goalSector = broad_phase_sector_finder(act->pathGoal, levelPos, &sectors[target_sector]);
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
	act->pathingLatch = 1;
	act->exceptionTimer = 0;
	act->curSector = spawner->curSector;
	act->curPathStep = INVALID_SECTOR;
	act->exceptionStep = INVALID_SECTOR;
	
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
				
				act->info.flags.losTarget = actorCheckPathOK(act, act->pathUV);
				if(!act->info.flags.losTarget)
				{
					findPathTo(act->goalSector, i);
				}
				
				nbg_sprintf(20, 16, "cur(%i)", act->curSector);
				nbg_sprintf(20, 17, "gol(%i)", act->goalSector);
				nbg_sprintf(20, 15, "nodes(%i)", pathing->count[act->curSector][act->goalSector]);
				
				// nbg_sprintf_decimal(3, 10, act->pathTarget[X]);                     
				// nbg_sprintf_decimal(3, 11, act->pathTarget[Y]);                       
				// nbg_sprintf_decimal(3, 12, act->pathTarget[Z]);
				// nbg_sprintf_decimal(3, 13, fxdot(act->pathUV, act->pathUV));
				
				// nbg_sprintf(5, 10, "los(%i)", act->info.flags.losTarget);

				//nbg_sprintf_decimal(5, 12, act->pos[X]);
				//nbg_sprintf_decimal(5, 13, act->pos[Y]);
				//nbg_sprintf_decimal(5, 14, act->pos[Z]);
				
				act->totalFriction = 32768;
				
				checkInPathSteps(i);
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

