#ifndef __MLOADER_H__
# define __MLOADER_H__

#define MAX_MODELS (64)
#define MODEL_TYPE_NORMAL ('N')
#define MODEL_TYPE_PLAYER ('P')
#define MODEL_TYPE_BUILDING ('B')
#define MODEL_TYPE_UNDEFINED ('F')

typedef     Sint16	compVert[XYZ];
typedef     Uint8   compNorm;

typedef struct
{
 compVert * cVert; //Pointer to cVert list, stored in memory for an animated model
 compNorm * cNorm; //Pointed to cNorm list, stored in memory for an animated model

} anim_struct;

/*
**************************** IT WILL CRASH IF IT ISN'T!*********************
**************************** IT WILL CRASH IF IT ISN'T!*********************
****IF YOU EVER MODIFY MODELDATA_T, BE AWARE: IT MUST BE 4-BYTE ALIGNED.****
**************************** IT WILL CRASH IF IT ISN'T!*********************
**************************** IT WILL CRASH IF IT ISN'T!*********************
*/

typedef struct
{
    unsigned short  TOTAL_MESH; //total amount of PDATA
    unsigned short	TOT_TEXT;  //total amount of textures
    unsigned int    PDATA_SIZE; //to quickly load from disk, total size of pdata in bytes
    unsigned int    TEXT_SIZE;  //to quickly load from disk, that's the size of the textures in bytes
    unsigned short	nbFrames; //Number of keyframes
	unsigned short	radius[XYZ];
	unsigned int	first_portal; //Polygon # of the first portal in the mesh
} modelData_t;

/*
Portal information:
Polygon ID of the next portal. 
If it is 255, this polygon is not a portal.
If it is 254, this is the last portal.
*/

typedef struct {
	unsigned char render_data_flags;
	unsigned char portal_information;
	unsigned char first_sector;
	unsigned char second_sector;
	unsigned short texno;
} gvAtr;

typedef struct {
    POINT *		pntbl;		/* Vertex position data table */
    Uint32		nbPoint;		/* Number of vertices */
    POLYGON *	pltbl;		/* Polygon definition table */
    Uint32		nbPolygon;		/* Number of polygons */
    gvAtr *		attbl;		/* The attribute table for the polygon */
} GVPLY ;

typedef struct
{
	Bool file_done;
	unsigned int size;
	short base_texture;
	unsigned short radius[XYZ];
	short useClip;		//To clip by system, in user, or outside of user.
	unsigned char numTexture;
	unsigned char first_portal;
	char sortType;
	char type;
    Uint16 nbMeshes;
    Uint16 nbFrames; // Number of keyframes
    anim_struct * animation[64];
	GVPLY * pol;
	FIXED * prematrix;
} entity_t;

/**Store all your PDATA meshes here**/
extern entity_t entities[MAX_MODELS];

void * loadPDATA(void * startAddress, entity_t * model);

unsigned char setTextures(entity_t * model, short baseTexture);

void * loadAnimations(void * startAddress, entity_t * model, modelData_t * modelData);

void * loadTextures(void * workAddress, entity_t * model);

/** This function loads a 3d polygonal model. Returns the last address in RAM (to allow loading multiple meshes)**/
void * gvLoad3Dmodel(Sint8 * filename, void * startAddress, entity_t * model, unsigned short sortType, char modelType);

void	init_entity_list(void);

#endif 


