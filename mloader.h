#pragma once

#define MAX_MODELS (64)
#define MAX_SECTORS (64)
#define INVALID_PLANE		(0xFFFF)
#define INVALID_SECTOR		(MAX_SECTORS)
#define MODEL_TYPE_NORMAL ('N')
#define MODEL_TYPE_PLAYER ('P')
#define MODEL_TYPE_BUILDING ('B')
#define MODEL_TYPE_SECTORED	('S')
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
Tile information is formatted the same as plane information, except it is regarding the subdivisions of the root plane (tiles).
Tiles are further subdivided.

uv_id: The base texture (in the UV cut list) that the tiles, absent of subdivisions, receives.
This is important for determining which texture IDs correspond to subdivisions of the tile.
*/

typedef struct {
	unsigned short render_data_flags;
	unsigned char tile_information;
	unsigned char plane_information;
	unsigned char uv_id;
	unsigned char first_sector;
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

typedef struct {
	unsigned short adjacent_plane_id[4];
	int guidance_points[4][3];
} _pathGuide;

typedef struct
{
	int z_plane;	//Boolean. 0 for far, 1 for near.
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
	_pathGuide * pathGuides;
} entity_t;


/////////////////////////////////
// Sector Data
/////////////////////////////////
typedef struct 
{
	entity_t * ent;				//																								0
	unsigned short nbPolygon; //# of polygons (planes) in the sector (for collision)											4
	unsigned short nbPoint; //# of points in the sector (for collision)															6
	unsigned short nbTileVert; //# of vertices for the tiles (to draw) in the sector											8
	unsigned short nbVisible; //# of sectors which are visible (drawn) from this sector											10
	unsigned short * nbTile; //# of tiles (polygons to draw) in each plane of sector, of size <nbPolygon>						12
	unsigned short * plStart; //Starting tile # in each plane of sector, of size <nbPolygon>									16
	unsigned short * pltbl;  //Stores the polygon IDs from <entity> which are in this sector; of size <nbPolygon>				20
	unsigned short * pntbl; //Stores the vertex IDs from <entity> which are in this sector; of size <nbPoint>					24
	unsigned short * pvs; //Stores the sector IDs from <entitiy> which are drawn from this sector; of size <nbVisible>			28
	unsigned short * altbl; //Stores the polygon ID alias of the tiles from <entity> in the sector; of size <nbTile>			32
	_quad * tltbl; //Stores the sector-specific vertex IDs used to draw the sector's tiles										36
	POINT * tvtbl; //Stores the sector-specific vertices used to draw the sector's tiles										40
	unsigned short * portals; //Stores the polygon IDs in ent->pol->attbl and ent->pol->pltbl and ent->pol->pntbl				44
	void * viewspace_tvtbl; //Memory allocated for the viewspace transform of the tvtbl	(sized of vertex_t)						48
	void * scrnspace_tvtbl; //Memory allocated for the screenspace transform of tvtbl (sized of vertex_t)						52
	unsigned short nbPortal; //Stores the # of portals in the sector															54
	unsigned short nbAdjacent; //# of sectors which are primary adjacent to this sector (immediately, physically touching)		56
	volatile unsigned short ready_this_frame; //Boolean; 0 if sector is not ready to draw this frame, 1 if it is.				58
	volatile unsigned short draw_this_frame; //Boolean; 0 if sector will be not be drawn this frame at all, 1 if it will be.	60
	volatile unsigned short	used_port_ct; //# of portals used to draw this portal this frame.									62
} _sector;
extern _sector sectors[MAX_SECTORS+1];

/**Store all your PDATA meshes here**/
extern entity_t entities[MAX_MODELS];

void	*	load_sectors(entity_t * ent, void * workAddress);

void * loadPDATA(void * startAddress, entity_t * model);

unsigned char setTextures(entity_t * model, short baseTexture);

void * loadAnimations(void * startAddress, entity_t * model, modelData_t * modelData);

void * loadTextures(void * workAddress, entity_t * model);

/** This function loads a 3d polygonal model. Returns the last address in RAM (to allow loading multiple meshes)**/
void * gvLoad3Dmodel(Sint8 * filename, void * startAddress, entity_t * model, unsigned short sortType, char modelType, entity_t * src_tex_model);


void	init_entity_list(void);


