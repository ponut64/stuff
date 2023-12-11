
#include <SL_DEF.H>
#include <SEGA_GFS.H>
#include "def.h"
#include "tga.h"
#include "render.h"
#include "bounder.h"
#include "physobjet.h"
#include "mymath.h"

#include "mloader.h"

entity_t entities[MAX_MODELS];

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
		
		if(!(smpAttr.render_data_flags & GV_FLAG_ANIM))
		{
		model->pol->attbl[i].texno += baseTexture;
		} else {
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
	if(modelType == MODEL_TYPE_TPACK)
	{
		readByte = unpackTextures(workAddress, model);
	} else if(modelType != MODEL_TYPE_BUILDING)
	{
		readByte = loadTextures(workAddress, model);
	}
	////////////////
	//	Most model types		-> Allowed to include raw textures, with no processing.
	//	Model type TPACK		-> Building texture pack. Textures from these are UV-cut or UV-combined.
	//	Model type BUILDING		-> Building model. Not allowed to add new textures. Can declare objects from its payload.
	////////////////
	if(model->type == MODEL_TYPE_BUILDING)
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

