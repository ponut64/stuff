
#include <SL_DEF.H>
#include <SEGA_GFS.H>
#include "def.h"
#include "tga.h"
#include "render.h"

#include "physobjet.h"
#include "mymath.h"

#include "mloader.h"

entity_t entities[MAX_MODELS];
_sector sectors[MAX_SECTORS+1];
_pathHost * pathing;

int debug_number;

void	*	load_sectors(entity_t * ent, void * workAddress)
{
	nbg_sprintf(1, 7, "Building sectors...");
	
	workAddress = align_4(workAddress);
	
	unsigned short * writeAddress = (unsigned short *)workAddress;
	unsigned short * slapBuffer = (unsigned short *)dirty_buf;
	unsigned short * clapBuffer = (unsigned short *)((unsigned int)dirty_buf + 16384);
	unsigned char plane_sector = 0;
	GVPLY * mesh = ent->pol;
	
	static int sectored_polygons = 0;
	static int sectored_verts = 0;
	static int sectors_made = 0;
	
	sectors[INVALID_SECTOR].nbPoint = 0;
	sectors[INVALID_SECTOR].nbPolygon = 0;
	sectors[INVALID_SECTOR].nbVisible = 0;
	sectors[INVALID_SECTOR].nbPortal = 0;
	sectors[INVALID_SECTOR].nbAdjacent = 0;
	sectors[INVALID_SECTOR].ent = ent;
	
	//Complicated. The idea is that each sector must have a coherent list built for it.
	//Because of that, we have to scan the mesh on a per-possible-sector basis.
	//The first thing we will do is build the pltbl.
	for(unsigned int k = 0; k < MAX_SECTORS; k++)
	{
		sectors[k].nbPolygon = 0;
		sectors[k].nbAdjacent = 0;
		sectors[k].nbVisible = 0;
		sectors[k].pltbl = writeAddress;
		int is_mover = 0;
		for(unsigned int i = 0; i < mesh->nbPolygon; i++)
		{
			plane_sector = mesh->attbl[i].first_sector;
			if(plane_sector != k) continue;
			//If this is a portal, don't add it to the polygon list. It's added elsewhere.
			if(mesh->attbl[i].render_data_flags & GV_FLAG_PORTAL) continue;
			//If this sector has a polygon representative of a mover, don't add it. It's built as a different kind of object.
			if((mesh->attbl[i].render_data_flags & GV_SCTR_MOVER) == GV_SCTR_MOVER)
			{
				is_mover = 1;
			}
			
			sectors[k].pltbl[sectors[k].nbPolygon] = i;
			sectors[k].nbPolygon++;			
		}
		writeAddress += sectors[k].nbPolygon;
		//If this condition is passed, the sector will be built as a mover-type object.
		//Movers are complex objects; they are defined from mesh data in the binary file used for sector data (here).
		//Then, the position to which they move is marked by item data carried in the binary payload as well.
		//The item data defers to rate and trigger types set inside of the game engine.
		//The first thing we have to do then is build up a PDATA and entity listing for the mover.
		if(is_mover)
		{
			//In disdainful deference of the way that sector data is built, we will not be destroying the mesh data within it.
			//Instead, we will advance the work RAM pointer to copy out the data relevant for the mover.
			//Then, register another model data entry and entity entry that points to them.
			//Dangerously, however, this mesh data will lie in the midst of sector data.
			
			//We already have a qualified list of polygons whom are a member of this mover sector from the prior loop.
			//We also know the number of polygons in the sector.
			//We need to basically do what we do to build a sector, but more.
			
			GVPLY * moverMesh = (GVPLY *)writeAddress;
			writeAddress += sizeof(GVPLY);
			moverMesh->nbPolygon = sectors[k].nbPolygon;
			sectors[k].nbPolygon = 0;
			moverMesh->pltbl = (_quad*)writeAddress;
			writeAddress += sizeof(_quad) * moverMesh->nbPolygon;
			moverMesh->attbl = (gvAtr*)writeAddress;
			writeAddress += sizeof(gvAtr) * moverMesh->nbPolygon;
			//(single byte tables incoming)
			//align the address appropriately
			writeAddress += ((unsigned int)workAddress & 1) ? 1 : 0;
			writeAddress += ((unsigned int)workAddress & 2) ? 2 : 0;
			
			moverMesh->maxtbl = (unsigned char *)writeAddress;
			writeAddress += moverMesh->nbPolygon;
			moverMesh->lumatbl = (unsigned char *)writeAddress;
			writeAddress += moverMesh->nbPolygon;
			
			//align the address appropriately
			writeAddress += ((unsigned int)workAddress & 1) ? 1 : 0;
			writeAddress += ((unsigned int)workAddress & 2) ? 2 : 0;
			
			moverMesh->nmtbl = (POINT*)writeAddress;
			writeAddress += sizeof(POINT) * moverMesh->nbPolygon;
			
			//Zero-out the mover sector polygon count; it will no longer be relevant.
			
			unsigned int moverVerts = 0;
			for(unsigned int i = 0; i < moverMesh->nbPolygon; i++)
			{
				//This is going to be pretty naive.
				//(these vertex IDs will have to be aliased back to the reduced vertex number list later)
				moverMesh->pltbl[i].vertices[0] = mesh->pltbl[sectors[k].pltbl[i]].vertices[0];
				moverMesh->pltbl[i].vertices[1] = mesh->pltbl[sectors[k].pltbl[i]].vertices[1];
				moverMesh->pltbl[i].vertices[2] = mesh->pltbl[sectors[k].pltbl[i]].vertices[2];
				moverMesh->pltbl[i].vertices[3] = mesh->pltbl[sectors[k].pltbl[i]].vertices[3];
				
				//Copy the attributes
				gvAtr * attr = &mesh->attbl[sectors[k].pltbl[i]];
				moverMesh->attbl[i].render_data_flags = attr->render_data_flags; //(probably exclude the mover flag here)
				moverMesh->attbl[i].tile_information = attr->tile_information;
				moverMesh->attbl[i].plane_information = attr->plane_information;
				moverMesh->attbl[i].uv_id = attr->uv_id;
				moverMesh->attbl[i].first_sector = attr->first_sector;
				moverMesh->attbl[i].texno = attr->texno;
				
				//Copy the normal table
				moverMesh->nmtbl[i][X] = mesh->nmtbl[sectors[k].pltbl[i]][X];
				moverMesh->nmtbl[i][Y] = mesh->nmtbl[sectors[k].pltbl[i]][Y];
				moverMesh->nmtbl[i][Z] = mesh->nmtbl[sectors[k].pltbl[i]][Z];
				
				//Copy maxtbl and lumatbl
				moverMesh->maxtbl[i] = mesh->maxtbl[sectors[k].pltbl[i]];
				moverMesh->lumatbl[i] = mesh->lumatbl[sectors[k].pltbl[i]];
				
				slapBuffer[moverVerts] = moverMesh->pltbl[i].vertices[0];
				clapBuffer[moverVerts] = INVALID_PLANE;
				moverVerts++;
				slapBuffer[moverVerts] = moverMesh->pltbl[i].vertices[1];
				clapBuffer[moverVerts] = INVALID_PLANE;
				moverVerts++;
				slapBuffer[moverVerts] = moverMesh->pltbl[i].vertices[2];
				clapBuffer[moverVerts] = INVALID_PLANE;
				moverVerts++;
				slapBuffer[moverVerts] = moverMesh->pltbl[i].vertices[3];
				clapBuffer[moverVerts] = INVALID_PLANE;
				moverVerts++;
			}
			int secMoverVerts = 0;
			int movUniqueVert = 0;
			//Sets "uniqueSet", checks the entire "clapBuffer" for unqiue set.
			//If uniqueSet is not in clapBuffer, add to clap buffer, add to total vertex number.
			for(unsigned int i = 0; i < moverVerts; i++)
			{
				movUniqueVert = slapBuffer[i];
				for(unsigned int l = 0; l < moverVerts; l++)
				{
					if(clapBuffer[l] == movUniqueVert) movUniqueVert = INVALID_PLANE;
				}
				if(movUniqueVert != INVALID_PLANE) 
				{
					clapBuffer[secMoverVerts] = movUniqueVert;
					secMoverVerts++;
				}
			}
			
			//
			
			moverMesh->nbPoint = secMoverVerts;
			moverMesh->pntbl = (POINT*)writeAddress;
			writeAddress += sizeof(POINT) * secMoverVerts;
			for(unsigned int i = 0; i < moverMesh->nbPoint; i++)
			{
				//sectors[k].pntbl[i] = clapBuffer[i];
				moverMesh->pntbl[i][X] = mesh->pntbl[clapBuffer[i]][X];
				moverMesh->pntbl[i][Y] = mesh->pntbl[clapBuffer[i]][Y];
				moverMesh->pntbl[i][Z] = mesh->pntbl[clapBuffer[i]][Z];
			}
			
 			for(unsigned int i = 0; i < moverMesh->nbPolygon; i++)
			{
				for(int p = 0; p < 4; p++)
				{
					for(unsigned int f = 0; f < moverMesh->nbPoint; f++)
					{
						if(mesh->pltbl[sectors[k].pltbl[i]].vertices[p] == clapBuffer[f])
						{
							//What this should do is:
							//Polygon has vertex ID #255, which is sector vertex ID #0, sector polygon vertex ID of #255 becomes #0
							moverMesh->pltbl[i].vertices[p] = f;
						}
					}
				}
			} 
			
			//With the mesh data settled, we can now create an entity.
			//First we have to find an empty entry in the entity list.
			entity_t * m_ent = &entities[MAX_MODELS-1];
			int eid = 0;
			for(int i = 0; i < MAX_MODELS; i++)
			{
				if(!entities[i].file_done && !entities[i].was_loaded_from_CD && entities[i].pol == NULL)
				{
					m_ent = &entities[i];
					m_ent->file_done = 1;
					m_ent->was_loaded_from_CD = 1;
					m_ent->pol = moverMesh;
					eid = i;
					break;
				}
			}
			if(m_ent->pol == NULL) continue; //When no empty entitiy slot, do not continue.
			
			debug_number = eid;
			//I've kinda got no clue what's happening here.
			//I do think I've miscalculated and I need to go back and make the vertices relevant to the center though.
			
			//We can inherit a lot of the configuration information from the sector data entity.
			m_ent->size = ent->size; //(false, but, w/e)
			m_ent->base_texture = ent->base_texture; //(the texture indicides are indexed from this point as well)
			m_ent->useClip = ent->useClip; //why do i even have this flag?
			m_ent->numTexture = ent->numTexture;
			m_ent->sortType = ent->sortType;
			m_ent->type = MODEL_TYPE_BUILDING;
			m_ent->nbFrames = 0;
			
			//Similiar to the sector data, a radius has to be assigned to the entity.
			int mov_accum[3] = {0,0,0};
			int mov_maxaxis[3] = {0,0,0};
			int mov_ctrPos[3] = {0,0,0}; //This is particularly important to calculate, to locate the final object.
			for(unsigned int i = 0; i < moverMesh->nbPoint; i++)
			{
				mov_accum[X] += (moverMesh->pntbl[i][X]>>16);
				mov_accum[Y] += (moverMesh->pntbl[i][Y]>>16);
				mov_accum[Z] += (moverMesh->pntbl[i][Z]>>16);
			}
			
			mov_accum[X] = (mov_accum[X] / moverMesh->nbPoint)<<16;
			mov_accum[Y] = (mov_accum[Y] / moverMesh->nbPoint)<<16;
			mov_accum[Z] = (mov_accum[Z] / moverMesh->nbPoint)<<16;
			//We must negate the applied value due to the inversion of the projection space.
			mov_ctrPos[X] = -mov_accum[X];
			mov_ctrPos[Y] = -mov_accum[Y];
			mov_ctrPos[Z] = -mov_accum[Z];
			
			//The final entity needs to have its vertices be relative to its center, so we have to introduce that.
			for(unsigned int i = 0; i < moverMesh->nbPoint; i++)
			{
				mov_accum[X] = JO_ABS(moverMesh->pntbl[i][X] + mov_ctrPos[X]);
				mov_accum[Y] = JO_ABS(moverMesh->pntbl[i][Y] + mov_ctrPos[Y]);
				mov_accum[Z] = JO_ABS(moverMesh->pntbl[i][Z] + mov_ctrPos[Z]);
				
				mov_maxaxis[X] = (mov_accum[X] > mov_maxaxis[X]) ? mov_accum[X] : mov_maxaxis[X];
				mov_maxaxis[Y] = (mov_accum[Y] > mov_maxaxis[Y]) ? mov_accum[Y] : mov_maxaxis[Y];
				mov_maxaxis[Z] = (mov_accum[Z] > mov_maxaxis[Z]) ? mov_accum[Z] : mov_maxaxis[Z];
				
				moverMesh->pntbl[i][X] += (mov_ctrPos[X]);
				moverMesh->pntbl[i][Y] += (mov_ctrPos[Y]);
				moverMesh->pntbl[i][Z] += (mov_ctrPos[Z]);
			}
			//Add a margin to it.
			m_ent->radius[X] = (mov_maxaxis[X]>>16) + 4;
			m_ent->radius[Y] = (mov_maxaxis[Y]>>16) + 4;
			m_ent->radius[Z] = (mov_maxaxis[Z]>>16) + 4;
			
			
			//Finally, we need to declare this as a world object.
			
			//Allocate memory for the type entry
			_sobject * type = (_sobject *)writeAddress;
			writeAddress += sizeof(_sobject);
			int valid_type_found = 0;
			for(int s = 0; s < OBJECT_ENTRY_CAP; s++)
			{
				if(objList[s] == NULL)
				{
					objList[s] = type;
					type->entity_ID = eid;
					type->clone_ID = eid;
					type->radius[X] = m_ent->radius[X];
					type->radius[Y] = m_ent->radius[Y];
					type->radius[Z] = m_ent->radius[Z];
					type->ext_dat = BUILD;
					type->light_bright = 0;
					type->light_y_offset = 0;
					type->effect = 0;
					type->effectTimeLimit = 0;
					type->effectTimeCount = 0;
					valid_type_found = s;
					break;
				}
			}
			
			if(valid_type_found)
			{
			BuildingPayload[total_building_payload].object_type = valid_type_found;
			BuildingPayload[total_building_payload].pos[X] = -(mov_ctrPos[X]>>16);
			BuildingPayload[total_building_payload].pos[Y] = -(mov_ctrPos[Y]>>16);
			BuildingPayload[total_building_payload].pos[Z] = -(mov_ctrPos[Z]>>16);
			BuildingPayload[total_building_payload].sector = INVALID_SECTOR;
			//Some way to find what entity # we're working with right now
			BuildingPayload[total_building_payload].root_entity = (unsigned short)(ent - entities);
			total_building_payload++;
			
			//nbg_sprintf(1, 10, "mover t(%i), p(%i)", valid_type_found, eid);
			}
			//If a valid type was found, we need to prepare an object declaration.
			//I think I will repurpose the building-object system.
			
			//Finally, after all of this processing is done for the sector, we have to mark the empty sector as this object.
			//We do this by changing the entity pointer of the sector.
			//Normally, it points to the entity data that hosts all of the sector data and is of type MODEL_TYPE_SECTORED.
			//Mover sectors will point to this newly created entity, which is of type MODEL_TYPE_BUILDING.
			//This difference in type will denote the sector as a sector for a mover (for now).
			sectors[k].ent = m_ent;
			//Please note that this part will not connect any associated triggers for this mover.
			//The mover target data will be connected later, when the mover target data is initialized and looks at this sector.
			//When they do that, they will find this entity. They will then scan all declared objects to find this entity.
			//They will only then link the mover target data to the declared object of the mover based on this entity pointer.
			
			continue;
		}
		if(!sectors[k].nbPolygon) continue; 
		sectors[k].ent = ent;
		sectors[k].nbPoint = 0;
		sectors[k].pntbl = writeAddress;
		//Damn, this is actually really complicated.
		/*
		Say you have four polygons making a sector, of vertices:
		[2, 10, 66, 15]
		[2, 10, 11, 14]
		[11, 14, 100, 5]
		[66, 13, 77, 15]
		The polygons we must make for this sector are:
		[0, 1, 2, 3]
		[0, 1, 4, 5]
		[4, 5, 6, 7]
		[2, 8, 9, 3]
		Thusly we must make the vertex table as follows:
		array[0] = 2
		array[1] = 10
		array[2] = 66
		[3] = 15
		[4] = 11
		[5] = 14
		[6] = 100
		[7] = 5
		[8] = 13
		[9] = 77
		What math can we apply to get this result?
		
		First, enter the vertex IDs in an unordered list:
		2 10 66 15 2 10 11 14 11 14 100 5 66 13 77 15
		Then, remove duplicates:
		2 10 66 15 11 14 100 5 13 77
		This is the sector vertex ID table.
		Now, what about the polygons?
		Work back to that list, such that the polygon:
		[2, 10, 66, 15] will look through the ID table made earlier and find:
		[0] is 2, such that the new table receives 0 where there was 2.
		
		Now, when we draw with this re-ordered list, we have to:
		1. Transform all vertices in the sector vertex ID table in the order they are in the table
		2. This draws at entry [9] in the vertex buffer from vertex #77 in the mesh pntbl
		3. The sector pltbl references [9] in the vertex buffer, which is an alias for vertex #77, its original vertex.
		*/
		unsigned int numVerts = 0;
		for(unsigned int i = 0; i < sectors[k].nbPolygon; i++)
		{
			//This is going to be pretty naive.
			slapBuffer[numVerts] = mesh->pltbl[sectors[k].pltbl[i]].vertices[0];
			clapBuffer[numVerts] = INVALID_PLANE;
			numVerts++;
			slapBuffer[numVerts] = mesh->pltbl[sectors[k].pltbl[i]].vertices[1];
			clapBuffer[numVerts] = INVALID_PLANE;
			numVerts++;
			slapBuffer[numVerts] = mesh->pltbl[sectors[k].pltbl[i]].vertices[2];
			clapBuffer[numVerts] = INVALID_PLANE;
			numVerts++;
			slapBuffer[numVerts] = mesh->pltbl[sectors[k].pltbl[i]].vertices[3];
			clapBuffer[numVerts] = INVALID_PLANE;
			numVerts++;
		}
		int secondNumVerts = 0;
		int uniqueSet = 0;
		//Sets "uniqueSet", checks the entire "clapBuffer" for unqiue set.
		//If uniqueSet is not in clapBuffer, add to clap buffer, add to total vertex number.
		for(unsigned int i = 0; i < numVerts; i++)
		{
			uniqueSet = slapBuffer[i];
			for(unsigned int l = 0; l < numVerts; l++)
			{
				if(clapBuffer[l] == uniqueSet) uniqueSet = INVALID_PLANE;
			}
			if(uniqueSet != INVALID_PLANE) 
			{
				clapBuffer[secondNumVerts] = uniqueSet;
				secondNumVerts++;
			}
		}
		
		sectors[k].nbPoint = secondNumVerts;
		for(int i = 0; i < secondNumVerts; i++)
		{
			sectors[k].pntbl[i] = clapBuffer[i];
		}
		writeAddress += secondNumVerts;
		//Now we have to go back to the pltbl, and alias it.
		//We alias it by finding the pltbl' "vertices" entries in the sector pntbl.
		sectors[k].tltbl = (_quad *)writeAddress;
		writeAddress += sizeof(_quad) * sectors[k].nbPolygon;
		
		for(unsigned int i = 0; i < sectors[k].nbPolygon; i++)
		{
			for(int p = 0; p < 4; p++)
			{
				for(unsigned int f = 0; f < sectors[k].nbPoint; f++)
				{
					if(mesh->pltbl[sectors[k].pltbl[i]].vertices[p] == sectors[k].pntbl[f])
					{
						//What this should do is:
						//Polygon has vertex ID #255, which is sector vertex ID #0, sector polygon vertex ID of #255 becomes #0
						sectors[k].tltbl[i].vertices[p] = f;
					}
				}
			}
		}
		
		sectors_made++;
		sectored_polygons += sectors[k].nbPolygon;
		sectored_verts += sectors[k].nbPoint;
	}
	
	//For address management purposes, we're going to do a very similar loop again.
	//In this case, we're looking specifically for the portals.
	writeAddress = align_4(writeAddress);
	for(unsigned int k = 0; k < MAX_SECTORS; k++)
	{
		sectors[k].nbPortal = 0;
		sectors[k].portals = (unsigned short *)writeAddress;
		for(unsigned int i = 0; i < mesh->nbPolygon; i++)
		{
			plane_sector = mesh->attbl[i].first_sector;
			if(plane_sector != k) continue;
			//If this is NOT a portal, we do not want to concern ourselves with it right now. Don't add it.
			if(!(mesh->attbl[i].render_data_flags & GV_FLAG_PORTAL)) continue;
			
			sectors[k].portals[sectors[k].nbPortal] = i;
			sectors[k].nbPortal++;			
		}
	writeAddress+= sectors[k].nbPortal * sizeof(unsigned short);
	}
	
	//For optimization purposes, we should accrue a center and radius of the sector.
	//The center shall first be placed at model-space; later the game engine will update it to world-space.
	_sector * sct;
	int accumulated_verts[3] = {0,0,0};
	int maxAxis[3] = {0,0,0};
	for(unsigned int s = 0; s < MAX_SECTORS; s++)
	{
		sct = &sectors[s];
		if(sct->nbPoint == 0 || sct->nbPolygon == 0) continue;
		accumulated_verts[X] = 0;
		accumulated_verts[Y] = 0;
		accumulated_verts[Z] = 0;
		maxAxis[X] = 0;
		maxAxis[Y] = 0;
		maxAxis[Z] = 0;
		
		for(unsigned int i = 0; i < sct->nbPoint; i++)
		{
			int alias = sct->pntbl[i];
			
			accumulated_verts[X] += (mesh->pntbl[alias][X]>>16);
			accumulated_verts[Y] += (mesh->pntbl[alias][Y]>>16);
			accumulated_verts[Z] += (mesh->pntbl[alias][Z]>>16);
		}
		
		accumulated_verts[X] = (accumulated_verts[X] / sct->nbPoint)<<16;
		accumulated_verts[Y] = (accumulated_verts[Y] / sct->nbPoint)<<16;
		accumulated_verts[Z] = (accumulated_verts[Z] / sct->nbPoint)<<16;
		//We must negate the applied value due to the inversion of the projection space.
		sct->center_pos[X] = -accumulated_verts[X];
		sct->center_pos[Y] = -accumulated_verts[Y];
		sct->center_pos[Z] = -accumulated_verts[Z];
		
		for(unsigned int i = 0; i < sct->nbPoint; i++)
		{
			int alias = sct->pntbl[i];
			
			accumulated_verts[X] = JO_ABS(mesh->pntbl[alias][X] + sct->center_pos[X]);
			accumulated_verts[Y] = JO_ABS(mesh->pntbl[alias][Y] + sct->center_pos[Y]);
			accumulated_verts[Z] = JO_ABS(mesh->pntbl[alias][Z] + sct->center_pos[Z]);
			
			maxAxis[X] = (accumulated_verts[X] > maxAxis[X]) ? accumulated_verts[X] : maxAxis[X];
			maxAxis[Y] = (accumulated_verts[Y] > maxAxis[Y]) ? accumulated_verts[Y] : maxAxis[Y];
			maxAxis[Z] = (accumulated_verts[Z] > maxAxis[Z]) ? accumulated_verts[Z] : maxAxis[Z];
			
		}
		//Add a margin to it.
		sct->radius[X] = maxAxis[X] + (64<<16);
		sct->radius[Y] = maxAxis[Y] + (64<<16);
		sct->radius[Z] = maxAxis[Z] + (64<<16);
		
	}

	nbg_sprintf(1, 7, "sct(%i),ply(%i),vts(%i)", sectors_made, sectored_polygons, sectored_verts);
	workAddress = (void *)writeAddress;
	return align_4(workAddress);
}

/**
Modified by ponut for madness
**/
unsigned char setTextures(entity_t * model, short baseTexture)
{
	gvAtr smpAttr;
	short maxTex = 0;
	
	model->base_texture = baseTexture;
	
	for(unsigned int i = 0; i < model->pol->nbPolygon; i++)
	{
		smpAttr = model->pol->attbl[i];
		maxTex = (maxTex < smpAttr.texno) ? smpAttr.texno : maxTex;
	}
	
	for(unsigned int i = 0; i < model->pol->nbPolygon; i++)
	{
		smpAttr = model->pol->attbl[i];
		
		if(!(smpAttr.render_data_flags & GV_FLAG_ANIM) && !(smpAttr.render_data_flags & GV_FLAG_PORTAL))
		{
		model->pol->attbl[i].texno += baseTexture;
		} else if(smpAttr.render_data_flags & GV_FLAG_ANIM)
		{
		model->pol->attbl[i].texno = animated_texture_list[smpAttr.texno];
		}
	}

	return maxTex;
}

//Gets texture information from small headers, and sends texture data to VRAM.
void * loadTextures(void * workAddress, entity_t * model)
{
	
	//unsigned short * debug_addr = (unsigned short *)workAddress;
	unsigned char * readByte = (unsigned char *)workAddress;
	unsigned char tHeight = 0;
	unsigned char tWidth = 0;
	unsigned int tSize = 0;
	//nbg_sprintf(0, 14, "(%i)", model->numTexture);
	// nbg_sprintf(0, 15, "(%i)", debug_addr[0]);
	for(int j = 0; j < model->numTexture+1; j++)
	{
		readByte+=2;	//Skip over a boundary short word, 0xF7F7
		tHeight = readByte[0];
		tWidth = readByte[1];
		tSize = tHeight * tWidth;
		readByte += 2; //Skip over the H x W bytes
		GLOBAL_img_addr = readByte;
		add_texture_to_vram((unsigned short)tHeight, (unsigned short)tWidth);
		readByte += tSize;
	}
	return (void*)readByte;
}

void * unpackTextures(void * workAddress, entity_t * model)
{
	// This function is for UV cutting a 64x64 (large) texture or for UV tiling an 8x8 (small) texture.
	//unsigned short * debug_addr = (unsigned short *)workAddress;
	unsigned char * readByte = (unsigned char *)workAddress;
	unsigned char tHeight = 0;
	unsigned char tWidth = 0;
	unsigned int tSize = 0;
	//nbg_sprintf(0, 14, "(%i)", model->numTexture);
	// nbg_sprintf(0, 15, "(%i)", debug_addr[0]);
	for(int j = 0; j < model->numTexture+1; j++)
	{
		readByte+=2;	//Skip over a boundary short word, 0xF7F7
		tHeight = readByte[0];
		tWidth = readByte[1];
		tSize = tHeight * tWidth;
		readByte += 2; //Skip over the H x W bytes
		GLOBAL_img_addr = readByte;
		if(tWidth == 64 || tWidth == 32)
		{
			// UV cut it
			uv_cut(readByte, tWidth, tHeight);
		} else if(tWidth == 8)
		{
			// UV tile it (or at least try to)
			uv_tile(readByte, tWidth, tHeight);
		}
		readByte += tSize;
	}
	return (void*)readByte;
}

void * loadAnimations(void * startAddress, entity_t * model, modelData_t * modelData)
{
    void * workAddress = startAddress;
    unsigned int a; //, ii;

    for (a=0; a<modelData->nbFrames; a++) 
    {
        model->animation[a]=(anim_struct*)(workAddress);
        workAddress=(void*)(workAddress+sizeof(anim_struct));

        unsigned int totPoints=0;
        unsigned int totNormals=0;


            totPoints += model->pol->nbPoint;
            totNormals += model->pol->nbPolygon;

        {

            model->animation[a]->cVert = (compVert*)(workAddress);
            workAddress=(void*)(workAddress+(sizeof(compVert) * totPoints));

            if (totPoints % 2 != 0){
               workAddress=(void*)(workAddress+(sizeof(short)));
			}
            model->animation[a]->cNorm = (compNorm*)(workAddress);
            workAddress=(void*)(workAddress+(sizeof(compNorm) * totNormals));
            while (totNormals % 4 != 0)
            {
                workAddress=(void*)(workAddress+(sizeof(char)));
                totNormals++;
            }

        }
    }

    return workAddress;

}

//i hope xl2 never looks at this weird mutant mess i've made
void * loadGVPLY(void * startAddress, entity_t * model)
{
    void * workAddress = startAddress;

        model->pol=(GVPLY*)workAddress;
        workAddress=(void*)(workAddress + sizeof(GVPLY));
        model->pol->pntbl = (POINT*)workAddress;
        workAddress=(void*)(workAddress + (sizeof(POINT) * model->pol->nbPoint));
        model->pol->pltbl = (_quad*)workAddress;
        workAddress=(void*)(workAddress + (sizeof(_quad) * model->pol->nbPolygon));
		model->pol->nmtbl = (POINT*)workAddress;
        workAddress=(void*)(workAddress + (sizeof(POINT) * model->pol->nbPolygon));
        model->pol->attbl = (gvAtr*)workAddress;
        workAddress=(void*)(workAddress + (sizeof(gvAtr) * model->pol->nbPolygon));
		
		//////////////////////////////////////////////////
		model->pol->maxtbl = (unsigned char *)workAddress;
        workAddress=(void*)(workAddress + (sizeof(unsigned char) * model->pol->nbPolygon));
		
        model->pol->lumatbl = (unsigned char*)workAddress;
		workAddress=(void*)(workAddress + (sizeof(unsigned char) * model->pol->nbPolygon));
		
		//Padding: This has to be 4-bytes aligned.
		//The converter tool will write 4-bytes in case of it being misaligned here.
		workAddress += ((unsigned int)workAddress & 1) ? 1 : 0;
		workAddress += ((unsigned int)workAddress & 2) ? 2 : 0;
		
    return workAddress;
}

void * gvLoad3Dmodel(Sint8 * filename, void * startAddress, entity_t * model, unsigned short sortType, char modelType, entity_t * src_tex_model)
{
	nbg_sprintf(2, 2, "%s", filename);
	modelData_t * model_header;
	void * workAddress = align_4(startAddress);

	model->type = modelType;
	GfsHn gfs_mdat;
	Sint32 sector_count;
	Sint32 file_size;
	
	Sint32 local_name = GFS_NameToId(filename);

//Open GFS
	gfs_mdat = GFS_Open((Sint32)local_name);
//Get sectors
	GFS_GetFileSize(gfs_mdat, NULL, &sector_count, NULL);
	GFS_GetFileInfo(gfs_mdat, NULL, NULL, &file_size, NULL);
	
	GFS_Close(gfs_mdat);
	
	GFS_Load(local_name, 0, (Uint32 *)workAddress, file_size);
	
	GFS_Close(gfs_mdat);
	
	// slDMACopy(workAddress, &model_header, sizeof(modelData_t));
	model_header = (modelData_t *)workAddress;

	//Needed to load/play animations correctly
	model->nbFrames = model_header->nbFrames;
	
	Sint32 bytesOff = (sizeof(modelData_t));
//Add the texture size and the binary meta data size to the work address to reach the model data	
	workAddress = (workAddress + bytesOff); 
	
	model->size = (unsigned int)workAddress;
	workAddress = loadGVPLY((workAddress), model);
	model->size = (unsigned int)workAddress - model->size;
	
	//Zero out the luma table
	for(unsigned int i = 0; i < model->pol->nbPolygon; i++)
	{
		model->pol->lumatbl[i] = 0;
	}

	int baseTex = numTex; //numTex is a tga.c directive
	if(src_tex_model != NULL) 
	{
	baseTex = src_tex_model->base_texture;
	model->numTexture = src_tex_model->numTexture;
	setTextures(model, baseTex); 
	} else {
	model->numTexture = setTextures(model, baseTex); 
	}
	
    workAddress = loadAnimations(workAddress, model, model_header);

	unsigned char * readByte = workAddress;
	if(model->type == MODEL_TYPE_TPACK)
	{
		readByte = unpackTextures(workAddress, model);
	} else if(model->type != MODEL_TYPE_BUILDING && model->type != MODEL_TYPE_SECTORED)
	{
		readByte = loadTextures(workAddress, model);
	}
	////////////////
	//	Most model types		-> Allowed to include raw textures, with no processing.
	//	Model type TPACK		-> Building texture pack. Textures from these are UV-cut or UV-combined.
	//	Model type BUILDING		-> Building model. Not allowed to add new textures. Can declare objects from its payload.
	////////////////
	if(model->type == MODEL_TYPE_BUILDING || model->type == MODEL_TYPE_SECTORED)
	{
		unsigned char * total_items = &readByte[0];
		//unsigned char * unique_items = &readByte[1];
		short * item_data = (short *)&readByte[2];	
		
		/////////////////////////////////////////////
		// Item Data Payload
		// It is appended at the end of the binary, past the textures.
		// It is copied out of this region for permanent use in the BuildingPayload struct.
		// It's order is:
		// 0 byte: total items
		// 1 byte: unique items
		// every 8 bytes after
		// item number, x, y, z, position (relative to entity) as 16-bit int
		/////////////////////////////////////////////
		for(int q = 0; q < *total_items; q++)
		{
			BuildingPayload[total_building_payload].object_type = *item_data++;
			BuildingPayload[total_building_payload].pos[X] = *item_data++;
			BuildingPayload[total_building_payload].pos[Y] = *item_data++;
			BuildingPayload[total_building_payload].pos[Z] = *item_data++;
			BuildingPayload[total_building_payload].sector = *item_data++;
			//Some way to find what entity # we're working with right now
			BuildingPayload[total_building_payload].root_entity = (unsigned short)(model - entities);
			total_building_payload++;
		// nbg_sprintf(1, 20+q, "item(%i)", BuildingPayload[q].object_type);
		// nbg_sprintf(16, 20+q, "item(%i)", BuildingPayload[q].root_entity);
		// nbg_sprintf(1, 15+q, "x(%i)", BuildingPayload[q].pos[X]);
		// nbg_sprintf(13, 15+q, "y(%i)", BuildingPayload[q].pos[Y]);
		// nbg_sprintf(26, 15+q, "z(%i)", BuildingPayload[q].pos[Z]);
		}
		
		// nbg_sprintf(1, 11, "uitem(%i)", *total_items);
		// nbg_sprintf(1, 13, "amnti(%i)", *unique_items);
		
		if(model->type == MODEL_TYPE_SECTORED)
		{
			workAddress = load_sectors(model, workAddress);			
		}
	} 

	
	//////////////////////////////////////////////////////////////////////
	// Set radius
	//////////////////////////////////////////////////////////////////////
	model->radius[X] = model_header->radius[X];
	model->radius[Z] = model_header->radius[Z];
	model->radius[Y] = model_header->radius[Y];
	//NOTE: We do NOT add the size of textures to the work address pointer.
	//The textures are at the end of the GVP payload and have no need to stay in work RAM. They are in VRAM.
	
	// jo_printf(0, 9, "(%i)H", tHeight);
	// jo_printf(0, 10, "(%i)W", tWidth);
	// jo_printf(0, 11, "(%i)T", tSize);
		if(sortType != 0)
		{
	for(unsigned int i = 0; i < model->pol->nbPolygon; i++)
	{
		//Decimate existing sort type bits
	model->pol->attbl[0].render_data_flags &= 0xFFCF;
		//Inject new sort type bits
	model->pol->attbl[0].render_data_flags |= sortType;
		//New render path only reads first attbl for sorting
	}
		}
	
	model->file_done = true;
	model->was_loaded_from_CD = true;
	
	//Alignment
	return align_4(workAddress);
}

void	init_entity_list(void)
{
	int bytes_to_clear = sizeof(entity_t) * MAX_MODELS;
	unsigned char * byte_pointer = (void*)&entities[0];
	
	for(int i = 0; i < bytes_to_clear; i++)
	{
		byte_pointer[i] = 0;
	}
	
	for(int i = 0; i < MAX_MODELS; i++)
	{
		entities[i].pol = NULL;
	}
	
}

