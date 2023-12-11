#pragma once

#define MAX_MODELS (192)
#define MODEL_TYPE_NORMAL ('N')
#define MODEL_TYPE_PLAYER ('P')
#define MODEL_TYPE_BUILDING ('B')
#define MODEL_TYPE_UNDEFINED ('F')
#define MODEL_TYPE_TPACK	('T')

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
//
// You need to be especially careful with alignment here.
// Two bytes must be two byte aligned. Four bytes must be four bytes aligned.
// Structs with any ints (four byte access) into them will be padded to four bytes alignment for them.
// So if you have one int and one short in a struct, it's going to be padded to 8 bytes.
//
*/

typedef struct
{
    unsigned short	nbFrames; //Number of keyframes
	unsigned short	radius[XYZ];
	unsigned int	first_portal; //Polygon # of the first portal in the mesh
} modelData_t;

/*
Portal information:
Polygon ID of the next portal. 
If it is 255, this polygon is not a portal.
If it is 254, this is the last portal.

Plane information:
Information about the scale and subdivision rules of the plane.
0-1: First subdivision rule
2-3: Second subdivision rule
4-5: Third subdivision rule
6-7: ???
*/

typedef struct {
	unsigned short render_data_flags;
	unsigned char plane_information;
	unsigned char uv_id;
	unsigned char first_sector;
	unsigned char second_sector;
	unsigned short texno;
} gvAtr;

//Struct which stores the vertex indexes of a polygon
//The entries here are for the pntbl array.
typedef struct {
	unsigned short vertices[4];
} _quad;

typedef struct {
    unsigned int nbPoint;		/* Number of vertices */
    unsigned int nbPolygon;		/* Number of polygons */
    POINT *		pntbl;			/* Vertex position data table */
    _quad *		pltbl;			/* Polygon definition table */
	POINT *		nmtbl;			/* Normal definition table */
    gvAtr *		attbl;			/* The attribute table for the polygon */
	unsigned char * maxtbl;		/* Major axis table */
	unsigned char * lumatbl;	/* Lighting table, as <<9 units. */
} GVPLY ;

typedef struct
{
	unsigned int size;
	short file_done;
	short was_loaded_from_CD;
	short base_texture;
	short useClip;		//To clip by system, in user, or outside of user.
	unsigned short radius[XYZ];
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
void * gvLoad3Dmodel(Sint8 * filename, void * startAddress, entity_t * model, unsigned short sortType, char modelType, entity_t * src_tex_model);


void	init_entity_list(void);


