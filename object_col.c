//object_col.c
//contains per polygon collision work
#include <sl_def.h>
#include "def.h"
#include "mymath.h"
#include "render.h"
#include "collision.h"
#include "physobjet.h"

#define MATH_TOLERANCE (16384)
#define MAX_COLLISION_PLANES (512)

//Purpose:
// Between loading levels, this function is ran to re-set the RAM pointer for rotated buildings.
// It is also ran to re-set the control data for said entities.
void	purge_rotated_entities(void)
{
	///////////////////////////////////////////////////////
	// Re-set data pointer
	///////////////////////////////////////////////////////
	HWRAM_ldptr = HWRAM_hldptr;
	///////////////////////////////////////////////////////
	// Purge control data of entities which weren't loaded from CD
	///////////////////////////////////////////////////////
	for(int i = 0; i < MAX_MODELS; i++)
	{
		if(!entities[i].was_loaded_from_CD)
		{
			//(Bit aggressive)
			entities[i].size = 0;
			entities[i].file_done = 0;
			entities[i].was_loaded_from_CD = 0;
			entities[i].base_texture = 0;
			entities[i].useClip = 0;
			entities[i].radius[X] = 0;
			entities[i].radius[Y] = 0;
			entities[i].radius[Z] = 0;
			entities[i].numTexture = 0;
			entities[i].first_portal = 0;
			entities[i].sortType = 0;
			entities[i].type = 0;
			entities[i].nbMeshes = 0;
			entities[i].nbFrames = 0;
			entities[i].pol = NULL;
			entities[i].prematrix = NULL;
		}
	}
	
}

// Purpose:
// Generate a pre-rotated permutation of a mesh for use in building-type rendering / collision.
// Efficient? No. Totally necessary? No. But easier to do this, than reconfigure those other code paths.
void	generate_rotated_entity_for_object(short declared_object_entry)
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
		if(entities[dWorldObjects[i].type.entity_ID].type == MODEL_TYPE_BUILDING)
		{
			short other_entity_original_id = dWorldObjects[i].type.clone_ID;
			if(other_entity_original_id == this_entity_ID && dWorldObjects[i].rot[X] == object->rot[X]
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
	newMesh->pltbl = oldMesh->pltbl;
	newMesh->lumatbl = oldMesh->lumatbl;
	//
	HWRAM_ldptr = align_4(HWRAM_ldptr);
	
	HWRAM_ldptr += sizeof(GVPLY);
	//Set a new memory address for the pntbl, nmtbl, and maxtbl.
	newMesh->pntbl = (POINT *)HWRAM_ldptr;
	HWRAM_ldptr += sizeof(POINT) * oldMesh->nbPoint;

	newMesh->nmtbl = (POINT *)HWRAM_ldptr;
	HWRAM_ldptr += sizeof(POINT) * oldMesh->nbPolygon;

	newMesh->maxtbl = (unsigned char *)HWRAM_ldptr;
	HWRAM_ldptr += sizeof(unsigned char) * oldMesh->nbPolygon;
	
	HWRAM_ldptr = align_4(HWRAM_ldptr);

	// nbg_sprintf(0, 8 + declared_object_entry,  "(%x)", newMesh->pntbl);
	// nbg_sprintf(10, 8 + declared_object_entry, "(%x)", newMesh->pltbl);
	// nbg_sprintf(20, 8 + declared_object_entry, "(%x)", newMesh->nmtbl);
	// nbg_sprintf(30, 8 + declared_object_entry, "(%x)", newMesh->maxtbl);
	
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

	//Using "1" as euler option.
	//Reason: this is happening in model-space.
	//The froward (world-space) euler is XZY; the model space is inverted from that.
	//That means the model space euler is YZX.
	//I am very annoyed that this happens but it is a fact of consequence in multiple places in the code.
	makeBoundBox(&bound_box_starter, EULER_OPTION_XYZ);
	
	POINT tRadius = {0, 0, 0};
	
	//Data copying with rotation
	for(int i = 0; i < numPt; i++)
	{
		newMesh->pntbl[i][X] = fxdot(oldMesh->pntbl[i], temp_box.UVX);
		newMesh->pntbl[i][Y] = fxdot(oldMesh->pntbl[i], temp_box.UVY);
		newMesh->pntbl[i][Z] = fxdot(oldMesh->pntbl[i], temp_box.UVZ);
		tRadius[X] = (JO_ABS(newMesh->pntbl[i][X]) > tRadius[X]) ? JO_ABS(newMesh->pntbl[i][X]) : tRadius[X];
		tRadius[Y] = (JO_ABS(newMesh->pntbl[i][Y]) > tRadius[Y]) ? JO_ABS(newMesh->pntbl[i][Y]) : tRadius[Y];
		tRadius[Z] = (JO_ABS(newMesh->pntbl[i][Z]) > tRadius[Z]) ? JO_ABS(newMesh->pntbl[i][Z]) : tRadius[Z];
	}
		POINT used_normal;
		unsigned char dominant_axis = 0;
	for(int i = 0; i < numPly; i++)
	{
		newMesh->nmtbl[i][X] = fxdot(oldMesh->nmtbl[i], temp_box.UVX);
		newMesh->nmtbl[i][Y] = fxdot(oldMesh->nmtbl[i], temp_box.UVY);
		newMesh->nmtbl[i][Z] = fxdot(oldMesh->nmtbl[i], temp_box.UVZ);
		
		//////////////////////////////////////////////////////////////
		// Grab the absolute normal used for finding the dominant axis
		//////////////////////////////////////////////////////////////
		used_normal[X] = JO_ABS(newMesh->nmtbl[i][X]);
		used_normal[Y] = JO_ABS(newMesh->nmtbl[i][Y]);
		used_normal[Z] = JO_ABS(newMesh->nmtbl[i][Z]);
		FIXED max_axis = JO_MAX(JO_MAX((used_normal[X]), (used_normal[Y])), (used_normal[Z]));
		dominant_axis = ((used_normal[X]) == max_axis) ? N_Xp : dominant_axis;
		dominant_axis = ((used_normal[Y]) == max_axis) ? N_Yp : dominant_axis;
		dominant_axis = ((used_normal[Z]) == max_axis) ? N_Zp : dominant_axis;
		//////////////////////////////////////////////////////////////
		// Check the sign.
		//////////////////////////////////////////////////////////////
		used_normal[X] = (newMesh->nmtbl[i][X]);
		used_normal[Y] = (newMesh->nmtbl[i][Y]);
		used_normal[Z] = (newMesh->nmtbl[i][Z]);
	
		if(dominant_axis == N_Xp && used_normal[X] < 0) dominant_axis = N_Xn;
		if(dominant_axis == N_Yp && used_normal[Y] < 0) dominant_axis = N_Yn;
		if(dominant_axis == N_Zp && used_normal[Z] < 0) dominant_axis = N_Zn;
		
		newMesh->maxtbl[i] = dominant_axis;
	}

	//Transformer the radius.
	entities[new_entity_ID].radius[X] = tRadius[X]>>16;
	entities[new_entity_ID].radius[Y] = tRadius[Y]>>16;
	entities[new_entity_ID].radius[Z] = tRadius[Z]>>16;
	//Make sure to tell the system that this entity was **NOT** loaded from the CD; it was generated.
	entities[new_entity_ID].was_loaded_from_CD = false;
	//Done!
	
	// nbg_sprintf(2, 6, "ox(%i)", entities[this_entity_ID].radius[X]);
	// nbg_sprintf(2, 7, "oy(%i)", entities[this_entity_ID].radius[Y]);
	// nbg_sprintf(2, 8, "oz(%i)", entities[this_entity_ID].radius[Z]);
	
	// nbg_sprintf(2, 10, "Nx(%i)", entities[new_entity_ID].radius[X]);
	// nbg_sprintf(2, 11, "Ny(%i)", entities[new_entity_ID].radius[Y]);
	// nbg_sprintf(2, 12, "Nz(%i)", entities[new_entity_ID].radius[Z]);
	
}

// Special notice for code observers:
// In rendering functions, prematrix data (which includes the translation data) is used.
// We could have simplified the function arguments here if we used the prematrix data.
// However, this cannot be done, because the prematrix data is entity-by-entity.
// Entities can be rendered multiple times, this means the prematrix data changes on which instance of the entity is rendered.
// This function is running on Master SH2. The rendering functions are running on Slave SH2.
// In this condition, the setting of the prematrix is controlled by the Slave SH2.
// Hitherto, the correct prematrix may not be set for testing collision. The position must be pointed to instead of changing the prematrix.
void	per_poly_collide(entity_t * ent, _boundBox * mover, FIXED * mesh_position, _lineTable * moverCFs, _lineTable * moverTimeAxis)
{
		//If the entity is not loaded, cease the test.
		if(ent->file_done != true) return;

GVPLY * mesh = ent->pol;
static unsigned short testing_planes[MAX_COLLISION_PLANES];
static unsigned char backfaced[MAX_COLLISION_PLANES];
static unsigned short last_hit_floor = 0;
static entity_t * last_floor_entity = 0;
short total_planes = 0;
static POINT discard_vector = {0, 0, 0};
static POINT plane_center = {0, 0, 0};
Bool hitY = false;
Bool hitXZ = false;
Bool shadowStruck = false;

	discard_vector[X] = JO_ABS(mesh_position[X] + mover->nextPos[X]) - JO_ABS(fxm(mover->velocity[X], time_fixed_scale));
	discard_vector[Y] = JO_ABS(mesh_position[Y] + mover->nextPos[Y]) - JO_ABS(fxm(mover->velocity[Y], time_fixed_scale));
	discard_vector[Z] = JO_ABS(mesh_position[Z] + mover->nextPos[Z]) - JO_ABS(fxm(mover->velocity[Z], time_fixed_scale));

	//If the player is farther away from the object than its radius, cease the test.
	if(discard_vector[X] > ((ent->radius[X])<<17) ||
	discard_vector[Y] > ((ent->radius[Y])<<17) ||
	discard_vector[Z] > ((ent->radius[Z])<<17)) return;
	
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
	// This test will also discard polygons which are too far away to possibly be collided with.
	// This assists greatly with levels made of plane data only.
	// PDATA vector space is inverted, so we negate them
	//////////////////////////////////////////////////////////////
	for(unsigned int dst_poly = 0; dst_poly < mesh->nbPolygon; dst_poly++)
	{
		//Need to use your *next* position, not your current, or last.
		discard_vector[X] = -(mesh->pntbl[mesh->pltbl[dst_poly].vertices[0]][X])
		- mover->nextPos[X] - mesh_position[X];
		discard_vector[Y] = -(mesh->pntbl[mesh->pltbl[dst_poly].vertices[0]][Y])
		- mover->nextPos[Y] - mesh_position[Y];
		discard_vector[Z] = -(mesh->pntbl[mesh->pltbl[dst_poly].vertices[0]][Z])
		- mover->nextPos[Z] - mesh_position[Z];

		int normal_discard = fxdot(discard_vector, mesh->nmtbl[dst_poly]);
		//This component should not be signed (distance).
		discard_vector[X] = JO_ABS(discard_vector[X]);
		discard_vector[Y] = JO_ABS(discard_vector[Y]);
		discard_vector[Z] = JO_ABS(discard_vector[Z]);
		int max_axis = 0;
		switch(mesh->maxtbl[dst_poly])
		{
			case(N_Xp):
			case(N_Xn):
			max_axis = JO_MAX(discard_vector[Y], discard_vector[Z])>>1;
			break;
			case(N_Yp):
			case(N_Yn):
			max_axis = JO_MAX(discard_vector[X], discard_vector[Z])>>1;
			break;
			case(N_Zp):
			case(N_Zn):
			max_axis = JO_MAX(discard_vector[Y], discard_vector[X])>>1;
			break;
			default:
			break;
		}
		int polygon_scale = (mesh->attbl[dst_poly].plane_information & 0x3) ? SUBDIVISION_SCALE<<1 : SUBDIVISION_SCALE;
		polygon_scale += (mesh->attbl[dst_poly].plane_information & 0xC) ? SUBDIVISION_SCALE : 0;
		polygon_scale += (mesh->attbl[dst_poly].plane_information & 0x30) ? SUBDIVISION_SCALE : 0;
		polygon_scale <<= 16;
		//mesh->attbl[dst_poly].render_data_flags &= (GV_FLAG_MESH ^ 0xFFFF);
		if(polygon_scale < normal_discard || polygon_scale < max_axis) continue;
	// slPrint("Discard vector:", slLocate(1, 9));
	// slPrintFX(discard_vector[X], slLocate(2, 10));
	// slPrintFX(discard_vector[Y], slLocate(2, 11));
	// slPrintFX(discard_vector[Z], slLocate(2, 12));
	// slPrint("Dot product:", slLocate(1, 13));
	// slPrintFX(normal_discard, slLocate(2, 14));
			

		if(!(mesh->attbl[dst_poly].render_data_flags & GV_FLAG_SINGLE) && (mesh->attbl[dst_poly].render_data_flags & GV_FLAG_PHYS) && total_planes < MAX_COLLISION_PLANES)
		{
		/////////
		// Dual-plane handling
		/////////
			//mesh->attbl[dst_poly].render_data_flags |= GV_FLAG_MESH;
		
			testing_planes[total_planes] = dst_poly;
			backfaced[total_planes] = (normal_discard >= 0) ? 0 : 1;
			total_planes++;
			continue;
		} else if(normal_discard >= -(5<<16) && (mesh->attbl[dst_poly].render_data_flags & GV_FLAG_PHYS) && total_planes < MAX_COLLISION_PLANES)
		{
		/////////
		// Single-plane handling
		///////// 
			//mesh->attbl[dst_poly].render_data_flags |= GV_FLAG_MESH;
		
			testing_planes[total_planes] = dst_poly;
			backfaced[total_planes] = 0;
			total_planes++;
		}
	}
	discard_vector[X] = 0;
	discard_vector[Y] = 0;
	discard_vector[Z] = 0;
	//nbg_sprintf(1, 15, "Total planes: (%i)", total_planes);

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
//__builtin_expect((unsigned int)last_floor_entity == (unsigned int)ent, (unsigned int)NULL);
if(you.hitSurface && last_floor_entity == ent && !you.setJet)
{
	//slPrint("Testing Old Floor", slLocate(2, 6));
	
	plane_center[X] = 0;
	plane_center[Y] = 0;
	plane_center[Z] = 0;
	for(int u = 0; u < 4; u++)
	{
	plane_points[u][X] = -(mesh->pntbl[mesh->pltbl[last_hit_floor].vertices[u]][X]) - mesh_position[X];
	plane_points[u][Y] = -(mesh->pntbl[mesh->pltbl[last_hit_floor].vertices[u]][Y]) - mesh_position[Y];
	plane_points[u][Z] = -(mesh->pntbl[mesh->pltbl[last_hit_floor].vertices[u]][Z]) - mesh_position[Z];
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
	// If it is a dual-plane which was determined to be otherwise back-facing, negate the normal.
	if(backfaced[last_hit_floor])
	{
		used_normal[X] = -mesh->nmtbl[last_hit_floor][X];
		used_normal[Y] = -mesh->nmtbl[last_hit_floor][Y];
		used_normal[Z] = -mesh->nmtbl[last_hit_floor][Z];
	} else {
		used_normal[X] = mesh->nmtbl[last_hit_floor][X];
		used_normal[Y] = mesh->nmtbl[last_hit_floor][Y];
		used_normal[Z] = mesh->nmtbl[last_hit_floor][Z];
	}
	dominant_axis = mesh->maxtbl[last_hit_floor];
	//////////////////////////////////////////////////////////////
	// Project the lines to the plane
	// We are only testing the Y axis right now, because we are specifically testing something previously determined to be a floor.
	// Note the second to last argument here: This is the tolerance level for collision.
	// The tolerance level for a collision here is double the normal tolerance for hitting a floor.
	// We prefer to have the player stick to surfaces they are standing on a little more strongly.
	//////////////////////////////////////////////////////////////
	lineChecks[Y] = line_hit_plane_here(moverCFs->yp0, moverCFs->yp1, plane_center, used_normal, discard_vector, 2<<16, lineEnds[Y]);
	//////////////////////////////////////////////////////////////
	// Proceed with edge wind testing if the line is still in the right spot.
	// Notice there are a few conditions here which are either manually inserted to a fixed value or omitted:
	// 1. We do not check the dominant axis. At this point, we already assume it is N_Yn, since it should be a floor.
	// 2. We do not check the backface of the plane. At this point, we already assume it met the backface condition to be a floor.
	// 3. We do not do any checks against it as if it were a wall. We already know it's supposed to be the floor.
	//////////////////////////////////////////////////////////////
	if(lineChecks[Y])
	{
		if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], lineEnds[Y], dominant_axis, 12))
		{
			you.floorNorm[X] = used_normal[X]; 
			you.floorNorm[Y] = used_normal[Y];
			you.floorNorm[Z] = used_normal[Z];
			
			if(mesh->attbl[last_hit_floor].render_data_flags & GV_FLAG_CLIMBABLE)
			{
				you.climbing = true;
				if(mesh->attbl[last_hit_floor].render_data_flags & GV_FLAG_LADDER)
				{
					if(slCos(you.rot2[Y]) >= 0)
					{
						you.rot2[Y] = 0;
					} else {
						you.rot2[Y] = 32768;
					}
					you.ladder = true;
				}
			} else {
			you.shadowPos[X] = lineEnds[Y][X];
			you.shadowPos[Y] = lineEnds[Y][Y];
			you.shadowPos[Z] = lineEnds[Y][Z];
			you.aboveObject = true;
			}
			standing_surface_alignment(you.floorNorm);
			you.floorPos[X] = (lineEnds[Y][X]) - mover->Yneg[X] - moverTimeAxis->yp1[X];
			you.floorPos[Y] = (lineEnds[Y][Y]) - mover->Yneg[Y] - moverTimeAxis->yp1[Y];
			you.floorPos[Z] = (lineEnds[Y][Z]) - mover->Yneg[Z] - moverTimeAxis->yp1[Z];
			

			
			you.hitSurface = true;
			you.hitObject = true;
			hitY = true; 
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
	plane_points[u][X] = -(mesh->pntbl[mesh->pltbl[testing_planes[i]].vertices[u]][X]) - mesh_position[X];
	plane_points[u][Y] = -(mesh->pntbl[mesh->pltbl[testing_planes[i]].vertices[u]][Y]) - mesh_position[Y];
	plane_points[u][Z] = -(mesh->pntbl[mesh->pltbl[testing_planes[i]].vertices[u]][Z]) - mesh_position[Z];
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
	// Grab dominant axis and set normal
	//////////////////////////////////////////////////////////////
	dominant_axis = mesh->maxtbl[testing_planes[i]];
	// If it is a dual-plane which was determined to be otherwise back-facing, negate the normal.
	if(backfaced[i])
	{
		used_normal[X] = -mesh->nmtbl[testing_planes[i]][X];
		used_normal[Y] = -mesh->nmtbl[testing_planes[i]][Y];
		used_normal[Z] = -mesh->nmtbl[testing_planes[i]][Z];
	} else {
		used_normal[X] = mesh->nmtbl[testing_planes[i]][X];
		used_normal[Y] = mesh->nmtbl[testing_planes[i]][Y];
		used_normal[Z] = mesh->nmtbl[testing_planes[i]][Z];
	}
	//////////////////////////////////////////////////////////////
	// Project the lines to the plane
	// Y first, then Z, then X
	// We separate the Y axis collision check with the X-Z axis collision checks.
	// The Y axis check is the only one which will plant the player on a floor,
	// so we want to continue checking after the first positive collision check of Y to see if we hit a wall or not.
	// Conversely, if we have hit a wall, it's possible we're hitting the floor on a different plane too.
	//////////////////////////////////////////////////////////////
	if(__builtin_expect(!hitY, 0))
	{
	lineChecks[Y] = line_hit_plane_here(moverCFs->yp0, moverCFs->yp1, plane_center, used_normal, discard_vector, 1<<16, lineEnds[Y]);
	} else {
	lineChecks[Y] = false;
	}
	if(__builtin_expect (!hitXZ, 0))
	{
	lineChecks[Z] = line_hit_plane_here(moverCFs->zp0, moverCFs->zp1, plane_center, used_normal, discard_vector, 32768, lineEnds[Z]);
	lineChecks[X] = line_hit_plane_here(moverCFs->xp0, moverCFs->xp1, plane_center, used_normal, discard_vector, 32768, lineEnds[X]);	
	} else {
	lineChecks[Z] = false;
	lineChecks[X] = false;
	}

	//////////////////////////////////////////////////////////////
	// Shadow Position
	// Just uses the Y line off the player.
	//////////////////////////////////////////////////////////////
 	if((!shadowStruck || !hitY) && (lineEnds[Y][Y] < you.pos[Y]))
	{	
		if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], lineEnds[Y], dominant_axis, 12))
		{
			shadowStruck = true;
			you.aboveObject = true;
			you.shadowPos[X] = lineEnds[Y][X];
			you.shadowPos[Y] = lineEnds[Y][Y];
			you.shadowPos[Z] = lineEnds[Y][Z];
		}
	}
	if(!lineChecks[0] && !lineChecks[1] && !lineChecks[2]) { continue; }
	//////////////////////////////////////////////////////////////
	// If we reach this point, at least one of the lines project to a point that is inside the player's bounding box.
	// Since we know that, we can start with a winding test comparing the point to the plane.
	// When we do this, we will discard the major axis of the normal to make it 2D.
	//////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////
	// Line Checks Y
	//////////////////////////////////////////////////////////////
	unsigned short climder = (mesh->attbl[testing_planes[i]].render_data_flags & GV_FLAG_LADDER) |
							(mesh->attbl[testing_planes[i]].render_data_flags & GV_FLAG_CLIMBABLE);
	if(!hitY)
	{
		if(lineChecks[Y])
		{
			//slPrint("Testing Y", slLocate(2, 6));
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], lineEnds[Y], dominant_axis, 12))
			{
				if((dominant_axis == N_Yn && !backfaced[i] && !you.setJet) || climder)
				{
					you.floorNorm[X] = used_normal[X]; 
					you.floorNorm[Y] = used_normal[Y];
					you.floorNorm[Z] = used_normal[Z];
					
					if(climder & GV_FLAG_CLIMBABLE)
					{
						you.climbing = true;
						if(climder & GV_FLAG_LADDER)
						{
							if(slCos(you.rot2[Y]) >= 0)
							{
								you.rot2[Y] = 0;
							} else {
								you.rot2[Y] = 32768;
							}
							you.ladder = true;
						}
					} else {
					you.shadowPos[X] = lineEnds[Y][X];
					you.shadowPos[Y] = lineEnds[Y][Y];
					you.shadowPos[Z] = lineEnds[Y][Z];
					last_hit_floor = testing_planes[i];
					last_floor_entity = ent;
					}
					
					standing_surface_alignment(you.floorNorm);
					
					you.floorPos[X] = (lineEnds[Y][X]) - mover->Yneg[X] - moverTimeAxis->yp1[X];
					you.floorPos[Y] = (lineEnds[Y][Y]) - mover->Yneg[Y] - moverTimeAxis->yp1[Y];
					you.floorPos[Z] = (lineEnds[Y][Z]) - mover->Yneg[Z] - moverTimeAxis->yp1[Z];
					
					you.hitSurface = true;
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
	//////////////////////////////////////////////////////////////
	// Line Checks Z
	//////////////////////////////////////////////////////////////
	if(!hitXZ)
	{
		if(lineChecks[Z])
		{
			//slPrint("Testing Z", slLocate(2, 6));
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], lineEnds[Z], dominant_axis, 12))
			{
				if(climder)
				{
					you.floorNorm[X] = used_normal[X]; 
					you.floorNorm[Y] = used_normal[Y];
					you.floorNorm[Z] = used_normal[Z];
					
					if(climder & GV_FLAG_CLIMBABLE)
					{
						you.climbing = true;
						if(climder & GV_FLAG_LADDER)
						{
							if(slCos(you.rot2[Y]) >= 0)
							{
								you.rot2[Y] = 0;
							} else {
								you.rot2[Y] = 32768;
							}
							you.ladder = true;
							you.IPaccel = 0;
						}
					}
					
					standing_surface_alignment(you.floorNorm);
					
					you.floorPos[X] = (lineEnds[Z][X]) - mover->Yneg[X] - moverTimeAxis->yp1[X];
					you.floorPos[Y] = (lineEnds[Z][Y]) - mover->Yneg[Y] - moverTimeAxis->yp1[Y];
					you.floorPos[Z] = (lineEnds[Z][Z]) - mover->Yneg[Z] - moverTimeAxis->yp1[Z];
					
					you.hitSurface = true;
				} else {
					you.wallNorm[X] = used_normal[X];
					you.wallNorm[Y] = used_normal[Y];
					you.wallNorm[Z] = used_normal[Z];
					you.wallPos[X] = -lineEnds[Z][X];
					you.wallPos[Y] = -lineEnds[Z][Y];
					you.wallPos[Z] = -lineEnds[Z][Z];
					
					you.hitWall = true;
				}
				you.hitObject = true;
				hitXZ = true;
			}
		}
	//////////////////////////////////////////////////////////////
	// Line Checks X
	//////////////////////////////////////////////////////////////
		if(lineChecks[X])
		{
			//slPrint("Testing X", slLocate(2, 6));
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], lineEnds[X], dominant_axis, 12))
			{
				if(climder)
				{
					you.floorNorm[X] = used_normal[X]; 
					you.floorNorm[Y] = used_normal[Y];
					you.floorNorm[Z] = used_normal[Z];
					
					if(climder & GV_FLAG_CLIMBABLE)
					{
						you.climbing = true;
						if(climder & GV_FLAG_LADDER)
						{
							if(slCos(you.rot2[Y]) >= 0)
							{
								you.rot2[Y] = 0;
							} else {
								you.rot2[Y] = 32768;
							}
							you.ladder = true;
							you.IPaccel = 0;
						}
					}
					
					standing_surface_alignment(you.floorNorm);
					
					you.floorPos[X] = (lineEnds[X][X]) - mover->Yneg[X] - moverTimeAxis->yp1[X];
					you.floorPos[Y] = (lineEnds[X][Y]) - mover->Yneg[Y] - moverTimeAxis->yp1[Y];
					you.floorPos[Z] = (lineEnds[X][Z]) - mover->Yneg[Z] - moverTimeAxis->yp1[Z];
					
					you.hitSurface = true;
				} else {
					you.wallNorm[X] = used_normal[X];
					you.wallNorm[Y] = used_normal[Y];
					you.wallNorm[Z] = used_normal[Z];
					you.wallPos[X] = -lineEnds[X][X];
					you.wallPos[Y] = -lineEnds[X][Y];
					you.wallPos[Z] = -lineEnds[X][Z];
						
					you.hitWall = true;
				}
				you.hitObject = true;
				hitXZ = true;
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

