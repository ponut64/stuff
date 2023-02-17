//object_col.c
//contains per polygon collision work
#include <sl_def.h>
#include "def.h"
#include "mymath.h"
#include "render.h"
#include "player_phy.h"
#include "collision.h"
#include "physobjet.h"

#include "object_col.h"

#define MATH_TOLERANCE (16384)

int prntidx = 0;

// Purpose:
// Generate a pre-rotated permutation of a mesh for use in building-type rendering / collision.
// Efficient? No. Totally necessary? No. But easier to do this, than reconfigure those other code paths.
void generate_rotated_entity_for_object(short declared_object_entry)
{
	///////////////////////////////////////////////////////
	/*
	Steps:
	1: Identify which entity ID this is
	A. Sanity check: Does this entity already exist with this rotation? If so, use that entity, and abort.
	2: Identify which entity ID we should use (...uh oh)
	B. Sanity check: Are there any available entity IDs? If not, abort.
	3: Create a new entity with the identical parameters to the type of the object
	4: Identify a new, usable, safe place in HWAM to use
	5: Copy the pntbl and pltbl of the mesh to this new place in HWRAM
	6: Adjust the pointers of the new entity to point to this new data
	7: Generate bound box (matrix) parameters from the rotation of the object
	8: Matrix transform all points of the new entity by this matrix
	9: Matrix transform all normals of the new entity by this matrix	
	*/
	///////////////////////////////////////////////////////
	

	//Sanity Checks
	_declaredObject * object = &dWorldObjects[declared_object_entry];
	short this_entity_ID = object->type.entity_ID;
	short new_entity_ID = -1;
	
	//First, we should not check further if the declared object we are looking at has a number higher than the current one.
	//The reason being that objects declared later than this declaration could not possibly have rotated meshes generated yet.
	for(int i = 0; i < declared_object_entry; i++)
	{
		if((dWorldObjects[i].type.ext_dat & OTYPE) == BUILD)
		{
			short other_entity_id = (dWorldObjects[i].type.ext_dat & 0x0FF0)>>4;
			if(other_entity_id == this_entity_ID && dWorldObjects[i].rot[X] == object->rot[X]
			&& dWorldObjects[i].rot[Y] == object->rot[Y] && dWorldObjects[i].rot[Z] == object->rot[Z])
			{
				//In this case, the two objects match rotation and thus should use the same entity ID.
				//No additional work is needed.
				object->type.entity_ID = dWorldObjects[i].type.entity_ID;
				return;
			}
		}
	}
	
	//In this sanity-check, we look and see if there are any available entity slots.
	//An entity slot will be full if "file_done" is Boolean "true" (has any data).
	//In addition, this also picks the slot we should use.
	for(int i = 0; i < MAX_MODELS; i++)
	{
		if(!entities[i].file_done)
		{
			new_entity_ID = i;
			break;
		}
	}
	//If there were no entity slots available, the new entity ID will not be set.
	//It will remain -1. In this case, we will abort.
	if(new_entity_ID == -1) return;
	
	/////////////////////////////////////////////////////
	/*
	
	At this point in the code, we have confirmed two things:
	1. This rotation of a building object is unique.
	2. There is an available slot in the entity list.
	
	We have NOT confirmed:
	1. If there is enough memory available for the model
	2. Where we will put the model in memory
	
	We will check #2. However, we won't check #1, at least not in this version.
	#2 will be the "active HWRAM pointer". This pointer is to the HWRAM model data heap.
	
	We now need to set the declared object's type to use the new entity ID number.
	We will also need to copy all other parameters from the old entity_t to the new entity_t, then change the pointers in entity->pol.
	
	*/
	/////////////////////////////////////////////////////
	
	//Set type, and copy entity parameters.
	object->type.entity_ID = new_entity_ID;
	entities[new_entity_ID] = entities[this_entity_ID];
	
	
	//Set a new address for the model drawing header (GVPLY struct)
	GVPLY * oldMesh = entities[this_entity_ID].pol;
	GVPLY * newMesh = (GVPLY *)HWRAM_ldptr;
	// Don't forget to tell the new entity to use the new GVPLY
	entities[new_entity_ID].pol = newMesh;
	// Copying
	newMesh->attbl = oldMesh->attbl;
	newMesh->nbPoint = oldMesh->nbPoint;
	newMesh->nbPolygon = oldMesh->nbPolygon;
	//
	HWRAM_ldptr += sizeof(GVPLY);
	//Set a new memory address for the pntbl & pltbl.
	newMesh->pntbl = (POINT *)HWRAM_ldptr;
	HWRAM_ldptr += sizeof(POINT) * oldMesh->nbPoint;
	newMesh->pltbl = (POLYGON *)HWRAM_ldptr;
	HWRAM_ldptr += sizeof(POLYGON) * oldMesh->nbPolygon;
	//Get the number of points and polygons.
	short numPt = oldMesh->nbPoint;
	short numPly = oldMesh->nbPolygon;
	
	//Make rotation parameters.
	
	bound_box_starter.x_location = 0;
	bound_box_starter.y_location = 0;
	bound_box_starter.z_location = 0;
	
	bound_box_starter.x_rotation = object->rot[X];
	bound_box_starter.y_rotation = object->rot[Y];
	bound_box_starter.z_rotation = object->rot[Z];
	
	bound_box_starter.x_radius = 1<<16;
	bound_box_starter.y_radius = 1<<16;
	bound_box_starter.z_radius = 1<<16;
	
	_boundBox temp_box;
	
	bound_box_starter.modified_box = &temp_box;
	//... Evil function. Evil!! But I love it, so we keep it.
	make2AxisBox(&bound_box_starter);
	
	//Data copying with rotation
	for(int i = 0; i < numPt; i++)
	{
		newMesh->pntbl[i][X] = fxdot(oldMesh->pntbl[i], temp_box.UVX);
		newMesh->pntbl[i][Y] = fxdot(oldMesh->pntbl[i], temp_box.UVY);
		newMesh->pntbl[i][Z] = fxdot(oldMesh->pntbl[i], temp_box.UVZ);
	}
	
	for(int i = 0; i < numPly; i++)
	{
		newMesh->pltbl[i].Vertices[0] = oldMesh->pltbl[i].Vertices[0];
		newMesh->pltbl[i].Vertices[1] = oldMesh->pltbl[i].Vertices[1];
		newMesh->pltbl[i].Vertices[2] = oldMesh->pltbl[i].Vertices[2];
		newMesh->pltbl[i].Vertices[3] = oldMesh->pltbl[i].Vertices[3];
		
		newMesh->pltbl[i].norm[X] = fxdot(oldMesh->pltbl[i].norm, temp_box.UVX);
		newMesh->pltbl[i].norm[Y] = fxdot(oldMesh->pltbl[i].norm, temp_box.UVY);
		newMesh->pltbl[i].norm[Z] = fxdot(oldMesh->pltbl[i].norm, temp_box.UVZ);
	}
	POINT tRadius = {(entities[new_entity_ID].radius[X] + 80)<<16,
	(entities[new_entity_ID].radius[Y])<<16,
	(entities[new_entity_ID].radius[Z] + 80)<<16};
	//Transformer the radius.
	int bigRadius = JO_MAX(JO_ABS((fxdot(tRadius, temp_box.UVZ)>>16)), JO_ABS((fxdot(tRadius, temp_box.UVX)>>16)));
	entities[new_entity_ID].radius[X] = bigRadius;
	entities[new_entity_ID].radius[Y] = JO_ABS((fxdot(tRadius, temp_box.UVY)>>16));
	entities[new_entity_ID].radius[Z] = bigRadius;

	//Done!
	
	// nbg_sprintf(2, 6, "ox(%i)", entities[this_entity_ID].radius[X]);
	// nbg_sprintf(2, 7, "oy(%i)", entities[this_entity_ID].radius[Y]);
	// nbg_sprintf(2, 8, "oz(%i)", entities[this_entity_ID].radius[Z]);
	
	// nbg_sprintf(2, 10, "Nx(%i)", entities[new_entity_ID].radius[X]);
	// nbg_sprintf(2, 11, "Ny(%i)", entities[new_entity_ID].radius[Y]);
	// nbg_sprintf(2, 12, "Nz(%i)", entities[new_entity_ID].radius[Z]);
	
}


int		edge_wind_test(POINT plane_p0, POINT plane_p1, POINT test_pt, int discard)
{
	
	int left = 0;
	int right = 0;
	
	/*
	
	Edge Winding test
	where p is the testing point and plane_p0 and plane_p1 are points of the polygon
	We ought to flatten these somehow.
	
	(y - y0) * (x1 - x0) - (x - x0) * (y1 - y0)
	left = (p[Y] - plane_p0[Y]) * (plane_p1[X] - plane_p0[X])
	
	right = (p[X] - plane_p0[X]) * (plane_p1[Y] - plane_p0[Y])
	
	side = left - right
	
	(this is a cross product)
	if side is > 0, the point is "left" of the winding. If it is < 0, it is "right" of the winding.
	Since all polygons have a known winding, this should work. Just make sure we're inside the winding of all edges.
	In other words - you have a good early-exit condition. If you are outside of one winding, you can exit the test.
	The caveat to the winding solution is that the winding will change depending on the facing of the normal.
	
	This is a 2D solution, but - it'll probably work if I drop the major axis of the normal in all the calculations.
	
	*/
	
	//Triangle exception handling
	// if(plane_p0[X] == plane_p1[X] &&
	// plane_p0[Y] == plane_p1[Y] &&
	// plane_p0[Z] == plane_p1[Z])
	// {
		// return true;
	// }
	
	/////////////////////////////////////////////////////////////////////////////////////////////
	//Using integer math. The precision of fixed point is not required, and this prevents overflows.
	/////////////////////////////////////////////////////////////////////////////////////////////
	if(discard == N_Xp)
	{
		// left = fxm((test_pt[Y] - plane_p0[Y]), (plane_p1[Z] - plane_p0[Z]));
		// right = fxm((test_pt[Z] - plane_p0[Z]), (plane_p1[Y] - plane_p0[Y]));
		left = ((test_pt[Y] - plane_p0[Y])>>12) * ((plane_p1[Z] - plane_p0[Z])>>12);
		right = ((test_pt[Z] - plane_p0[Z])>>12) * ((plane_p1[Y] - plane_p0[Y])>>12);
		//slPrint("Discard X+", slLocate(2, 5));
	} else if(discard == N_Zp)
	{
		// left = fxm((test_pt[X] - plane_p0[X]), (plane_p1[Y] - plane_p0[Y]));
		// right = fxm((test_pt[Y] - plane_p0[Y]), (plane_p1[X] - plane_p0[X]));
		left = ((test_pt[X] - plane_p0[X])>>12) * ((plane_p1[Y] - plane_p0[Y])>>12);
		right = ((test_pt[Y] - plane_p0[Y])>>12) * ((plane_p1[X] - plane_p0[X])>>12);
		//slPrint("Discard Z+", slLocate(2, 5));
	} else if(discard == N_Yn)
	{
		// left = fxm((test_pt[X] - plane_p0[X]), (plane_p1[Z] - plane_p0[Z]));
		// right = fxm((test_pt[Z] - plane_p0[Z]), (plane_p1[X] - plane_p0[X]));
		left = ((test_pt[X] - plane_p0[X])>>12) * ((plane_p1[Z] - plane_p0[Z])>>12);
		right = ((test_pt[Z] - plane_p0[Z])>>12) * ((plane_p1[X] - plane_p0[X])>>12);
		//slPrint("Discard Y+", slLocate(2, 5));
	} else if(discard == N_Xn)
	{
		// right = fxm((test_pt[Y] - plane_p0[Y]), (plane_p1[Z] - plane_p0[Z]));
		// left = fxm((test_pt[Z] - plane_p0[Z]), (plane_p1[Y] - plane_p0[Y]));
		right = ((test_pt[Y] - plane_p0[Y])>>12) * ((plane_p1[Z] - plane_p0[Z])>>12);
		left = ((test_pt[Z] - plane_p0[Z])>>12) * ((plane_p1[Y] - plane_p0[Y])>>12);
		//slPrint("Discard X-", slLocate(2, 5));
	} else if(discard == N_Zn)
	{
		// right = fxm((test_pt[X] - plane_p0[X]), (plane_p1[Y] - plane_p0[Y]));
		// left = fxm((test_pt[Y] - plane_p0[Y]), (plane_p1[X] - plane_p0[X]));
		right = ((test_pt[X] - plane_p0[X])>>12) * ((plane_p1[Y] - plane_p0[Y])>>12);
		left = ((test_pt[Y] - plane_p0[Y])>>12) * ((plane_p1[X] - plane_p0[X])>>12);
		//slPrint("Discard Z-", slLocate(2, 5));
	} else if(discard == N_Yp)
	{
		// right = fxm((test_pt[X] - plane_p0[X]), (plane_p1[Z] - plane_p0[Z]));
		// left = fxm((test_pt[Z] - plane_p0[Z]), (plane_p1[X] - plane_p0[X])); 
		right = ((test_pt[X] - plane_p0[X])>>12) * ((plane_p1[Z] - plane_p0[Z])>>12);
		left = ((test_pt[Z] - plane_p0[Z])>>12) * ((plane_p1[X] - plane_p0[X])>>12);
		//slPrint("Discard Y-", slLocate(2, 5));
	}
	// slPrint("Left:", slLocate(2, 7 + (prntidx * 2)));
	// slPrintFX(left, slLocate(2, 8 + (prntidx * 2)));
	
	// slPrint("Right:", slLocate(18, 7 + (prntidx * 2)));
	// slPrintFX(right, slLocate(18, 8 + (prntidx * 2)));
	//prntidx++;
	return left - right;
}

// Special notice for code observers:
// In rendering functions, prematrix data (which includes the translation data) is used.
// We could have simplified the function arguments here if we used the prematrix data.
// However, this cannot be done, because the prematrix data is entity-by-entity.
// Entities can be rendered multiple times, this means the prematrix data changes on which instance of the entity is rendered.
// This function is running on Master SH2. The rendering functions are running on Slave SH2.
// In this condition, the setting of the prematrix is controlled by the Slave SH2.
// Hitherto, the correct prematrix may not be set for testing collision. The position must be pointed to instead of changing the prematrix.
void	per_poly_collide(entity_t * ent, _boundBox * mover, FIXED * mesh_position)
{
		//If the entity is not loaded, cease the test.
		if(ent->file_done != true) return;
		//If the player has already hit a *different* object, cease the test.
		if(you.hitObject == true) return;

GVPLY * mesh = ent->pol;
static unsigned short testing_planes[128];
static unsigned char backfaced[128];
static unsigned short last_hit_floor = 0;
static entity_t * last_floor_entity = 0;
short total_planes = 0;
POINT discard_vector = {0, 0, 0};
POINT plane_center = {0, 0, 0};
Bool hitY = false;
Bool hitXZ = false;
Bool shadowStruck = false;

	discard_vector[X] = (mesh_position[X] - mover->pos[X]);
	discard_vector[Y] = (mesh_position[Y] - mover->pos[Y]);
	discard_vector[Z] = (mesh_position[Z] - mover->pos[Z]);
	//If the player is farther away from the object than twice its radius, cease the test.
	if(discard_vector[X] > (ent->radius[X]<<1) &&
	discard_vector[Y] > (ent->radius[Y]<<1) &&
	discard_vector[Z] > (ent->radius[Z]<<1)) return;

prntidx = 0;

	/**
	This test can only be performed on un-rotated meshes.
	(DO NOT rotate the mesh!) (Rotate to desired orientation before exporting!)
	Summary:
	First, use a normal-based discard to throw out polygons/planes that face away from where the player is.
	That builds a list of which polygons/planes to test for collision.
	Then, draw a point from every major axis of the player's bounding box to every polygon.
	If no point is inside the box, move on to the next polygon.
	If a point is inside the box, also test that point against the polygon with a 2D edge winding test.
	If it is inside the polygon and inside the player's bounding box, collision has occurred.
	**/
	
	//////////////////////////////////////////////////////////////
	// Normal-based discard of planes
	// If the plane's normal is facing away from where the player is,
	// it will not be put into the pile of planes to collision test.
	// PDATA vector space is inverted, so we negate them
	//////////////////////////////////////////////////////////////
	for(unsigned int dst_poly = 0; dst_poly < mesh->nbPolygon; dst_poly++)
	{
		discard_vector[X] = -(mesh->pntbl[mesh->pltbl[dst_poly].Vertices[0]][X])
		- mover->prevPos[X] - mesh_position[X];
		discard_vector[Y] = -(mesh->pntbl[mesh->pltbl[dst_poly].Vertices[0]][Y])
		- mover->prevPos[Y] - mesh_position[Y];
		discard_vector[Z] = -(mesh->pntbl[mesh->pltbl[dst_poly].Vertices[0]][Z])
		- mover->prevPos[Z] - mesh_position[Z];

		int normal_discard = fxdot(discard_vector, mesh->pltbl[dst_poly].norm);
				
	// slPrint("Discard vector:", slLocate(1, 9));
	// slPrintFX(discard_vector[X], slLocate(2, 10));
	// slPrintFX(discard_vector[Y], slLocate(2, 11));
	// slPrintFX(discard_vector[Z], slLocate(2, 12));
	// slPrint("Dot product:", slLocate(1, 13));
	// slPrintFX(normal_discard, slLocate(2, 14));
			
		/////////
		// Dual-plane handling
		/////////
		if(!(mesh->attbl[dst_poly].render_data_flags & GV_FLAG_SINGLE) && (mesh->attbl[dst_poly].render_data_flags & GV_FLAG_PHYS) && total_planes < 128)
		{
			testing_planes[total_planes] = dst_poly;
			backfaced[total_planes] = (normal_discard >= 0) ? 0 : 1;
			total_planes++;
			continue;
		}
		/////////
		// Single-plane handling
		/////////
		if(normal_discard >= -(5<<16) && (mesh->attbl[dst_poly].render_data_flags & GV_FLAG_PHYS) && total_planes < 128)
		{
			testing_planes[total_planes] = dst_poly;
			backfaced[total_planes] = 0;
			total_planes++;
		}
	}
	discard_vector[X] = 0;
	discard_vector[Y] = 0;
	discard_vector[Z] = 0;
	//nbg_sprintf(1, 15, "Total planes: (%i)", total_planes);
	
	//////////////////////////////////////////////////////////////
	// Add the position of the mover's box centre-faces to the mover's world position
	// "Get world-space point position"
	//////////////////////////////////////////////////////////////
_lineTable moverCFs = {
	.xp0[X] = mover->Xplus[X] + mover->pos[X] - mover->velocity[X],
	.xp0[Y] = mover->Xplus[Y] + mover->pos[Y] - mover->velocity[Y],
	.xp0[Z] = mover->Xplus[Z] + mover->pos[Z] - mover->velocity[Z],
	.xp1[X] = mover->Xneg[X] + mover->pos[X] + mover->velocity[X],
	.xp1[Y] = mover->Xneg[Y] + mover->pos[Y] + mover->velocity[Y],
	.xp1[Z] = mover->Xneg[Z] + mover->pos[Z] + mover->velocity[Z],
	.yp0[X] = mover->Yplus[X] + mover->pos[X] - mover->velocity[X],
	.yp0[Y] = mover->Yplus[Y] + mover->pos[Y] - mover->velocity[Y],
	.yp0[Z] = mover->Yplus[Z] + mover->pos[Z] - mover->velocity[Z],
	.yp1[X] = mover->Yneg[X] + mover->pos[X] + mover->velocity[X],
	.yp1[Y] = mover->Yneg[Y] + mover->pos[Y] + mover->velocity[Y],
	.yp1[Z] = mover->Yneg[Z] + mover->pos[Z] + mover->velocity[Z],
	.zp0[X] = mover->Zplus[X] + mover->pos[X] - mover->velocity[X],
	.zp0[Y] = mover->Zplus[Y] + mover->pos[Y] - mover->velocity[Y],
	.zp0[Z] = mover->Zplus[Z] + mover->pos[Z] - mover->velocity[Z],
	.zp1[X] = mover->Zneg[X] + mover->pos[X] + mover->velocity[X],
	.zp1[Y] = mover->Zneg[Y] + mover->pos[Y] + mover->velocity[Y],
	.zp1[Z] = mover->Zneg[Z] + mover->pos[Z] + mover->velocity[Z]
}; 

POINT plane_points[4];
VECTOR used_normal;
int dominant_axis = N_Yp;
Bool lineChecks[3];
POINT lineEnds[3];

//////////////////////////////////////////////////////////////
//	Shortcut: If we have already collided with a floor plane on the previous frame,
//	we will store the polygon ID of that floor plane.
//	If we are currently determined to be standing on a floor this frame,
//	we sill first collision test with that plane, the last one we were standing on.
//	If we still collide with that floor properly, don't test for any other floor collision.
//////////////////////////////////////////////////////////////
// nbg_sprintf(1, 7, "(%x)", last_floor_entity);
// nbg_sprintf(1, 8, "(%x)", ent);
if(you.hitSurface && last_floor_entity == ent)
{
	//slPrint("Testing Old Floor", slLocate(2, 6));
	
	plane_center[X] = 0;
	plane_center[Y] = 0;
	plane_center[Z] = 0;
	for(int u = 0; u < 4; u++)
	{
	plane_points[u][X] = -(mesh->pntbl[mesh->pltbl[last_hit_floor].Vertices[u]][X]) - mesh_position[X];
	plane_points[u][Y] = -(mesh->pntbl[mesh->pltbl[last_hit_floor].Vertices[u]][Y]) - mesh_position[Y];
	plane_points[u][Z] = -(mesh->pntbl[mesh->pltbl[last_hit_floor].Vertices[u]][Z]) - mesh_position[Z];
	//Add to the plane's center
	plane_center[X] += plane_points[u][X];
	plane_center[Y] += plane_points[u][Y];
	plane_center[Z] += plane_points[u][Z];
	}
	//Divide sum of plane points by 4 to average all the points
	plane_center[X] >>=2;
	plane_center[Y] >>=2;
	plane_center[Z] >>=2;
	//////////////////////////////////////////////////////////////
	// Grab the normal used for point-to-plane projection
	// This also retains the sign, which we use to check if the axis is + or -.
	//////////////////////////////////////////////////////////////
	used_normal[X] = (mesh->pltbl[last_hit_floor].norm[X]);
	used_normal[Y] = (mesh->pltbl[last_hit_floor].norm[Y]);
	used_normal[Z] = (mesh->pltbl[last_hit_floor].norm[Z]);
	//////////////////////////////////////////////////////////////
	// Project the lines to the plane
	// We are only testing the Y axis right now, because we are specifically testing something previously determined to be a floor.
	// Note the second to last argument here: This is the tolerance level for collision.
	// The tolerance level for a collision here is double the normal tolerance for hitting a floor.
	// We prefer to have the player stick to surfaces they are standing on a little more strongly.
	//////////////////////////////////////////////////////////////
	lineChecks[Y] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, plane_center, used_normal, discard_vector, 2<<16, lineEnds[Y]);
	//////////////////////////////////////////////////////////////
	// Proceed with edge wind testing if the line is still in the right spot.
	// Notice there are a few conditions here which are either manually inserted to a fixed value or omitted:
	// 1. We do not check the dominant axis. At this point, we already assume it is N_Yn, since it should be a floor.
	// 2. We do not check the backface of the plane. At this point, we already assume it met the backface condition to be a floor.
	// 3. We do not do any checks against it as if it were a wall. We already know it's supposed to be the floor.
	//////////////////////////////////////////////////////////////
	if(lineChecks[Y])
	{
		if(edge_wind_test(plane_points[0], plane_points[1], lineEnds[Y], N_Yn) >= 0)
		{
			if(edge_wind_test(plane_points[1], plane_points[2], lineEnds[Y], N_Yn) >= 0)
			{
				if(edge_wind_test(plane_points[2], plane_points[3], lineEnds[Y], N_Yn) >= 0)
				{
					if(edge_wind_test(plane_points[3], plane_points[0], lineEnds[Y], N_Yn) >= 0)
					{
						used_normal[X] = -used_normal[X]; 
						used_normal[Y] = -used_normal[Y];
						used_normal[Z] = -used_normal[Z];
						you.floorNorm[X] = used_normal[X]; 
						you.floorNorm[Y] = used_normal[Y];
						you.floorNorm[Z] = used_normal[Z];
						
						standing_surface_alignment(used_normal, you.renderRot);
						
						you.floorPos[X] = ((lineEnds[Y][X]) - (mover->Yneg[X]));
						you.floorPos[Y] = ((lineEnds[Y][Y]) - (mover->Yneg[Y]));
						you.floorPos[Z] = ((lineEnds[Y][Z]) - (mover->Yneg[Z]));
						you.shadowPos[X] = lineEnds[Y][X];
						you.shadowPos[Y] = lineEnds[Y][Y];
						you.shadowPos[Z] = lineEnds[Y][Z];
						
						you.hitSurface = true;
						you.aboveObject = true;
						you.hitObject = true;
						hitY = true; 
					}
				}
			}
		}
	}
//End of early floor test
}


for(int i = 0; i < total_planes; i++)
{
	if(testing_planes[i] == last_hit_floor && last_floor_entity == ent && hitY) continue;
	plane_center[X] = 0;
	plane_center[Y] = 0;
	plane_center[Z] = 0;
	//////////////////////////////////////////////////////////////
	// Add the position of the mesh to the position of its points
	// PDATA vector space is inverted, so we negate them
	// "Get world-space point position"
	//////////////////////////////////////////////////////////////
	for(int u = 0; u < 4; u++)
	{
	plane_points[u][X] = -(mesh->pntbl[mesh->pltbl[testing_planes[i]].Vertices[u]][X]) - mesh_position[X];
	plane_points[u][Y] = -(mesh->pntbl[mesh->pltbl[testing_planes[i]].Vertices[u]][Y]) - mesh_position[Y];
	plane_points[u][Z] = -(mesh->pntbl[mesh->pltbl[testing_planes[i]].Vertices[u]][Z]) - mesh_position[Z];
	//Add to the plane's center
	plane_center[X] += plane_points[u][X];
	plane_center[Y] += plane_points[u][Y];
	plane_center[Z] += plane_points[u][Z];
	}
	//Divide sum of plane points by 4 to average all the points
	plane_center[X] >>=2;
	plane_center[Y] >>=2;
	plane_center[Z] >>=2;

	//////////////////////////////////////////////////////////////
	// Grab the absolute normal used for finding the dominant axis
	//////////////////////////////////////////////////////////////
	used_normal[X] = JO_ABS(mesh->pltbl[testing_planes[i]].norm[X]);
	used_normal[Y] = JO_ABS(mesh->pltbl[testing_planes[i]].norm[Y]);
	used_normal[Z] = JO_ABS(mesh->pltbl[testing_planes[i]].norm[Z]);
	FIXED max_axis = JO_MAX(JO_MAX((used_normal[X]), (used_normal[Y])), (used_normal[Z]));
	dominant_axis = ((used_normal[X]) == max_axis) ? N_Xp : dominant_axis;
	dominant_axis = ((used_normal[Y]) == max_axis) ? N_Yp : dominant_axis;
	dominant_axis = ((used_normal[Z]) == max_axis) ? N_Zp : dominant_axis;
	//////////////////////////////////////////////////////////////
	// Grab the normal used for point-to-plane projection
	// This also retains the sign, which we use to check if the axis is + or -.
	//////////////////////////////////////////////////////////////
	used_normal[X] = (mesh->pltbl[testing_planes[i]].norm[X]);
	used_normal[Y] = (mesh->pltbl[testing_planes[i]].norm[Y]);
	used_normal[Z] = (mesh->pltbl[testing_planes[i]].norm[Z]);

	if(dominant_axis == N_Xp && used_normal[X] < 0) dominant_axis = N_Xn;
	if(dominant_axis == N_Yp && used_normal[Y] < 0) dominant_axis = N_Yn;
	if(dominant_axis == N_Zp && used_normal[Z] < 0) dominant_axis = N_Zn;
	//////////////////////////////////////////////////////////////
	// Project the lines to the plane
	// Y first, then Z, then X
	// We separate the Y axis collision check with the X-Z axis collision checks.
	// The Y axis check is the only one which will plant the player on a floor,
	// so we want to continue checking after the first positive collision check of Y to see if we hit a wall or not.
	// Conversely, if we have hit a wall, it's possible we're hitting the floor on a different plane too.
	//////////////////////////////////////////////////////////////
	if(!hitY)
	{
	lineChecks[Y] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, plane_center, used_normal, discard_vector, 1<<16, lineEnds[Y]);
	} else {
	lineChecks[Y] = false;
	}
	if(!hitXZ)
	{
	lineChecks[Z] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, plane_center, used_normal, discard_vector, 32768, lineEnds[Z]);
	lineChecks[X] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, plane_center, used_normal, discard_vector, 32768, lineEnds[X]);	
	} else {
	lineChecks[Z] = false;
	lineChecks[X] = false;
	}

	//////////////////////////////////////////////////////////////
	// Shadow Posititon
	//////////////////////////////////////////////////////////////
	if((!shadowStruck || !hitY) && (lineEnds[Y][Y] < you.pos[Y]))
	{	
		if(edge_wind_test(plane_points[0], plane_points[1], lineEnds[Y], dominant_axis))
		{
			if(edge_wind_test(plane_points[1], plane_points[2], lineEnds[Y], dominant_axis))
			{
				if(edge_wind_test(plane_points[2], plane_points[3], lineEnds[Y], dominant_axis))
				{
					if(edge_wind_test(plane_points[3], plane_points[0], lineEnds[Y], dominant_axis))
					{
						shadowStruck = true;
						you.aboveObject = true;
						you.shadowPos[X] = lineEnds[Y][X];
						you.shadowPos[Y] = lineEnds[Y][Y];
						you.shadowPos[Z] = lineEnds[Y][Z];
					}
				}
			}
		}
	}
	if(!lineChecks[0] && !lineChecks[1] && !lineChecks[2]) { continue; }
	//////////////////////////////////////////////////////////////
	// If we reach this point, at least one of the lines project to a point that is inside the player's bounding box.
	// Since we know that, we can start with a winding test comparing the point to the plane.
	// When we do this, we will discard the major axis of the normal to make it 2D.
	//////////////////////////////////////////////////////////////
	// If it is a dual-plane which was determined to be otherwise back-facing, negate the normal.
	if(backfaced[i])
	{
		used_normal[X] = -used_normal[X];
		used_normal[Y] = -used_normal[Y];
		used_normal[Z] = -used_normal[Z];
	}
	//////////////////////////////////////////////////////////////
	// Line Checks Y
	//////////////////////////////////////////////////////////////
	if(!hitY)
	{
		if(lineChecks[Y]){
			//slPrint("Testing Y", slLocate(2, 6));
			if(edge_wind_test(plane_points[0], plane_points[1], lineEnds[Y], dominant_axis) >= 0)
			{
				if(edge_wind_test(plane_points[1], plane_points[2], lineEnds[Y], dominant_axis) >= 0)
				{
					if(edge_wind_test(plane_points[2], plane_points[3], lineEnds[Y], dominant_axis) >= 0)
					{
						if(edge_wind_test(plane_points[3], plane_points[0], lineEnds[Y], dominant_axis) >= 0)
						{
							if(dominant_axis == N_Yn && !backfaced[i])
							{
								used_normal[X] = -used_normal[X]; 
								used_normal[Y] = -used_normal[Y];
								used_normal[Z] = -used_normal[Z];
								you.floorNorm[X] = used_normal[X]; 
								you.floorNorm[Y] = used_normal[Y];
								you.floorNorm[Z] = used_normal[Z];
								
								standing_surface_alignment(used_normal, you.renderRot);
								
								you.floorPos[X] = ((lineEnds[Y][X]) - (mover->Yneg[X]));
								you.floorPos[Y] = ((lineEnds[Y][Y]) - (mover->Yneg[Y]));
								you.floorPos[Z] = ((lineEnds[Y][Z]) - (mover->Yneg[Z]));
								you.shadowPos[X] = lineEnds[Y][X];
								you.shadowPos[Y] = lineEnds[Y][Y];
								you.shadowPos[Z] = lineEnds[Y][Z];
								
								you.hitSurface = true;
								last_hit_floor = testing_planes[i];
								last_floor_entity = ent;
							} else {
								you.wallNorm[X] = used_normal[X];
								you.wallNorm[Y] = used_normal[Y];
								you.wallNorm[Z] = used_normal[Z];
								you.wallPos[X] = -lineEnds[Y][X];
								you.wallPos[Y] = -lineEnds[Y][Y];
								you.wallPos[Z] = -lineEnds[Y][Z];
								
								you.hitWall = true;
							}
							you.aboveObject = true;
							you.hitObject = true;
							hitY = true; 
						}
					}
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////
	// Line Checks Z
	//////////////////////////////////////////////////////////////
	if(!hitXZ)
	{
		if(lineChecks[Z]){
			//slPrint("Testing Z", slLocate(2, 6));
			if(edge_wind_test(plane_points[0], plane_points[1], lineEnds[Z], dominant_axis) >= 0)
			{
				if(edge_wind_test(plane_points[1], plane_points[2], lineEnds[Z], dominant_axis) >= 0)
				{
					if(edge_wind_test(plane_points[2], plane_points[3], lineEnds[Z], dominant_axis) >= 0)
					{
						if(edge_wind_test(plane_points[3], plane_points[0], lineEnds[Z], dominant_axis) >= 0)
						{

								you.wallNorm[X] = used_normal[X];
								you.wallNorm[Y] = used_normal[Y];
								you.wallNorm[Z] = used_normal[Z];
								you.wallPos[X] = -lineEnds[Z][X];
								you.wallPos[Y] = -lineEnds[Z][Y];
								you.wallPos[Z] = -lineEnds[Z][Z];
								
								you.hitWall = true;

							you.hitObject = true;
							hitXZ = true;
						}
					}
				}
			}
		}
		
	//////////////////////////////////////////////////////////////
	// Line Checks X
	//////////////////////////////////////////////////////////////
		if(lineChecks[X]){
			//slPrint("Testing X", slLocate(2, 6));
			if(edge_wind_test(plane_points[0], plane_points[1], lineEnds[X], dominant_axis) >= 0)
			{
				if(edge_wind_test(plane_points[1], plane_points[2], lineEnds[X], dominant_axis) >= 0)
				{
					if(edge_wind_test(plane_points[2], plane_points[3], lineEnds[X], dominant_axis) >= 0)
					{
						if(edge_wind_test(plane_points[3], plane_points[0], lineEnds[X], dominant_axis) >= 0)
						{
							
							you.wallNorm[X] = used_normal[X];
							you.wallNorm[Y] = used_normal[Y];
							you.wallNorm[Z] = used_normal[Z];
							you.wallPos[X] = -lineEnds[X][X];
							you.wallPos[Y] = -lineEnds[X][Y];
							you.wallPos[Z] = -lineEnds[X][Z];
								
							you.hitWall = true;
							
							you.hitObject = true;
							hitXZ = true;
						}
					}
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////
	// If we've hit both XZ (a wall) and Y (the floor), stop testing further.
	//////////////////////////////////////////////////////////////
	if(hitXZ && hitY) return;
	//////////////////////////////////////////////////////////////
	// Per testing plane loop end stub
	//////////////////////////////////////////////////////////////
}
	//////////////////////////////////////////////////////////////
	// Per polygon collision function end stub
	//////////////////////////////////////////////////////////////
}


	/*
	
	TEXTURE ID MATRIX:
	1	- full size
	2	- full y, 1/2 x
	3	- full y, 1/4 x
	4	- full y, 1/8 x
	5	- 1/2 y, full x
	6	- 1/2 y, 1/2 x
	7	- 1/2 y, 1/4 x
	8	- 1/2 y, 1/8 x
	9	- 1/4 y, full x
	10	- 1/4 y, 1/2 x
	11	- 1/4 y, 1/4 x
	12	- 1/4 y, 1/8 x
	13	- 1/8 y, full x
	14	- 1/8 y, 1/2 x
	15	- 1/8 y, 1/4 x
	16	- 1/8 y, 1/8 x

	*/

	#define TEXTS_GENERATED_PER_TEXTURE_LOADED (16)
	#define SUBDIVISION_NEAR_PLANE (15<<16)

		POINT	subdivided_points[512];
		short	subdivided_polygons[512][4]; //4 Vertex IDs of the subdivided_points
		short	used_textures[512];
		short	sub_poly_cnt = 0;
		short	sub_vert_cnt = 0;
		char	subdivision_rules[4]	= {'N', 'N', 'N', 'N'};
		short	texture_rules[4]		= {16, 16, 16, 16};
		int		z_rules[4]				= {200<<16, 150<<16, 100<<16, 50<<16};
void	subdivide_plane(short start_point, short overwritten_polygon, short num_divisions, short total_divisions)
{

	//"Load" the original points (code shortening operation)
	FIXED * ptv[4];
	static int max_z;
	static char new_rule;
	for(int u = 0; u < 4; u++)
	{
		ptv[u] = &subdivided_points[subdivided_polygons[overwritten_polygon][u]][X];
	}
	
	//if(ptv[0][Z] < 0 && ptv[1][Z] < 0 && ptv[2][Z] < 0 && ptv[3][Z] < 0) return;
	new_rule = subdivision_rules[total_divisions];

	used_textures[overwritten_polygon] = texture_rules[total_divisions];
	if(num_divisions == 0)
	{
		return;
	}
	//////////////////////////////////////////////////////////////////
	// Warning: There is no extreme polygon smallness exit.
	// It will mess with texture assignment so I got rid of it.
	// Unfortunately I think this needs to come back in some way to handle trapezoids.
	//////////////////////////////////////////////////////////////////
	//if(minLen <= 50 && maxLen <= 50)
	//{
		//return;
	//} 
	//////////////////////////////////////////////////////////////////
	// Quick check: If we are subdividing a polygon above the z level, stop further subdivision.
	// This is mostly useful in cases where a large polygon is being recursively subdivided and parts of it may be far away.
	//////////////////////////////////////////////////////////////////
	int polygon_minimum = JO_MIN(JO_MIN(ptv[0][Z], ptv[1][Z]),  JO_MIN(ptv[2][Z], ptv[3][Z]));
	if(polygon_minimum > z_rules[total_divisions])
	{
		return;
	}

	short tgt_pnt = start_point;
	
	short poly_a = overwritten_polygon; //Polygon A is a polygon ID we will overwrite (replace the original polygon)
	short poly_b = sub_poly_cnt;
	short poly_c = sub_poly_cnt+1;
	short poly_d = sub_poly_cnt+2;
	
	if(new_rule == '+') 
	{
	//////////////////////////////////////////////////////////////////
	// Subdivide by all rules / Subdivide polygon into four new quads
	//Turn 4 points into 9 points
	//Make the 4 new polygons
	//////////////////////////////////////////////////////////////////
	/*
	0A			1A | 0B			1B
							
			A				B
							
	3A			2A | 3B			2B		
	
	0D			1D | 0C			1C
	
			D				C

	3D			2D | 3C			2C
	*/
	// Initial Conditions
	subdivided_polygons[poly_a][0] = subdivided_polygons[overwritten_polygon][0];
	subdivided_polygons[poly_b][1] = subdivided_polygons[overwritten_polygon][1];
	subdivided_polygons[poly_c][2] = subdivided_polygons[overwritten_polygon][2];
	subdivided_polygons[poly_d][3] = subdivided_polygons[overwritten_polygon][3];

	// Center
	// 
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[1][X] + 
										ptv[2][X] + ptv[3][X])>>2;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[1][Y] + 
										ptv[2][Y] + ptv[3][Y])>>2;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[1][Z] + 
										ptv[2][Z] + ptv[3][Z])>>2;
	

	subdivided_polygons[poly_a][2] = tgt_pnt;
	subdivided_polygons[poly_b][3] = tgt_pnt;
	subdivided_polygons[poly_c][0] = tgt_pnt;
	subdivided_polygons[poly_d][1] = tgt_pnt;

	tgt_pnt++;
	// 0 -> 1
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[1][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[1][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[1][Z])>>1;
	
	subdivided_polygons[poly_a][1] = tgt_pnt;
	subdivided_polygons[poly_b][0] = tgt_pnt;
	tgt_pnt++;
	// 1 -> 2
	subdivided_points[tgt_pnt][X] = (ptv[2][X] + ptv[1][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[2][Y] + ptv[1][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[2][Z] + ptv[1][Z])>>1;
	
	subdivided_polygons[poly_b][2] = tgt_pnt;
	subdivided_polygons[poly_c][1] = tgt_pnt;
	tgt_pnt++;
	// 3 -> 2
	subdivided_points[tgt_pnt][X] = (ptv[2][X] + ptv[3][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[2][Y] + ptv[3][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[2][Z] + ptv[3][Z])>>1;
	
	subdivided_polygons[poly_c][3] = tgt_pnt;
	subdivided_polygons[poly_d][2] = tgt_pnt;
	tgt_pnt++;
	// 3 -> 0
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[3][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[3][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[3][Z])>>1;
	
	subdivided_polygons[poly_a][3] = tgt_pnt;
	subdivided_polygons[poly_d][0] = tgt_pnt;
	tgt_pnt++;
	sub_vert_cnt = tgt_pnt;
	sub_poly_cnt += 3; //Only add 3, as there was already 1 polygon. It was split into four.
	
		if(num_divisions > 0)
		{
		//used_textures[poly_a] = texture_rules[num_divisions];
		//used_textures[poly_b] = texture_rules[num_divisions];
		//used_textures[poly_c] = texture_rules[num_divisions];
		//used_textures[poly_d] = texture_rules[num_divisions];
		///////////////////////////////////////////
		//	Recursively subdivide the polygon.
		//	Check the maximum Z of every new polygon.
		// 	If the maximum Z is less than zero, it's not on screen. No point in subdividing it any further.
		///////////////////////////////////////////
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][0]][Z], subdivided_points[subdivided_polygons[poly_a][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][2]][Z], subdivided_points[subdivided_polygons[poly_a][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_a, num_divisions-1, total_divisions+1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][0]][Z], subdivided_points[subdivided_polygons[poly_b][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][2]][Z], subdivided_points[subdivided_polygons[poly_b][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_b, num_divisions-1, total_divisions+1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_c][0]][Z], subdivided_points[subdivided_polygons[poly_c][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_c][2]][Z], subdivided_points[subdivided_polygons[poly_c][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_c, num_divisions-1, total_divisions+1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_d][0]][Z], subdivided_points[subdivided_polygons[poly_d][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_d][2]][Z], subdivided_points[subdivided_polygons[poly_d][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_d, num_divisions-1, total_divisions+1);
		}
	
	} else if(new_rule == '-') 
	{
	//////////////////////////////////////////////////////////////////
	// Subdivide between the edges 0->1 and 3->2 (""Vertically"")
	// (Splits the polygon such that new vertices are created between 0->3 and 1->2)
	//Turn 4 points into 6 points
	//Make the 2 new polygons
	//////////////////////////////////////////////////////////////////
	/*
	0A							1A		
					A
	3A--------------------------2A		
	0B--------------------------1B
					B
	3B							2B
	*/
	// Initial Conditions
	subdivided_polygons[poly_a][0] = subdivided_polygons[overwritten_polygon][0];
	subdivided_polygons[poly_a][1] = subdivided_polygons[overwritten_polygon][1];
	subdivided_polygons[poly_b][2] = subdivided_polygons[overwritten_polygon][2];
	subdivided_polygons[poly_b][3] = subdivided_polygons[overwritten_polygon][3];
	
	// 1 -> 2
	subdivided_points[tgt_pnt][X] = (ptv[2][X] + ptv[1][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[2][Y] + ptv[1][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[2][Z] + ptv[1][Z])>>1;
	
	subdivided_polygons[poly_a][2] = tgt_pnt;
	subdivided_polygons[poly_b][1] = tgt_pnt;
	tgt_pnt++;
	// 3 -> 0
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[3][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[3][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[3][Z])>>1;
	
	subdivided_polygons[poly_a][3] = tgt_pnt;
	subdivided_polygons[poly_b][0] = tgt_pnt;
	tgt_pnt++;
		
	sub_vert_cnt = tgt_pnt;
	sub_poly_cnt += 1; //Only add 1, as there was already 1 polygon. It was split in two.
	
		if(num_divisions > 0)
		{
		//used_textures[poly_a] = texture_rules[num_divisions];
		//used_textures[poly_b] = texture_rules[num_divisions];
		///////////////////////////////////////////
		//	Recursively subdivide the polygon.
		//	Check the maximum Z of every new polygon.
		// 	If the maximum Z is less than zero, it's not on screen. No point in subdividing it any further.
		///////////////////////////////////////////
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][0]][Z], subdivided_points[subdivided_polygons[poly_a][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][2]][Z], subdivided_points[subdivided_polygons[poly_a][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_a, num_divisions-1, total_divisions+1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][0]][Z], subdivided_points[subdivided_polygons[poly_b][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][2]][Z], subdivided_points[subdivided_polygons[poly_b][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_b, num_divisions-1, total_divisions+1);
		}
		
	} else if(new_rule == '|')
	{
	//////////////////////////////////////////////////////////////////
	// Subdivide between the edges 0->3 and 1->2 (""Horizontally"")
	// (Splits the polygon such that new vertices are created between 0->1 and 3->2)
	//Turn 4 points into 6 points
	//Make the 2 new polygons
	//////////////////////////////////////////////////////////////////
	/*
	0A			 1A | 0B			1B
					|		
			A		|		B
					|
	3A			 2A | 3B			2B
	*/
	// Initial Conditions
	subdivided_polygons[poly_a][0] = subdivided_polygons[overwritten_polygon][0];
	subdivided_polygons[poly_a][3] = subdivided_polygons[overwritten_polygon][3];
	subdivided_polygons[poly_b][1] = subdivided_polygons[overwritten_polygon][1];
	subdivided_polygons[poly_b][2] = subdivided_polygons[overwritten_polygon][2];
		
	// 0 -> 1
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[1][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[1][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[1][Z])>>1;
	
	subdivided_polygons[poly_a][1] = tgt_pnt;
	subdivided_polygons[poly_b][0] = tgt_pnt;
	tgt_pnt++;
	// 3 -> 2
	subdivided_points[tgt_pnt][X] = (ptv[2][X] + ptv[3][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[2][Y] + ptv[3][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[2][Z] + ptv[3][Z])>>1;
	
	subdivided_polygons[poly_a][2] = tgt_pnt;
	subdivided_polygons[poly_b][3] = tgt_pnt;
	tgt_pnt++;
	
	sub_vert_cnt = tgt_pnt;
	sub_poly_cnt += 1; //Only add 1, as there was already 1 polygon. It was split in two.
	
		if(num_divisions > 0)
		{
		//used_textures[poly_a] = texture_rules[num_divisions];
		//used_textures[poly_b] = texture_rules[num_divisions];
		///////////////////////////////////////////
		//	Recursively subdivide the polygon.
		//	Check the maximum Z of every new polygon.
		// 	If the maximum Z is less than zero, it's not on screen. No point in subdividing it any further.
		///////////////////////////////////////////
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][0]][Z], subdivided_points[subdivided_polygons[poly_a][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][2]][Z], subdivided_points[subdivided_polygons[poly_a][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_a, num_divisions-1, total_divisions+1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][0]][Z], subdivided_points[subdivided_polygons[poly_b][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][2]][Z], subdivided_points[subdivided_polygons[poly_b][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_b, num_divisions-1, total_divisions+1);
		}
	}
	
}

void	plane_rendering_with_subdivision(entity_t * ent)
{
	///////////////////////////////////////////
	// If the file is not yet loaded, do not try and render it.
	// If the entity type is not 'B' for BUILDING, do not try and render it as it won't have proper textures.
	///////////////////////////////////////////
		if(ent->file_done != true) return;
		if(ent->type != 'B') return;
		GVPLY * mesh = ent->pol;
		
		sub_poly_cnt = 0;
		sub_vert_cnt = 0;
		unsigned short	colorBank = 0;
		int		inverseZ = 0;
		int 	luma = 0;
		int 	zDepthTgt = 0;
		unsigned short	flags = 0;
		unsigned short	flip = 0;
		unsigned short	pclp = 0;
		
		FIXED * mesh_position = &ent->prematrix[9];

		POINT	pl_pts[4];

    static MATRIX newMtx;
	static FIXED m0x[4];
	static FIXED m1y[4];
	static FIXED m2z[4];
	
	//These can't have an orienation... eh, we'll do it anyway.
	slMultiMatrix((POINT *)ent->prematrix);
    slGetMatrix(newMtx);
	
	m0x[0] = newMtx[X][X];
	m0x[1] = newMtx[Y][X];
	m0x[2] = newMtx[Z][X];
	m0x[3] = newMtx[3][X];
	
	m1y[0] = newMtx[X][Y];
	m1y[1] = newMtx[Y][Y];
	m1y[2] = newMtx[Z][Y];
	m1y[3] = newMtx[3][Y];
	
	m2z[0] = newMtx[X][Z];
	m2z[1] = newMtx[Y][Z];
	m2z[2] = newMtx[Z][Z];
	m2z[3] = newMtx[3][Z];

prntidx = 0;

	/**
	Rendering Planes
	With polygon subdivision based on the Z (depth)

	Load up a plane.
	Transform its vertices by the matrix, but don't explicitly screenspace transform it.
	These vertices will be "screen-centered", so now we can subdivide the plane.
	Check the span of the plane to see if it is large.
	If it large in one of two particular ways, subdivide it by its longitude or its latitude (make two from one).
	If it is large in both ways, subdivide it both ways (make four from one).
	At this point, subdivision will occur recursively up to a limit arbitrated from the plane's Z,
	will cease subdivision on polygons with a high Z, and continue subdivision on polygons with a low Z.
	
	All but the lowest subdivision of a polygon will receive a combined texture.
	The texture is either a X*2, Y*2, or an X*2 & Y*2 combination -- the same as the subdivision pattern of the polygon.
	**/

	int max_z = 0;
	int min_z = 0;
	
	int max_subdivisions = 0;
	int specific_texture = 0;
	
	////////////////////////////////////////////////////
	// Transform each light source position by the matrix parameters.
	////////////////////////////////////////////////////
	POINT relative_light_pos = {0, 0, 0};
	static POINT tx_light_pos[MAX_DYNAMIC_LIGHTS];
	int inverted_proxima;
	
	for(int l = 0; l < MAX_DYNAMIC_LIGHTS; l++)
	{
		if(active_lights[l].pop == 1)
		{
			relative_light_pos[X] = -active_lights[l].pos[X] - mesh_position[X];
			relative_light_pos[Y] = -active_lights[l].pos[Y] - mesh_position[Y];
			relative_light_pos[Z] = -active_lights[l].pos[Z] - mesh_position[Z];
			tx_light_pos[l][X] = trans_pt_by_component(relative_light_pos, m0x);
			tx_light_pos[l][Y] = trans_pt_by_component(relative_light_pos, m1y);
			tx_light_pos[l][Z] = trans_pt_by_component(relative_light_pos, m2z);
		}
	}

for(unsigned int i = 0; i < mesh->nbPolygon; i++)
{
		sub_vert_cnt = 0;
		sub_poly_cnt = 0;
	for(int u = 0; u < 4; u++)
	{
	//////////////////////////////////////////////////////////////
	// Load the points
	//////////////////////////////////////////////////////////////
		pl_pts[u][X] = (mesh->pntbl[mesh->pltbl[i].Vertices[u]][X]);
		pl_pts[u][Y] = (mesh->pntbl[mesh->pltbl[i].Vertices[u]][Y]);
		pl_pts[u][Z] = (mesh->pntbl[mesh->pltbl[i].Vertices[u]][Z]);
	//////////////////////////////////////////////////////////////
	// Matrix transformation of the plane's points
	// Note: Does not yet transform to screenspace, clip by screen or portal, or push out to near plane.
	//////////////////////////////////////////////////////////////
        subdivided_points[u][Z] = trans_pt_by_component(pl_pts[u], m2z);
        subdivided_points[u][Y] = trans_pt_by_component(pl_pts[u], m1y);
        subdivided_points[u][X] = trans_pt_by_component(pl_pts[u], m0x);

		subdivided_polygons[0][u] = u;
	//////////////////////////////////////////////////////////////
	// Early screenspace transform to throw out off-screen planes
	//////////////////////////////////////////////////////////////
		//Push to near-plane
		ssh2VertArea[u].pnt[Z] = (subdivided_points[u][Z] > SUBDIVISION_NEAR_PLANE) ? subdivided_points[u][Z] : SUBDIVISION_NEAR_PLANE;
		//Get 1/z
		inverseZ = fxdiv(scrn_dist, ssh2VertArea[u].pnt[Z]);
        //Transform to screen-space
        ssh2VertArea[u].pnt[X] = fxm(subdivided_points[u][X], inverseZ)>>SCR_SCALE_X;
        ssh2VertArea[u].pnt[Y] = fxm(subdivided_points[u][Y], inverseZ)>>SCR_SCALE_Y;
        //Screen Clip Flags for on-off screen decimation
		ssh2VertArea[u].clipFlag = ((ssh2VertArea[u].pnt[X]) > TV_HALF_WIDTH) ? SCRN_CLIP_X : 0; 
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[X]) < -TV_HALF_WIDTH) ? SCRN_CLIP_NX : ssh2VertArea[u].clipFlag; 
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[Y]) > TV_HALF_HEIGHT) ? SCRN_CLIP_Y : ssh2VertArea[u].clipFlag;
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[Y]) < -TV_HALF_HEIGHT) ? SCRN_CLIP_NY : ssh2VertArea[u].clipFlag;
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[Z]) <= SUBDIVISION_NEAR_PLANE) ? CLIP_Z : ssh2VertArea[u].clipFlag;
		// clipping(&ssh2VertArea[u], USER_CLIP_INSIDE);
	}
		 if(ssh2VertArea[0].clipFlag
		 & ssh2VertArea[1].clipFlag
		 & ssh2VertArea[2].clipFlag
		 & ssh2VertArea[3].clipFlag) continue;
		 
	flags = mesh->attbl[i].render_data_flags;
	zDepthTgt = GET_SORT_DATA(flags);
		 
	//////////////////////////////////////////////////////////////
	// Screen-space back face culling segment. Will also avoid if the plane is flagged as dual-plane.
	//////////////////////////////////////////////////////////////
	if(flags & GV_FLAG_SINGLE)
	{
		 int cross0 = (ssh2VertArea[1].pnt[X] - ssh2VertArea[3].pnt[X])
							* (ssh2VertArea[0].pnt[Y] - ssh2VertArea[2].pnt[Y]);
		 int cross1 = (ssh2VertArea[1].pnt[Y] - ssh2VertArea[3].pnt[Y])
							* (ssh2VertArea[0].pnt[X] - ssh2VertArea[2].pnt[X]);
		if(cross0 >= cross1) continue;
	}
	//
	//////////////////////////////////////////////////////////////
	// We have at least four vertices, and at least one polygon (the plane's data itself).
	//////////////////////////////////////////////////////////////
		sub_vert_cnt += 4;
		sub_poly_cnt += 1;
	///////////////////////////////////////////
	//	Check the maximum Z of every new polygon.
	// 	This is the first polygon. So, if its maximum Z is too low, just discard it.
	///////////////////////////////////////////
	max_z = JO_MAX(JO_MAX(subdivided_points[subdivided_polygons[0][0]][Z], subdivided_points[subdivided_polygons[0][1]][Z]),
			JO_MAX(subdivided_points[subdivided_polygons[0][2]][Z], subdivided_points[subdivided_polygons[0][3]][Z]));
	if(max_z <= SUBDIVISION_NEAR_PLANE) continue;
	min_z = JO_MIN(JO_MIN(subdivided_points[subdivided_polygons[0][0]][Z], subdivided_points[subdivided_polygons[0][1]][Z]),
			JO_MIN(subdivided_points[subdivided_polygons[0][2]][Z], subdivided_points[subdivided_polygons[0][3]][Z]));
	///////////////////////////////////////////
	// Just a side note:
	// Doing subdivision in screen-space **does not work**.
	// Well, technically it works, but affine warping is experienced. It looks pretty awful.
	///////////////////////////////////////////
	
	if(flags & GV_FLAG_NDIV)
	{ 
		max_subdivisions = 0;
		used_textures[0] = 0;
	} else {
	//////////////////////////////////////////////////////////////
	// Check: Find the polygon's scale and thus subdivision scale.
	// We find the true perimeter of the polygon.
	//////////////////////////////////////////////////////////////
		int len01 = unfix_length(pl_pts[0], pl_pts[1]);
		int len12 = unfix_length(pl_pts[1], pl_pts[2]);
		int len23 = unfix_length(pl_pts[2], pl_pts[3]);
		int len30 = unfix_length(pl_pts[3], pl_pts[0]);
		int perimeter = len01 + len12 + len23 + len30;

		int len_w = (len01 + len23)>>1;
		int len_h = (len12 + len30)>>1;

		// nbg_sprintf(1, 6, "len12(%i)", len12);
		// nbg_sprintf(1, 7, "sx(%i)", unfix12[X]);
		// nbg_sprintf(1, 8, "sy(%i)", unfix12[Y]);
		
		if(perimeter <= 100 || perimeter >= 1600)
		{
			//In this case the polygon is too small or too large.
			//Large polygons will be excepted by making it look obviously wrong.
			used_textures[0] = 0;
			max_subdivisions = 0;
		} else {
			///////////////////////////////////////////
			// Resolve the maximum # of subdivisions.
			// This scale is based on the perimeter.
			// The texture assignment is not done yet.
			///////////////////////////////////////////
			if(perimeter > 800)
			{
				max_subdivisions = 3;
				
				texture_rules[3] = 16;
				texture_rules[2] = 11;
				texture_rules[1] = 6;
				texture_rules[0] = 1;
			} else if(perimeter > 400)
			{
				max_subdivisions = 2;
				
				texture_rules[3] = 16;
				texture_rules[2] = 16;
				texture_rules[1] = 11;
				texture_rules[0] = 6;
			} else if(perimeter > 100)
			{
				max_subdivisions = 1;
				
				texture_rules[3] = 16;
				texture_rules[2] = 16;
				texture_rules[1] = 16;
				texture_rules[0] = 11;
			}
			
			//A ratio should be established between the shorter length and longest length.
			//The ratio should follow powers-of-two rules, or atleast the logic that handles it should.
			//It is preferred that + is always done last.
			subdivision_rules[0] = '+';
			subdivision_rules[1] = '+';
			subdivision_rules[2] = '+';
			subdivision_rules[3] = '+';
			// nbg_sprintf(1, 7, "lw(%i)", len_w);
			// nbg_sprintf(1, 8, "lh(%i)", len_h);
			
			if(len_w >= (len_h<<2)) 
			{
				//If our max subdivisions is <3 at this point, we will never do '+', go ahead and add another subdivision.
				max_subdivisions += (max_subdivisions < 3) ? 1 : 0;

				if(max_subdivisions == 3)
				{
				//We know we're going to do |||
				texture_rules[2] = 15;
				texture_rules[1] = 14;
				texture_rules[0] = 13;
				} else if(max_subdivisions == 2)
				{
				//We know we're going to do ||
				texture_rules[2] = 16;
				texture_rules[1] = 15;
				texture_rules[0] = 14;
				}/*  else if(max_subdivisions == 1) // Case is handled by adding a subdivision earlier.
				{
				//We know we're going to do |
				texture_rules[2] = 16;
				texture_rules[1] = 16;
				texture_rules[0] = 15;
				}*/
				
				subdivision_rules[2] = '|';
				subdivision_rules[1] = '|';
				subdivision_rules[0] = '|';
			} else if(len_w >= (len_h * 3))
			{
				//If our max subdivisions is <2 at this point, we will never do '+', go ahead and add another subdivision.
				max_subdivisions += (max_subdivisions < 2) ? 1 : 0;
				
				if(max_subdivisions == 3)
				{
				//We know we're going to do ||+
				texture_rules[2] = 11;
				texture_rules[1] = 10;
				texture_rules[0] = 9;
				} else if(max_subdivisions == 2)
				{
				//We know we're going to do ||
				texture_rules[2] = 16;
				texture_rules[1] = 15;
				texture_rules[0] = 14;
				}/*  else if(max_subdivisions == 1) // Case is handled by adding a subdivision earlier.
				{
				//We know we're going to do |
				texture_rules[2] = 16;
				texture_rules[1] = 16;
				texture_rules[0] = 15;
				}*/
				
				subdivision_rules[1] = '|';
				subdivision_rules[0] = '|';
			} else if(len_w >= (len_h<<1))
			{
				//We cannot reach this point in the code if our max subdivisions is <1.
				if(max_subdivisions == 3)
				{
				//We know we're going to do |++
				texture_rules[2] = 11;
				texture_rules[1] = 6;
				texture_rules[0] = 5;
				} else if(max_subdivisions == 2)
				{
				//We know we're going to do |+
				texture_rules[2] = 16;
				texture_rules[1] = 11;
				texture_rules[0] = 10;
				} else if(max_subdivisions == 1)
				{
				//We know we're going to do ||
				//Why? To keep a polygon like this on the same scale as another polygon which received + once,
				//we have to subdivide by | twice. This is because + makes polygons of quarter size, instead of half size.
				//Again, this only applies to polygons that are twice-width or twice-height.
				//For thrice-height or thrice-width, a subdivision can be straight added, it will follow the same rules.
				texture_rules[2] = 16;
				texture_rules[1] = 15;
				texture_rules[0] = 14;
				subdivision_rules[1] = '|';
				max_subdivisions++;
				}

				subdivision_rules[0] = '|';
			}
			
			
			if(len_h >= (len_w<<2)) 
			{
				//If our max subdivisions is <3 at this point, we will never do '+', go ahead and add another subdivision.
				max_subdivisions += (max_subdivisions < 3) ? 1 : 0;
				if(max_subdivisions == 3)
				{
				//We know we're going to do ---
				texture_rules[2] = 12;
				texture_rules[1] = 8;
				texture_rules[0] = 4;
				} else if(max_subdivisions == 2)
				{
				//We know we're going to do --
				texture_rules[2] = 16;
				texture_rules[1] = 12;
				texture_rules[0] = 8;
				}/*  else if(max_subdivisions == 1) // Case handled above by adding a subdivision
				{
				//We know we're going to do -
				texture_rules[2] = 16;
				texture_rules[1] = 16;
				texture_rules[0] = 12;
				}*/
				
				subdivision_rules[2] = '-';
				subdivision_rules[1] = '-';
				subdivision_rules[0] = '-';
			} else if(len_h >= (len_w * 3))
			{
				//If our max subdivisions is <2 at this point, we will never do '+', go ahead and add another subdivision.
				max_subdivisions += (max_subdivisions < 2) ? 1 : 0;
				
				if(max_subdivisions == 3)
				{
				//We know we're going to do --+
				texture_rules[2] = 11;
				texture_rules[1] = 7;
				texture_rules[0] = 3;
				} else if(max_subdivisions == 2)
				{
				//We know we're going to do --
				texture_rules[2] = 16;
				texture_rules[1] = 12;
				texture_rules[0] = 8;
				}/*  else if(max_subdivisions == 1) //Case handled above by adding a subdivision
				{
				//We know we're going to do -
				texture_rules[2] = 16;
				texture_rules[1] = 16;
				texture_rules[0] = 12;
				} */
				
				subdivision_rules[1] = '-';
				subdivision_rules[0] = '-';
			
			} else if(len_h >= (len_w<<1))
			{
				//There is no situation where we can be <1 subdivisions if we are at this point in the code.
				if(max_subdivisions == 3)
				{
				//We know we're going to do -++
				texture_rules[2] = 11;
				texture_rules[1] = 7;
				texture_rules[0] = 3;
				} else if(max_subdivisions == 2)
				{
				//We know we're going to do -+
				texture_rules[2] = 16;
				texture_rules[1] = 11;
				texture_rules[0] = 7;
				} else if(max_subdivisions == 1)
				{
				//We know we're going to do --
				//Why? To keep a polygon like this on the same scale as another polygon which received + once,
				//we have to subdivide by - twice. This is because + makes polygons of quarter size, instead of half size.
				//Again, this only applies to polygons that are twice-width or twice-height.
				texture_rules[2] = 16;
				texture_rules[1] = 12;
				texture_rules[0] = 8;
				subdivision_rules[1] = '-';
				max_subdivisions++;
				}
				
				subdivision_rules[0] = '-';
			}
	
			// texture_rules[0] = 17;
			// texture_rules[1] = 17;
			// texture_rules[2] = 17;
			// texture_rules[3] = 17;
			subdivide_plane(sub_vert_cnt, 0, max_subdivisions, 0);
			///////////////////////////////////////////
			//
			// Screenspace Transform of Vertices
			// v = subdivided point index
			// testing_planes[i] = plane data index
			//
			///////////////////////////////////////////
			for(int v = 0; v < sub_vert_cnt; v++)
			{
				//Push to near-plane
				ssh2VertArea[v].pnt[Z] = (subdivided_points[v][Z] > SUBDIVISION_NEAR_PLANE) ? subdivided_points[v][Z] : SUBDIVISION_NEAR_PLANE;
				//Get 1/z
				inverseZ = fxdiv(scrn_dist, ssh2VertArea[v].pnt[Z]);
				//Transform to screen-space
				ssh2VertArea[v].pnt[X] = fxm(subdivided_points[v][X], inverseZ)>>SCR_SCALE_X;
				ssh2VertArea[v].pnt[Y] = fxm(subdivided_points[v][Y], inverseZ)>>SCR_SCALE_Y;
				//Screen Clip Flags for on-off screen decimation
				ssh2VertArea[v].clipFlag = ((ssh2VertArea[v].pnt[X]) > TV_HALF_WIDTH) ? SCRN_CLIP_X : 0; 
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[X]) < -TV_HALF_WIDTH) ? SCRN_CLIP_NX : ssh2VertArea[v].clipFlag; 
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) > TV_HALF_HEIGHT) ? SCRN_CLIP_Y : ssh2VertArea[v].clipFlag;
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) < -TV_HALF_HEIGHT) ? SCRN_CLIP_NY : ssh2VertArea[v].clipFlag;
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Z]) <= SUBDIVISION_NEAR_PLANE) ? CLIP_Z : ssh2VertArea[v].clipFlag;
				// clipping(&ssh2VertArea[v], USER_CLIP_INSIDE);
			}
		//Subdivision activation end stub
		}
	//Subdivision enabled end stub
	}
	///////////////////////////////////////////
	//
	// Z-sort Insertion & Command Arrangement of Polygons
	// j = subdivided polygon index
	//
	///////////////////////////////////////////
	if(ssh2SentPolys[0] + sub_poly_cnt > MAX_SSH2_SENT_POLYS) return;
	flags = (((flags & GV_FLAG_MESH)>>1) | ((flags & GV_FLAG_DARK)<<4))<<8;
	vertex_t * ptv[5] = {0, 0, 0, 0, 0};
	for(int j = 0; j < sub_poly_cnt; j++)
	{
		
		ptv[0] = &ssh2VertArea[subdivided_polygons[j][0]];
		ptv[1] = &ssh2VertArea[subdivided_polygons[j][1]];
		ptv[2] = &ssh2VertArea[subdivided_polygons[j][2]];
		ptv[3] = &ssh2VertArea[subdivided_polygons[j][3]];
		
		 int offScrn = (ptv[0]->clipFlag & ptv[1]->clipFlag & ptv[2]->clipFlag & ptv[3]->clipFlag);
		///////////////////////////////////////////
		// Z-Sorting Stuff	
		// Uses weighted max
		///////////////////////////////////////////
		//	if(JO_ABS(mesh->pltbl[i].norm[Y]) < 16384)
		//	{
		// zDepthTgt = (ptv[0]->pnt[Z] + ptv[2]->pnt[Z])>>1;
		//	} else {
		 zDepthTgt = (JO_MAX(
		JO_MAX(ptv[0]->pnt[Z], ptv[2]->pnt[Z]),
		JO_MAX(ptv[1]->pnt[Z], ptv[3]->pnt[Z])) + 
		((ptv[0]->pnt[Z] + ptv[1]->pnt[Z] + ptv[2]->pnt[Z] + ptv[3]->pnt[Z])>>2))>>1;
		//	}
			if(offScrn || zDepthTgt < NEAR_PLANE_DISTANCE || zDepthTgt > FAR_PLANE_DISTANCE) continue;
		///////////////////////////////////////////
		// Use a combined texture, if the subdivision system stated one should be used.
		// Otherwise, use the base texture.
		///////////////////////////////////////////
			if(used_textures[j] != 0)
			{
				specific_texture = ((mesh->attbl[i].texno - ent->base_texture) * TEXTS_GENERATED_PER_TEXTURE_LOADED)
				+ (ent->numTexture + ent->base_texture) + used_textures[j];
			} else {
				specific_texture = mesh->attbl[i].texno;
			}
		///////////////////////////////////////////
		// Flipping polygon such that vertice 0 is on-screen, or disable pre-clipping
		///////////////////////////////////////////
		flip = GET_FLIP_DATA(flags);
		preclipping(ptv, &flip, &pclp);
		///////////////////////////////////////////
		// Lighting Math
		// Using some approximation of an inverse squared law
		// The position of the polygon is treated as the average of points 0 and 2.
		///////////////////////////////////////////
		luma = 0;
		for(int l = 0; l < MAX_DYNAMIC_LIGHTS; l++)
		{
			if(active_lights[l].pop == 1)
			{
				
				/*
				This should be tabled for speed.
				A 3D relative pos table should be used. 
				Each entry is 10-bit precise.
				The output for each entry is the dot product of the three entries divided into one (inverse).
				*/
				relative_light_pos[X] = (tx_light_pos[l][X] - ((subdivided_points[subdivided_polygons[j][0]][X]
														+ subdivided_points[subdivided_polygons[j][2]][X])>>1))>>12;
				relative_light_pos[Y] = (tx_light_pos[l][Y] - ((subdivided_points[subdivided_polygons[j][0]][Y]
														+ subdivided_points[subdivided_polygons[j][2]][Y])>>1))>>12;
				relative_light_pos[Z] = (tx_light_pos[l][Z] - ((subdivided_points[subdivided_polygons[j][0]][Z]
														+ subdivided_points[subdivided_polygons[j][2]][Z])>>1))>>12;
				inverted_proxima = ((relative_light_pos[X] * relative_light_pos[X]) +
									(relative_light_pos[Y] * relative_light_pos[Y]) +
									(relative_light_pos[Z] * relative_light_pos[Z]))>>8;
				inverted_proxima = (inverted_proxima < 65536) ? division_table[inverted_proxima] : 0;
				
				// relative_light_pos[X] = (tx_light_pos[l][X] - ((subdivided_points[subdivided_polygons[j][0]][X]
														// + subdivided_points[subdivided_polygons[j][2]][X])>>1));
				// relative_light_pos[Y] = (tx_light_pos[l][Y] - ((subdivided_points[subdivided_polygons[j][0]][Y]
														// + subdivided_points[subdivided_polygons[j][2]][Y])>>1));
				// relative_light_pos[Z] = (tx_light_pos[l][Z] - ((subdivided_points[subdivided_polygons[j][0]][Z]
														// + subdivided_points[subdivided_polygons[j][2]][Z])>>1));
				// inverted_proxima = fxdot(relative_light_pos, relative_light_pos)>>16;
				
				// inverted_proxima = (inverted_proxima < 65536) ? division_table[inverted_proxima] : 0;
						
				luma += inverted_proxima * (int)active_lights[l].bright;
			}
		//	if(luma > 0) break; // Early exit
		}
		luma = (luma < 0) ? 0 : luma; 
		luma += fxdot(mesh->pltbl[i].norm, active_lights[0].ambient_light) + active_lights[0].min_bright; 
		determine_colorbank(&colorBank, &luma);
/* 		//Use transformed normal as shade determinant
		colorBank = (luma >= 98294) ? 0 : 1;
		colorBank = (luma < 49152) ? 2 : colorBank;
		colorBank = (luma < 32768) ? 3 : colorBank; 
		colorBank = (luma < 16384) ? 515 : colorBank; //Make really dark? use MSB shadow
 */			

      ssh2SetCommand(ptv[0]->pnt, ptv[1]->pnt,
					ptv[2]->pnt, ptv[3]->pnt,
		VDP1_BASE_CMDCTRL | flip, (VDP1_BASE_PMODE | flags) | pclp, //Reads flip value, mesh enable, and msb bit
		pcoTexDefs[specific_texture].SRCA, colorBank<<6, pcoTexDefs[specific_texture].SIZE, 0, zDepthTgt);
	}
    transVerts[0] += sub_vert_cnt;
	transPolys[0] += sub_poly_cnt;
}
	//////////////////////////////////////////////////////////////
	// Planar polygon subdivision rendering end stub
	//////////////////////////////////////////////////////////////

}





