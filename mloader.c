
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
		for(unsigned int i = 0; i < mesh->nbPolygon; i++)
		{
			plane_sector = mesh->attbl[i].first_sector;
			if(plane_sector != k) continue;
			//If this is a portal, don't add it to the polygon list. It's added elsewhere.
			if(mesh->attbl[i].render_data_flags & GV_FLAG_PORTAL) continue;
			
			sectors[k].pltbl[sectors[k].nbPolygon] = i;
			sectors[k].nbPolygon++;			
		}
		if(!sectors[k].nbPolygon) continue; 
		sectors[k].ent = ent;
		writeAddress += sectors[k].nbPolygon;
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
		if(sct->nbPoint == 0) continue;
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

	model->first_portal = (unsigned char)model_header->first_portal;

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
	
}

