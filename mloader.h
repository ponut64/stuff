#ifndef __MLOADER_H__
# define __MLOADER_H__

#include <jo/jo.h>
#include "tga.h"

//Mostly from XL2

#define MAX_MESHES (5)
#define MAX_MODELS (40)

typedef     Sint16	compVert[XYZ];
typedef     Uint8   compNorm;

typedef struct
{
 compVert * cVert;
 compNorm * cNorm;

} anim_struct;

typedef struct
{
    unsigned short  TOTAL_MESH; //total amount of PDATA
    short           TOT_TEXT;  //total amount of textures
    unsigned int    PDATA_SIZE; //to quickly load from disk, total size of pdata in bytes
    unsigned int    TEXT_SIZE;  //to quickly load from disk, that's the size of the textures in bytes
    unsigned short nbFrames;
    anim_struct *  animation;
} modelData_t;

typedef struct
{
	bool file_done;
	unsigned int size;
	short base_texture;
	char numTexture;
	char sortType;
    Uint16 nbMeshes;
    Uint16 nbFrames;
    anim_struct * animation[240];
    PDATA * pol[MAX_MESHES];
} entity_t;


/**Store all your PDATA meshes here**/
extern entity_t entities[MAX_MODELS];
extern unsigned int gouraudCounter;



void setTextures(entity_t * model, short baseTexture, char * numTexture);

Uint16 loadTextures(void * startAddress, modelData_t * modelData);

void * loadPDATA(void * startAddress, entity_t * model, modelData_t * modelData);

void * loadAnimations(void * startAddress, entity_t * model, modelData_t * modelData);

/** This function loads a 3d polygonal model. Returns the last address in LWRAM (to allow loading multiple meshes)**/
void *	gvLoad3Dmodel(Sint8 * filename, void * startAddress, entity_t * model, unsigned short sortType);


#endif 

