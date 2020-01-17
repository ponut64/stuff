#include "ZT_COMMON.H"
#include "ZT_LOAD_MODEL.H"
#include <jo/jo.h>

entity_t entities[MAX_MODELS];
unsigned int gouraudCounter;

/**
Modified by ponut for madness
**/
void setTextures(entity_t * model, short baseTexture, char * numTexture)
{
	ATTR smpAttr;
	short maxTex = 0;
	
	for(int i = 0; i < model->pol[0]->nbPolygon; i++)
	{
		smpAttr = model->pol[0]->attbl[i];
		maxTex = (maxTex < smpAttr.texno) ? smpAttr.texno : maxTex;
	}
	
	for(int i = 0; i < model->pol[0]->nbPolygon; i++)
	{
		smpAttr = model->pol[0]->attbl[i];
		
//ATTR bufAttr = ATTRIBUTE(smpAttr.flag, smpAttr.sort&0x03 , (smpAttr.texno)+baseTexture, 0, No_Gouraud,Window_In|MESHoff|HSSon|ECdis | SPdis |CL64Bnk, smpAttr.dir, UseNearClip);
//model->pol[0]->attbl[i] = bufAttr;
model->pol[0]->attbl[i].texno += baseTexture;
	}
	

	*numTexture = maxTex;
}


Uint16 loadTextures(void * startAddress, modelData_t * modelData)
{
	    int first_sprite = jo_get_last_sprite_id();
     int i;
    jo_texture_definition   *texture;
    jo_img *  pimg[modelData->TOT_TEXT];
    void * workAddress = (void*)(startAddress+sizeof(modelData_t));


    for (i=0; i<modelData->TOT_TEXT; ++i)
    {
        pimg[i] = (jo_img *)workAddress;
        workAddress=(void*)(workAddress+12);
        pimg[i]->width=pimg[i]->width/4;  //Quick and dirty way to get around the fact that Jo Engine doesn't support 4 bits pixel data.
        pimg[i]->data = (Uint16*)(workAddress);
        workAddress=(void*)(workAddress+(sizeof(Uint16)*pimg[i]->width * pimg[i]->height));

        int id = jo_sprite_add(pimg[i]);

       
        texture=&__jo_sprite_def[id];
        texture->width=pimg[i]->width<<2;  //Ghetto technique for compatibility with Jo Engine, but trying to replace the sprite will throw an error
        __jo_sprite_pic[id].color_mode=COL_16;
        texture->size = JO_MULT_BY_32(texture->width & 0x1f8) | texture->height;
    }
    slDMACopy(workAddress, (void*)(returnLUTaddr((Uint16)(first_sprite+1))), sizeof(Uint16)*16 * modelData->TOT_TEXT);
    slDMAWait();
 
    return (Uint16)(first_sprite+1);
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

void * ztLoad3Dmodel(Sint8 * filename, void * startAddress, entity_t * model, Bool UseRealtimeGouraud)
{
    //memset_l((void*)startAddress,0x0000, (0xFFFE - ((Uint32)startAddress-(Uint32)LWRAM)));  //Not 100% necessary, since data can just be overwritten, but good for testing and see how much data a level takes
    void * workAddress;
    workAddress=startAddress;

    /**Load the file header and map data**/
    modelData_t bufModel; //If you wish to keep the model info elsewhere, you can do it
    void * ptr = &bufModel;
    Sint32 fid = GFS_NameToId((Sint8*)filename);
	GfsHn gfs_ztp;
	
	gfs_ztp = GFS_Open((Sint32)fid);
	GFS_GetFileInfo(gfs_ztp, NULL, NULL, (Sint32*)&model->size, NULL);
	GFS_Close(gfs_ztp);
	
    GFS_Load(fid, 0, (Uint32 *)startAddress, (sizeof(modelData_t)));
    memcpy_l((Sint32*)ptr, (Sint32*)(startAddress), (sizeof(modelData_t)));

    /**ADDED**/
    model->nbMeshes=bufModel.TOTAL_MESH;
	model->nbFrames=bufModel.nbFrames;

    /**Turns on the graphics (mainly for debugging, remove it if everything works)**/
 //   slScrAutoDisp(NBG3ON);
   slPrint("NOW LOADING...", slLocate(5, 5));    slPrint((char*)filename, slLocate(5, 6));
   fadeIn();


    /**Load the texture list (using an offset to allow DMA transfer)**/
	slPrint("LOADING TEXTURES", slLocate(5, 8));
    GFS_Load(fid, 0, (void*)startAddress, bufModel.TEXT_SIZE+(sizeof(modelData_t)));
   // Uint16 first_texture = loadTextures(startAddress, &bufModel);


    /**Load PDATA**/
	slPrint("LOADING PDATA   ", slLocate(5, 8));
    Sint32 bytesOff = (bufModel.TEXT_SIZE+(sizeof(modelData_t)))/2048;

    /****Should really just take the filesize here...*****/
    GFS_Load(fid, bytesOff, (void*)startAddress, bufModel.PDATA_SIZE + 2048 + (1024*128));
    bytesOff = bufModel.TEXT_SIZE+(sizeof(modelData_t)) - (bytesOff*2048);
    workAddress = (void*)(workAddress + bytesOff);

    workAddress = loadPDATA((void*)workAddress, model, &bufModel);


    /**Set textures**/
	slPrint("SETTING TEXTURES", slLocate(5, 8));
  // setTextures(model, bufModel.TOTAL_MESH, UseRealtimeGouraud);

    /**Setting animation data**/
    workAddress = loadAnimations((void*)workAddress, model, &bufModel);

	model->file_done = true;
	jo_clear_screen();

    return workAddress;
}

