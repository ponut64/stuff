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
			entities[i].sortType = 0;
			entities[i].type = 0;
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

void *	buildAdjacentSectorList(int entity_id, void * workAddress)
{
	//Step 1: Is this a valid entity type?
	if(entities[entity_id].type != MODEL_TYPE_SECTORED) return workAddress;
	if(!entities[entity_id].file_done) return workAddress;
	
	nbg_sprintf(1, 6, "Building render graph...");
	
	GVPLY * mesh = entities[entity_id].pol;

	workAddress = align_4(workAddress);
	unsigned short * writeAddress = (unsigned short *)workAddress;
	unsigned short * slapBuffer = (unsigned short *)dirty_buf;

	int t_plane[4][3];
	int t_center[3] = {0, 0, 0};
	int c_plane[4][3];
	int c_center[3] = {0, 0, 0};
	int y_min = 0;
	int y_max = 0;
	int within_span = 0;
	int within_shape_ct = 0;
	
	int adjSector = 0;
	
	int passNumber = 1;
	//Plan:
	//Pass 1: Find out how many sectors are adjacent to each sector.
	//Step: Allocate that memory for each sector (some amount of two byte indices)
	//Pass 2: Write the IDs of the adjacent sectors (in the respective reserved memory areas)
	//First, we have to initialize all sectors' adjacent count to zero. We do it here, but later we'll have to do it in an init function.
	PASS_2:
	for(unsigned int s = 0; s < MAX_SECTORS; s++)
	{
		sectors[s].nbVisible = 0;
	}

	//
	// Okay, some memory is bound to be wasted now.
	// Perhaps not if I run this over the dirty buf first.
	// Either way, I need to do this, and then remove duplicates.
	//

	for(unsigned int s = 0; s < MAX_SECTORS; s++)
	{
		_sector * sctA = &sectors[s];
		if(sctA->nbPolygon == 0) continue;
	for(unsigned int l = 0; l < MAX_SECTORS; l++)
	{
		int sector_adjacent = 0;
		_sector * sctB = &sectors[l];
		if(sctB->nbPolygon == 0) continue;
		//Don't test a sector against itself.
		if(l == s) continue;
		for(unsigned int i = 0; i < sctA->nbPolygon; i++)
		{
			//For now, we are only looking at floors.
			//Realistically, we should also look at walls. Maybe not ceilings ever.
			if(mesh->maxtbl[sctA->pltbl[i]] != N_Yn) continue;
			//In this loop, we're setting up a specific polygon of sctA to test against every polygon in sctB.
			y_min = mesh->pntbl[mesh->pltbl[sctA->pltbl[i]].vertices[0]][Y];
			y_max = mesh->pntbl[mesh->pltbl[sctA->pltbl[i]].vertices[0]][Y];
			for(int k = 0; k < 4; k++)
			{
				t_plane[k][X] = mesh->pntbl[mesh->pltbl[sctA->pltbl[i]].vertices[k]][X];
				t_plane[k][Y] = mesh->pntbl[mesh->pltbl[sctA->pltbl[i]].vertices[k]][Y];
				t_plane[k][Z] = mesh->pntbl[mesh->pltbl[sctA->pltbl[i]].vertices[k]][Z];
				t_center[X] += t_plane[k][X];
				t_center[Y] += t_plane[k][Y];
				t_center[Z] += t_plane[k][Z];
				y_min = (t_plane[k][Y] < y_min) ? t_plane[k][Y] : y_min;
				y_max = (t_plane[k][Y] > y_max) ? t_plane[k][Y] : y_max;
			}
			t_center[X] >>=2;
			t_center[Y] >>=2;
			t_center[Z] >>=2;
			
			//Add some margin of error on height axis
			y_min -= 1<<16;
			y_max += 1<<16;
			
			for(unsigned int p = 0; p < sctB->nbPolygon; p++)
			{
				if(p == i) continue;
				
				//No normal discardh here. Perhaps I should discard stuff if it's Y+ (facing down),
				//but otherwise, it'll test everything in sctB against the floors in sctA.
				//In this test, we're setting up polygons in sctB to be tested against a specific polygon in sctA.
				within_span = 0;
				for(int k = 0; k < 4; k++)
				{
					c_plane[k][X] = mesh->pntbl[mesh->pltbl[sctB->pltbl[p]].vertices[k]][X];
					c_plane[k][Y] = mesh->pntbl[mesh->pltbl[sctB->pltbl[p]].vertices[k]][Y];
					c_plane[k][Z] = mesh->pntbl[mesh->pltbl[sctB->pltbl[p]].vertices[k]][Z];
					if((t_plane[k][Y] > y_min) && (t_plane[k][Y] < y_max)) within_span = 1;
					c_center[X] += c_plane[k][X];
					c_center[Y] += c_plane[k][Y];
					c_center[Z] += c_plane[k][Z];
				}
				if(!within_span) continue;
				
				c_center[X] >>=2;
				c_center[Y] >>=2;
				c_center[Z] >>=2;
				
				//Method for detecting adjacency:
				//Hooray, it's the edge-wind test again. We check if a vertex is within the original polygon being tested.
				within_shape_ct = 0;
				for(int k = 0; k < 4; k++)
				{
					//For finding adjacent sectors, we need less information.
					//We can work with just a single adjacent vertex.
					if(edge_wind_test(t_plane[0], t_plane[1], t_plane[2], t_plane[3], c_plane[k], N_Yn, 16))
					{
						within_shape_ct++;
						break;
					}
				}
				//No vertices found within the other shape, continue.
				if(!within_shape_ct) continue;
				
				//So we think these floor polygons are adjacent.
				//We also think they are in different sectors.
				//Therefore, the sectors should be marked adjacent.
				//Depending on which phase of this loop we're in, we want to:
				//Pass 1: Add to each sector's adjacent sector count
				//Between the passes, we allocate the memory for *ALL* sectors adjacent sector list at once.
				//Pass 2: Assign each sector to each sector's adjacent list
				if(passNumber == 1)
				{
					sctA->nbVisible++;
					sctB->nbVisible++;
					adjSector++;
				} else if(passNumber == 2)
				{
					sctA->pvs[sctA->nbVisible] = mesh->attbl[sctB->pltbl[p]].first_sector;
					sctB->pvs[sctB->nbVisible] = mesh->attbl[sctA->pltbl[i]].first_sector;
					sctA->nbVisible++;
					sctB->nbVisible++;
				}
				//Once we know these are adjacent, break this loop. No further tests are needed.
				sector_adjacent = 1;
				break;
			}
			//We know A and B are adjacent now; break.
			if(sector_adjacent == 1) break;
		}
	}
	}
	
	//We've looked through every sector on the first pass at this point. That, or we're exiting the function.
	if(passNumber == 2)
	{
		//Second pass:
		//Copy the data we just made.
		//When building the final sector, add the pimary and secondary adjacents to each sector's adjacent list.
		//When finding secondary adjacents, we have to use the copied list, because the data of other sector's can change.
			unsigned short ** pvsCpy = (void*)slapBuffer;
			slapBuffer += sizeof(void*) * MAX_SECTORS;
			unsigned short * nbVisibleCpy = slapBuffer;
			slapBuffer += sizeof(void*) * MAX_SECTORS;
		for(int s = 0; s < MAX_SECTORS; s++)
		{
			_sector * sct = &sectors[s];
			nbVisibleCpy[s] = sct->nbVisible;
			//Copy the pvs's to another RAM set
			//Reserve some memory for that too.
			pvsCpy[s] = slapBuffer;
			unsigned short * this_pvs = pvsCpy[s];
			slapBuffer += nbVisibleCpy[s];
			for(unsigned int f = 0; f < nbVisibleCpy[s]; f++)
			{
				this_pvs[f] = sct->pvs[f];
			}
			
		}
		//Now we have to use the copied table as the source here.
 		for(int s = 0; s < MAX_SECTORS; s++)
		{
			unsigned short * actual_pvs = writeAddress;
			unsigned short actual_nbVisible = 0;
			unsigned short uniqueSet = 0;
			_sector * sct = &sectors[s];
			unsigned short * copy_primary_pvs = pvsCpy[s];

			//Process for adjacent / draw / near table:
			//We made a list of primary adjacents with the adjacent-sector math in the first half of the second pass.
			//We then copied those lists to pvsCpy and the amount in each sector to nbVisibleCpy.
			//We will use those unmodified lists to write the final lists, which contain the sector IDs of all
			//primary adjacent sectors and all sectors adjacent to those primary sectors.
			//By definition, we then include this sector in the list (because it's adjacent to the other ones).
			//While writing these to the final adjacent list, we will remove duplicates.
			/////////////////////////////////////////////////////////////////////////////
			// Explicit change
			// When processing the PVS, the system must be able to find the primary adjacent sectors in the PVS.
			// A "primary adjacent" is a sector which is physically touching another sector. Thus, it is mutual.
			// The way we do this is to place all primary adjacents in the same place in the PVS: after the sector self-identifier,
			// but before the non-adjacent sectors (in this case, sectors which are adjacent to primary adjacents).
			// We also store the number of primary adjacents to each sector when removing duplicates in the primary adjacent section.
			// This is important for determining when portals should or should not be used, which this part of the code does not handle,
			// but makes possible.
			/////////////////////////////////////////////////////////////////////////////
			unsigned short * secondary_adjacents = slapBuffer;
			int nbVisible2nd = 0;
			for(unsigned int l = 0; l < nbVisibleCpy[s]; l++)
			{
				unsigned short * copy_secondary_pvs = pvsCpy[copy_primary_pvs[l]];
				//Include primary adjacents first.
				secondary_adjacents[nbVisible2nd] = copy_primary_pvs[l];
				nbVisible2nd++;
				
				//Then include secondary adjacents.
				for(int e = 0; e < nbVisibleCpy[copy_primary_pvs[l]]; e++)
				{
					secondary_adjacents[nbVisible2nd] = copy_secondary_pvs[e];
					nbVisible2nd++;
				}
			}
			//At this point, the "secondary_adjacents" table has "nbVisible2nd" list of the two-deep adjacent sectors.
			//We also added the sectors adjacent in the first sector to it.
			//This is now the table we want to use to purge duplicates & write permanently.
			//Note that sectors add themselves to the adjacent list.
			//Here, we are just setting the PVS list to a known value.
			for(int l = 0; l < nbVisible2nd; l++)
			{
			actual_pvs[l] = INVALID_SECTOR;
			}
			
			//We can seed the PVS with a sector self-identifier; obviously a sector must see itself.
			actual_pvs[0] = s;
			actual_nbVisible++;
			
			//In this pass, we are exclusively writing the primary adjacent list as visible.
			//We do not need to know the total # of visible sectors now; just the # of primary adjacents.
			nbVisible2nd = 0;
			for(unsigned int l = 0; l < nbVisibleCpy[s]; l++)
			{
				//Include primary adjacents first.
				secondary_adjacents[nbVisible2nd] = copy_primary_pvs[l];
				nbVisible2nd++;
			}
			//In this loop, we determine the exact number of primary adjacents after removing duplicates from it.
			for(int i = 0; i < nbVisible2nd; i++)
			{
				uniqueSet = secondary_adjacents[i];
				for(int l = 0; l < nbVisible2nd; l++)
				{
					if(actual_pvs[l] == uniqueSet) uniqueSet = INVALID_SECTOR;
				}
				if(uniqueSet != INVALID_SECTOR) 
				{
					actual_pvs[actual_nbVisible] = uniqueSet;
					actual_nbVisible++;
					sct->nbAdjacent++;
				}
			}
			//In the next pass, we include the secondary adjacents after all primary adjacents.
			for(unsigned int l = 0; l < nbVisibleCpy[s]; l++)
			{
				unsigned short * copy_secondary_pvs = pvsCpy[copy_primary_pvs[l]];
				//Then include secondary adjacents.
				for(int e = 0; e < nbVisibleCpy[copy_primary_pvs[l]]; e++)
				{
					secondary_adjacents[nbVisible2nd] = copy_secondary_pvs[e];
					nbVisible2nd++;
				}
			}
			//After the primary adjacents were added to the PVS and duplicates removed,
			//we've now added all secondary adjacents to the potential PVS, and now they need to be added with duplicates removed.
			for(int i = 0; i < nbVisible2nd; i++)
			{
				uniqueSet = secondary_adjacents[i];
				for(int l = 0; l < nbVisible2nd; l++)
				{
					if(actual_pvs[l] == uniqueSet) uniqueSet = INVALID_SECTOR;
				}
				if(uniqueSet != INVALID_SECTOR) 
				{
					actual_pvs[actual_nbVisible] = uniqueSet;
					actual_nbVisible++;
				}
			}
			
			
			sct->nbVisible = actual_nbVisible;
			sct->pvs = actual_pvs;
			writeAddress += sct->nbVisible;
			
			sct->paths = (_pathNodes *)writeAddress;
			writeAddress += sct->nbAdjacent * sizeof(_pathNodes);
		} 
		
		nbg_sprintf(1, 6, "adjacents:(%i)", adjSector);
		workAddress = (void *)writeAddress;
		return align_4(workAddress);
	}
	//First pass:
	//Allocate memory for each sector's adjacent table in the temporary work area (dirtybuf).
	//I'm doing it this way with the goto statements because I don't want to repeat the above code,
	//and you can't do this on a per-sector basis (as in the loop) as each sector can add to another's adjacent count.
	//So they all have to be done at the same time once they're all done finding their adjacent counts.
	for(int s = 0; s < MAX_SECTORS; s++)
	{
		sectors[s].pvs = slapBuffer;
		slapBuffer += sectors[s].nbVisible;
	}
	
	passNumber = 2;
	
	goto PASS_2;
	
	//So this code is shit and we could end up down here past the goto, i guess? Nah, probably not.
}


//Exhaustive process here...
int		hitscan_vector_from_position_building(int * ray_normal, int * ray_pos, int * hit, int * hitPolyID, entity_t * ent, int * mesh_position, _sector * sct)
{
	GVPLY * mesh = ent->pol;
	
	//To hitscan every plane, we want to minimize the amount of work done.
	//Thus, we want to:
	//1. Discard backfaced planes (very important!)
	//2. Discard planes that are not collidable
	static POINT trash = {0, 0, 0};
	static POINT possible_hit = {0,0,0};
	static int plane_center[3] = {0,0,0};
	static int plane_points[4][3];
	int hasHit = 0;
	

	if(ent->type != MODEL_TYPE_SECTORED) sct = &sectors[INVALID_SECTOR];
	if(sct == NULL) sct = &sectors[INVALID_SECTOR];
	if(sct != &sectors[INVALID_SECTOR] && sct->nbPolygon == 0) return 0;
	
	//We should have already handled a case where the system may feed this a sector that is a mover sector.
	//This should never be valid, but instead such a case can be detected by the sector's entity not being MODEL_TYPE_SECTORED.
	//However, it sould be noted that this function recieving invalid sector is oddly enough valid.
	
	unsigned int ply_limit = (sct != &sectors[INVALID_SECTOR]) ? sct->nbPolygon : mesh->nbPolygon;
	
	for(unsigned int i = 0; i < ply_limit; i++)
	{		
		//The alias must change depending on if we're looking through sectors, all sectors, or an object without sectors.
		int alias = (sct != &sectors[INVALID_SECTOR]) ? sct->pltbl[i] : i;
		if(mesh->attbl[alias].render_data_flags & GV_FLAG_PORTAL) continue;
		//Exceptor: if the plane is not physical (no collision), don't try to collide with it.
		if(!(mesh->attbl[alias].render_data_flags & GV_FLAG_PHYS)) continue;
		if(mesh->attbl[alias].render_data_flags & GV_FLAG_SINGLE)
		{
			if(fxdot(ray_normal, mesh->nmtbl[alias]) > 0) continue;
		}
		
		plane_center[X] = 0;
		plane_center[Y] = 0;
		plane_center[Z] = 0;
		for(int u = 0; u < 4; u++)
		{
		plane_points[u][X] = (mesh->pntbl[mesh->pltbl[alias].vertices[u]][X]) + mesh_position[X];
		plane_points[u][Y] = (mesh->pntbl[mesh->pltbl[alias].vertices[u]][Y]) + mesh_position[Y];
		plane_points[u][Z] = (mesh->pntbl[mesh->pltbl[alias].vertices[u]][Z]) + mesh_position[Z];
		//Add to the plane's center
		plane_center[X] += plane_points[u][X];
		plane_center[Y] += plane_points[u][Y];
		plane_center[Z] += plane_points[u][Z];
		}
		//Divide sum of plane points by 4 to average all the points
		plane_center[X] >>=2;
		plane_center[Y] >>=2;
		plane_center[Z] >>=2;
		
		// for(int u = 0; u < 4; u++)
		// {
			// plane_points[u][X] -= plane_center[X];
			// plane_points[u][X] -= plane_center[Y];
			// plane_points[u][X] -= plane_center[Z];
			// anchor[X] = plane_points[u][X] & 0xFFFF;
			// anchor[Y] = plane_points[u][Y] & 0xFFFF;
			// anchor[Z] = plane_points[u][Z] & 0xFFFF;
			// plane_points[u][X] += anchor[X] + plane_center[X];
			// plane_points[u][Y] += anchor[Y] + plane_center[Y];
			// plane_points[u][Z] += anchor[Z] + plane_center[Z];
		// }

		ray_to_plane(ray_normal, ray_pos, mesh->nmtbl[alias], plane_center, possible_hit);

		//It can almost work without this. Might be an investigation point to find another shortcut.
		//(this cuts collisions behind the origin point, so we are only left with things in front)
		trash[X] = ray_pos[X] - possible_hit[X];
		trash[Y] = ray_pos[Y] - possible_hit[Y];
		trash[Z] = ray_pos[Z] - possible_hit[Z];
		int scale_to_phit = fxdot(trash, ray_normal);
		if(scale_to_phit > 0) continue;
		
		if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], possible_hit, mesh->maxtbl[alias], 12))
			{
				//Distance test "possible hit" to "ray_pos"
				//Can be very far, must use integers.
				//This is a process to filter multiple potential hits to the closest one.
				//We do however need to do this only when we have a hit at all.
				unsigned int possible_hit_scale = 0;
				unsigned int hit_scale = 0;
				trash[X] = (possible_hit[X] - ray_pos[X])>>16;
				trash[Y] = (possible_hit[Y] - ray_pos[Y])>>16;
				trash[Z] = (possible_hit[Z] - ray_pos[Z])>>16;
				possible_hit_scale = (trash[X] * trash[X]) + (trash[Y] * trash[Y]) + (trash[Z] * trash[Z]);
				trash[X] = (hit[X] - ray_pos[X])>>16;
				trash[Y] = (hit[Y] - ray_pos[Y])>>16;
				trash[Z] = (hit[Z] - ray_pos[Z])>>16;
				hit_scale = (trash[X] * trash[X]) + (trash[Y] * trash[Y]) + (trash[Z] * trash[Z]);
				
				if(possible_hit_scale < hit_scale)
				{
					hit[X] = possible_hit[X];
					hit[Y] = possible_hit[Y];
					hit[Z] = possible_hit[Z];
					*hitPolyID = alias;
					hasHit = 1;
				}
			}
	}
	return hasHit;
}

void	collide_in_sector_of_entity(entity_t * ent, _sector * sct, _boundBox * mover, _lineTable * realTimeAxis)
{
		//If the entity is not loaded, cease the test.
		if(ent->file_done != true) return;
		if(ent->type != MODEL_TYPE_SECTORED) return;
		
		//Primary Flaw:
		//We are testing collision with the *next* position of the box.
		//In doing so, we find the collision point on the *next* frame.
		//For walls, this is correct.
		//For floors, it's correct to test for the next frame, but we need to snap to where we are *this* frame.
		
		GVPLY * mesh = ent->pol;
	
	static int plane_center[3];
	static int plane_points[4][3];
	static int anchor_to_plane[3];
	static int used_normal[3];
	static int possible_floor[3];
	static int possible_wall[3];
	
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
			plane_points[u][X] += plane_center[X] + ent->prematrix[9];
			plane_points[u][Y] += plane_center[Y] + ent->prematrix[10];
			plane_points[u][Z] += plane_center[Z] + ent->prematrix[11];
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
			anchor_to_plane[X] = (mover->pos[X] - plane_center[X])>>16;
			anchor_to_plane[Y] = (mover->pos[Y] - plane_center[Y])>>16;
			anchor_to_plane[Z] = (mover->pos[Z] - plane_center[Z])>>16;
			
			int anchor_scale = (anchor_to_plane[X] * mesh->nmtbl[alias][X]) + (anchor_to_plane[Y] * mesh->nmtbl[alias][Y]) + (anchor_to_plane[Z] * mesh->nmtbl[alias][Z]);
			
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
			//case(N_Yp):
			case(N_Yn):
			
			//////////////////////////////////////////////////////////////
			// Floor branch
			//////////////////////////////////////////////////////////////
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis->yp0, mesh->maxtbl[alias], 12))
			{
				ray_to_plane(mover->UVY, mover->nextPos, used_normal, plane_center, possible_floor);
				
				// nbg_sprintf_decimal(2, 7, possible_floor[X]);
				// nbg_sprintf_decimal(2, 8, possible_floor[Y]);
				// nbg_sprintf_decimal(2, 9, possible_floor[Z]);
				
				// nbg_sprintf_decimal(2, 11, mover->nextPos[X]);
				// nbg_sprintf_decimal(2, 12, mover->nextPos[Y]);
				// nbg_sprintf_decimal(2, 13, mover->nextPos[Z]);
				
				if(isPointonSegment(possible_floor, realTimeAxis->yp0, realTimeAxis->yp1, 16384))
				{
					//
					you.hitSurface = true;
					you.floorNorm[X] = used_normal[X]; 
					you.floorNorm[Y] = used_normal[Y];
					you.floorNorm[Z] = used_normal[Z];
					
					//Subtract the velocity added to the projection from the position to snap to.
					//Potentially an issue; if serious, just reproject at current position.
					you.floorPos[X] = -(possible_floor[X] - fxm(mover->velocity[X], time_fixed_scale));
					you.floorPos[Y] = -(possible_floor[Y] - fxm(mover->velocity[Y], time_fixed_scale));
					you.floorPos[Z] = -(possible_floor[Z] - fxm(mover->velocity[Z], time_fixed_scale));
				}
			}
			break;
			case(N_Xp):
			case(N_Xn):
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis->xp1, mesh->maxtbl[alias], 12))
			{
				ray_to_plane(mover->UVX, mover->nextPos, used_normal, plane_center, possible_wall);
				if(isPointonSegment(possible_wall, realTimeAxis->xp0, realTimeAxis->xp1, 16384))
				{
					you.hitWall = true;
					you.wallNorm[X] = used_normal[X];
					you.wallNorm[Y] = used_normal[Y];
					you.wallNorm[Z] = used_normal[Z];
					
					you.wallPos[X] = possible_wall[X];
					you.wallPos[Y] = possible_wall[Y];
					you.wallPos[Z] = possible_wall[Z];
				}
			}
			break;
			case(N_Zp):
			case(N_Zn):
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis->zp1, mesh->maxtbl[alias], 12))
			{
				ray_to_plane(mover->UVZ, mover->nextPos, used_normal, plane_center, possible_wall);
				if(isPointonSegment(possible_wall, realTimeAxis->zp0, realTimeAxis->zp1, 16384))
				{
					you.hitWall = true;
					you.wallNorm[X] = used_normal[X];
					you.wallNorm[Y] = used_normal[Y];
					you.wallNorm[Z] = used_normal[Z];
					
					you.wallPos[X] = possible_wall[X];
					you.wallPos[Y] = possible_wall[Y];
					you.wallPos[Z] = possible_wall[Z];
				}
			}
			break;
		}
	
	}
	
	
}


// Special notice for code observers:
// In rendering functions, prematrix data (which includes the translation data) is used.
// We could have simplified the function arguments here if we used the prematrix data.
// However, this cannot be done, because the prematrix data is entity-by-entity.
// Entities can be rendered multiple times, this means the prematrix data changes on which instance of the entity is rendered.
// This function is running on Master SH2. The rendering functions are running on Slave SH2.
// In this condition, the setting of the prematrix is controlled by the Slave SH2.
// Hitherto, the correct prematrix may not be set for testing collision. The position must be pointed to instead of changing the prematrix.
void	collide_per_polygon_of_mesh(entity_t * ent, _boundBox * mover, _lineTable * realTimeAxis)
{
		//If the entity is not loaded, cease the test.
		if(ent->file_done != true) return;
		if(ent->type != MODEL_TYPE_BUILDING) return;
		
		//Primary Flaw:
		//We are testing collision with the *next* position of the box.
		//In doing so, we find the collision point on the *next* frame.
		//For walls, this is correct.
		//For floors, it's correct to test for the next frame, but we need to snap to where we are *this* frame.
		
		GVPLY * mesh = ent->pol;
		
	static int plane_center[3];
	static int plane_points[4][3];
	static int anchor_to_plane[3];
	static int used_normal[3];
	static int possible_floor[3];
	static int possible_wall[3];
	
	for(unsigned int i = 0; i < mesh->nbPolygon; i++)
	{
		
		//////////////////////////////////////////////////////////////
		// Add the position of the mesh to the position of its points
		// PDATA vector space is inverted, so we negate them
		// "Get world-space point position"
		//////////////////////////////////////////////////////////////
		int alias = i;
		plane_center[X] = 0;
		plane_center[Y] = 0;
		plane_center[Z] = 0;
		for(int u = 0; u < 4; u++)
		{
		plane_points[u][X] = (mesh->pntbl[mesh->pltbl[alias].vertices[u]][X] + ent->prematrix[9] ); 
		plane_points[u][Y] = (mesh->pntbl[mesh->pltbl[alias].vertices[u]][Y] + ent->prematrix[10] ); 
		plane_points[u][Z] = (mesh->pntbl[mesh->pltbl[alias].vertices[u]][Z] + ent->prematrix[11] );
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
			anchor_to_plane[X] = (mover->pos[X] - plane_center[X])>>16;
			anchor_to_plane[Y] = (mover->pos[Y] - plane_center[Y])>>16;
			anchor_to_plane[Z] = (mover->pos[Z] - plane_center[Z])>>16;
			
			int anchor_scale = (anchor_to_plane[X] * mesh->nmtbl[alias][X]) + (anchor_to_plane[Y] * mesh->nmtbl[alias][Y]) + (anchor_to_plane[Z] * mesh->nmtbl[alias][Z]);
			
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
			//case(N_Yp):
			case(N_Yn):
			
			//////////////////////////////////////////////////////////////
			// Floor branch
			//////////////////////////////////////////////////////////////
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis->yp0, mesh->maxtbl[alias], 12))
			{
				ray_to_plane(mover->UVY, mover->nextPos, used_normal, plane_center, possible_floor);
				
				if(isPointonSegment(possible_floor, realTimeAxis->yp0, realTimeAxis->yp1, 16384))
				{
					//
					you.hitSurface = true;
					you.floorNorm[X] = used_normal[X]; 
					you.floorNorm[Y] = used_normal[Y];
					you.floorNorm[Z] = used_normal[Z];
					
					//Subtract the velocity added to the projection from the position to snap to.
					//Potentially an issue; if serious, just reproject at current position.
					you.floorPos[X] = -(possible_floor[X] - fxm(mover->velocity[X], time_fixed_scale));
					you.floorPos[Y] = -(possible_floor[Y] - fxm(mover->velocity[Y], time_fixed_scale));
					you.floorPos[Z] = -(possible_floor[Z] - fxm(mover->velocity[Z], time_fixed_scale));
				}
			}
			break;
			case(N_Xp):
			case(N_Xn):
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis->xp1, mesh->maxtbl[alias], 12))
			{
				ray_to_plane(mover->UVX, mover->nextPos, used_normal, plane_center, possible_wall);
				if(isPointonSegment(possible_wall, realTimeAxis->xp0, realTimeAxis->xp1, 16384))
				{
					you.hitWall = true;
					you.wallNorm[X] = used_normal[X];
					you.wallNorm[Y] = used_normal[Y];
					you.wallNorm[Z] = used_normal[Z];
					
					you.wallPos[X] = possible_wall[X];
					you.wallPos[Y] = possible_wall[Y];
					you.wallPos[Z] = possible_wall[Z];
				}
			}
			break;
			case(N_Zp):
			case(N_Zn):
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], realTimeAxis->zp1, mesh->maxtbl[alias], 12))
			{
				ray_to_plane(mover->UVZ, mover->nextPos, used_normal, plane_center, possible_wall);
				if(isPointonSegment(possible_wall, realTimeAxis->zp0, realTimeAxis->zp1, 16384))
				{
					you.hitWall = true;
					you.wallNorm[X] = used_normal[X];
					you.wallNorm[Y] = used_normal[Y];
					you.wallNorm[Z] = used_normal[Z];
					
					you.wallPos[X] = possible_wall[X];
					you.wallPos[Y] = possible_wall[Y];
					you.wallPos[Z] = possible_wall[Z];
				}
			}
			break;
		}
	
	}
	
	
}

