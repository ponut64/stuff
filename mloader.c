#include "mloader.h"

entity_t entities[MAX_MODELS];
unsigned int gouraudCounter;

/**
Modified by ponut for madness
**/
void setTextures(entity_t * model, short baseTexture, char * numTexture)
{
	ATTR smpAttr;
	short maxTex = 0;
	
	model->base_texture = baseTexture;
	
	for(int i = 0; i < model->pol[0]->nbPolygon; i++)
	{
		smpAttr = model->pol[0]->attbl[i];
		maxTex = (maxTex < smpAttr.texno) ? smpAttr.texno : maxTex;
	}
	
	for(int i = 0; i < model->pol[0]->nbPolygon; i++)
	{
		smpAttr = model->pol[0]->attbl[i];
		
		model->pol[0]->attbl[i].texno += baseTexture;
	}

	*numTexture = maxTex;
}


Uint16 loadTextures(void * startAddress, modelData_t * modelData)
{
	/** Empty **/
	return 0;
}

void * loadAnimations(void * startAddress, entity_t * model, modelData_t * modelData)
{
    void * workAddress = startAddress;
    unsigned int a, i; //, ii;

    for (a=0; a<modelData->nbFrames; a++)  /**NEED TO DIVIDE NBFRAMES BY INTERPOLATION**/
    {
        model->animation[a]=(anim_struct*)(workAddress);
        workAddress=(void*)(workAddress+sizeof(anim_struct));

        unsigned int totPoints=0;
        unsigned int totNormals=0;

        for (i=0; i<model->nbMeshes; i++)
        {
            totPoints+=model->pol[i]->nbPoint;
            totNormals+=model->pol[i]->nbPolygon;
        }
        {

            model->animation[a]->cVert = (compVert*)(workAddress);
            workAddress=(void*)(workAddress+(sizeof(compVert) * totPoints));

            if (totPoints % 2 != 0)
               workAddress=(void*)(workAddress+(sizeof(short)));

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

void * loadPDATA(void * startAddress, entity_t * model, modelData_t * modelData)
{
    void * workAddress = startAddress;
    unsigned int i;

    for (i=0; i<modelData->TOTAL_MESH; i++)
    {
        model->pol[i]=(PDATA*)workAddress;
        workAddress=(void*)(workAddress + sizeof(PDATA));
        model->pol[i]->pntbl = (POINT*)workAddress;
        workAddress=(void*)(workAddress + (sizeof(POINT) * model->pol[i]->nbPoint));
        model->pol[i]->pltbl = (POLYGON*)workAddress;
        workAddress=(void*)(workAddress + (sizeof(POLYGON) * model->pol[i]->nbPolygon));
        model->pol[i]->attbl = (ATTR*)workAddress;
        workAddress=(void*)(workAddress + (sizeof(ATTR) * model->pol[i]->nbPolygon));
		//Model is of PDATA type, there are no per-vertice normals for gouraud shading
    }

    return workAddress;
}

void * gvLoad3Dmodel(Sint8 * filename, void * startAddress, entity_t * model, unsigned short sortType)
{
	/** Not useful anymore due to ZTP changes, must be redone **/
	modelData_t model_header;
	void * workAddress = startAddress;
	
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
	
	slDMACopy(workAddress, &model_header, sizeof(modelData_t));
	//ADDED
    model->nbMeshes = model_header.TOTAL_MESH;
	model->nbFrames = model_header.nbFrames;
	
	//Uint16 first_texture = loadTextures(workAddress, &model_header);
	Sint32 bytesOff = (sizeof(modelData_t)); 
	workAddress = (workAddress + bytesOff); //Add the texture size and the binary meta data size to the work address to reach the PDATA
	
	model->size = (unsigned int)workAddress;
	workAddress = loadPDATA((workAddress), model, &model_header);
	model->size = (unsigned int)workAddress - model->size;


	setTextures(model, numTex, &model->numTexture); //numTex is a tga.c directive
    workAddress = loadAnimations(workAddress, model, &model_header);
	
	unsigned short * uAddr = (unsigned short *)workAddress;
	unsigned char * readByte = (unsigned char *)workAddress;
	unsigned char tHeight = 0;
	unsigned char tWidth = 0;
	unsigned int tSize = 0;
	// jo_printf(0, 14, "(%i)", model->numTexture);
	// jo_printf(0, 15, "(%i)", uAddr[0]);
	for(int j = 0; j < model->numTexture+1; j++)
	{
		readByte+=2;	//Skip over a boundary short word, 0xF7F7
		tHeight = readByte[0];
		tWidth = readByte[1];
		tSize = tHeight * tWidth;
		readByte += 2; //Skip over the H x W bytes
		GLOBAL_img_addr = readByte;
		add_texture_to_vram((unsigned short)tHeight, (unsigned short)tWidth);
		readByte += tSize; //Get us to the next texture
	}
	
	//NOTE: We do NOT add the size of textures to the work address pointer.
	//The textures are at the end of the GVP payload and have no need to stay in work RAM. They are in VRAM.
	
	// jo_printf(0, 9, "(%i)H", tHeight);
	// jo_printf(0, 10, "(%i)W", tWidth);
	// jo_printf(0, 11, "(%i)T", tSize);
	
		//Decimate existing sort type bits
	model->pol[0]->attbl[0].sort &= 252;
		//Inject new sort type bits
	model->pol[0]->attbl[0].sort |= sortType;
		//New render path only reads first attbl for sorting
	
	model->file_done = true;
	
	return workAddress;
}
