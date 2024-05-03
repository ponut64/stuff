//subrender.c
//This file compiled separately

#include <SL_DEF.H>
#include "def.h"
#include "mymath.h"
#include "render.h"
#include "draw.h"
#include "dspm.h"

unsigned short texIDs_cut_from_texID[225][4] = {
	{30, 31, 32, 33}, // +++
	{30, 32, 0, 0}, // -++
	{31, 33, 0, 0}, // -++
	{30, 31, 0, 0}, // |++
	{32, 33, 0, 0}, // |++
	{34, 38, 0, 0}, // --+
	{32, 39, 0, 0}, // --+
	{31, 36, 0, 0}, // --+
	{37, 41, 0, 0}, // --+
	{42, 44, 0, 0}, // ||+
	{43, 45, 0, 0}, // ||+
	{46, 48, 0, 0}, // ||+
	{47, 49, 0, 0}, // ||+
	{50, 58, 0, 0}, // ---
	{51, 59, 0, 0}, // ---
	{52, 60, 0, 0}, // ---
	{53, 61, 0, 0}, // ---
	{54, 62, 0, 0}, // ---
	{55, 63, 0, 0}, // ---
	{56, 64, 0, 0}, // ---
	{57, 65, 0, 0}, // ---
	{66, 70, 0, 0}, // |||
	{67, 71, 0, 0}, // |||
	{68, 72, 0, 0}, // |||
	{69, 73, 0, 0}, // |||
	{74, 78, 0, 0}, // |||
	{75, 79, 0, 0}, // |||
	{76, 80, 0, 0}, // |||
	{77, 81, 0, 0}, // |||
	{82, 83, 86, 87}, // ++
	{84, 85, 88, 89}, // ++
	{90, 91, 94, 95}, // ++
	{92, 93, 96, 97}, // ++
	{82, 86, 0, 0}, // -+
	{83, 87, 0, 0}, // -+
	{84, 88, 0, 0}, // -+
	{85, 89, 0, 0}, // -+
	{90, 94, 0, 0}, // -+
	{91, 95, 0, 0}, // -+
	{92, 96, 0, 0}, // -+
	{93, 97, 0, 0}, // -+
	{82, 83, 0, 0}, // |+
	{86, 87, 0, 0}, // |+
	{84, 85, 0, 0}, // |+
	{88, 89, 0, 0}, // |+
	{90, 91, 0, 0}, // |+
	{94, 95, 0, 0}, // |+
	{92, 93, 0, 0}, // |+
	{96, 97, 0, 0}, // |+
	{98, 99, 0, 0}, // --
	{100, 101, 0, 0}, // --
	{102, 103, 0, 0}, // --
	{104, 105, 0, 0}, // --
	{106, 107, 0, 0}, // --
	{108, 109, 0, 0}, // --
	{110, 111, 0, 0}, // --
	{112, 113, 0, 0}, // --
	{114, 115, 0, 0}, // --
	{116, 117, 0, 0}, // --
	{118, 119, 0, 0}, // --
	{120, 121, 0, 0}, // --
	{122, 123, 0, 0}, // --
	{124, 125, 0, 0}, // --
	{126, 127, 0, 0}, // --
	{128, 129, 0, 0}, // --
	{130, 132, 0, 0}, // ||
	{131, 133, 0, 0}, // ||
	{138, 140, 0, 0}, // ||
	{139, 141, 0, 0}, // ||
	{134, 136, 0, 0}, // ||
	{135, 137, 0, 0}, // ||
	{142, 144, 0, 0}, // ||
	{143, 145, 0, 0}, // ||
	{146, 148, 0, 0}, // ||
	{147, 149, 0, 0}, // ||
	{154, 156, 0, 0}, // ||
	{155, 157, 0, 0}, // ||
	{150, 152, 0, 0}, // ||
	{151, 153, 0, 0}, // ||
	{158, 160, 0, 0}, // ||
	{159, 161, 0, 0}, // ||
	{162, 166, 163, 167}, // +
	{170, 174, 171, 175}, // +
	{178, 182, 179, 183}, // +
	{186, 190, 187, 191}, // +
	{164, 168, 165, 169}, // +
	{172, 176, 173, 177}, // +
	{180, 184, 181, 185}, // +
	{188, 192, 189, 193}, // +
	{194, 198, 195, 199}, // +
	{202, 206, 203, 207}, // +
	{210, 214, 211, 215}, // +
	{218, 222, 219, 223}, // +
	{196, 200, 197, 201}, // +
	{204, 208, 205, 209}, // +
	{212, 216, 213, 217}, // +
	{220, 224, 221, 225}, // +
	{162, 163, 0, 0}, // -
	{164, 165, 0, 0}, // -
	{166, 167, 0, 0}, // -
	{168, 169, 0, 0}, // -
	{170, 171, 0, 0}, // -
	{172, 173, 0, 0}, // -
	{174, 175, 0, 0}, // -
	{176, 177, 0, 0}, // -
	{178, 179, 0, 0}, // -
	{180, 181, 0, 0}, // -
	{182, 183, 0, 0}, // -
	{184, 185, 0, 0}, // -
	{186, 187, 0, 0}, // -
	{188, 189, 0, 0}, // -
	{190, 191, 0, 0}, // -
	{192, 193, 0, 0}, // -
	{194, 195, 0, 0}, // -
	{196, 197, 0, 0}, // -
	{198, 199, 0, 0}, // -
	{200, 201, 0, 0}, // -
	{202, 203, 0, 0}, // -
	{204, 205, 0, 0}, // -
	{206, 207, 0, 0}, // -
	{208, 209, 0, 0}, // -
	{210, 211, 0, 0}, // -
	{212, 213, 0, 0}, // -
	{214, 215, 0, 0}, // -
	{216, 217, 0, 0}, // -
	{218, 219, 0, 0}, // -
	{220, 221, 0, 0}, // -
	{222, 223, 0, 0}, // -
	{224, 225, 0, 0}, // -
	{162, 166, 0, 0}, // |
	{163, 167, 0, 0}, // |
	{170, 174, 0, 0}, // |
	{171, 175, 0, 0}, // |
	{178, 182, 0, 0}, // |
	{179, 183, 0, 0}, // |
	{186, 190, 0, 0}, // |
	{187, 191, 0, 0}, // |
	{164, 168, 0, 0}, // |
	{165, 169, 0, 0}, // |
	{172, 176, 0, 0}, // |
	{173, 177, 0, 0}, // |
	{180, 184, 0, 0}, // |
	{181, 185, 0, 0}, // |
	{188, 192, 0, 0}, // |
	{189, 193, 0, 0}, // |
	{194, 198, 0, 0}, // |
	{195, 199, 0, 0}, // |
	{202, 206, 0, 0}, // |
	{203, 207, 0, 0}, // |
	{210, 214, 0, 0}, // |
	{211, 215, 0, 0}, // |
	{218, 222, 0, 0}, // |
	{219, 223, 0, 0}, // |
	{196, 200, 0, 0}, // |
	{197, 201, 0, 0}, // |
	{204, 208, 0, 0}, // |
	{205, 209, 0, 0}, // |
	{212, 216, 0, 0}, // |
	{213, 217, 0, 0}, // |
	{220, 224, 0, 0}, // |
	{221, 225, 0, 0}  // |
	//(remaining values do not subdivide)
};

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
	#define SUBDIVIDE_X		(1) // |
	#define SUBDIVIDE_Y		(2) // -
	#define SUBDIVIDE_XY	(3) // +
	
	#define SUBDIVIDE_3XY	(0x3F) // +++
	#define SUBDIVIDE_1Y2XY	(0x3E) // -++
	#define SUBDIVIDE_1X2XY	(0x3D) // |++
	#define SUBDIVIDE_2Y1XY	(0x3A) // --+
	#define SUBDIVIDE_2X1XY	(0x35) // ||+
	#define SUBDIVIDE_3Y	(0x2A) // ---
	#define SUBDIVIDE_3X	(0x15) // |||
	
	#define SUBDIVIDE_2XY	(0xF) // ++
	#define SUBDIVIDE_1Y1XY	(0xE) // -+
	#define SUBDIVIDE_1X1XY	(0xD) // |+
	#define SUBDIVIDE_2Y	(0xA) // --
	#define SUBDIVIDE_2X	(0x5) // ||

	#define UV_CUT_COUNT (224)
	
	#define MAX_IN_TILE (128)

int			sub_transform_buffer[MAX_SSH2_ENTITY_VERTICES][4];
vertex_t	screen_transform_buffer[MAX_SSH2_ENTITY_VERTICES];
//Realistically, this could go up to 4^6. That's a lot of RAM.
//Even this being set at 1024 is quite a lot of RAM used.
// 12kb pt buffer + 8kb poly buffer + 2kb tex buffer // 22kb
int			subdivided_points[MAX_IN_TILE][4];
short		subdivided_polygons[MAX_IN_TILE][4]; //4 Vertex IDs of the subdivided_points
short		used_textures[MAX_IN_TILE];

short	sub_poly_cnt = 0;
short	sub_vert_cnt = 0;
short	tile_rules[4]	= {0, 0, 0, SUBDIVIDE_XY};
short	plane_rules[4] = {0, 0, 0, SUBDIVIDE_XY};
short	texture_rules[4]		= {16, 16, 16, 0};
// big performance nob.
int		z_rules[4]				= {512<<16, 256<<16, 128<<16, 0};

int		clip_settings[7] = {TV_HALF_WIDTH, -TV_HALF_WIDTH, TV_HALF_HEIGHT, -TV_HALF_WIDTH, SUBDIVISION_NEAR_PLANE, 0, 0};
//								0					4			8				12					16			   20,24
typedef struct {
	FIXED * ptv[4]; //ptv[0] is byte 0, ptv[1] is byte 4, ptv[2] is byte 8, ptv[3] is byte 12
	short * poly_a; // 	byte 16
	short * poly_b; // 	byte 20
	short * poly_c; // 	byte 24
	short * poly_d; // 	byte 28
} _subdivision_settings;
	
void	subdivide_xy(_subdivision_settings * set)
{
	//////////////////////////////////////////////////////////////////
	// Subdivide by all rules / Subdivide polygon into four new quads
	//Turn 4 points into 9 points
	//Make the 4 new polygons
	//////////////////////////////////////////////////////////////////
	/*
	0A			1A | 0B			1B
							
			A				B
							
	3A			2A | 3B			2B		
	
	0D			1D | 0C			1C
	
			C				D

	3D			2D | 3C			2C
	*/
	// Initial Conditions
	set->poly_a[0] = set->poly_a[0];
	set->poly_b[1] = set->poly_a[1];
	set->poly_c[3] = set->poly_a[3];
	set->poly_d[2] = set->poly_a[2];

	// Center
	// 
	int * live_vt = &subdivided_points[sub_vert_cnt][0];
	live_vt[X] = (set->ptv[0][X] + set->ptv[1][X] + 
										set->ptv[2][X] + set->ptv[3][X])>>2;
	live_vt[Y] = (set->ptv[0][Y] + set->ptv[1][Y] + 
										set->ptv[2][Y] + set->ptv[3][Y])>>2;
	live_vt[Z] = (set->ptv[0][Z] + set->ptv[1][Z] + 
										set->ptv[2][Z] + set->ptv[3][Z])>>2;
	
	set->poly_a[2] = sub_vert_cnt;
	set->poly_b[3] = sub_vert_cnt;
	set->poly_c[1] = sub_vert_cnt;
	set->poly_d[0] = sub_vert_cnt;

	sub_vert_cnt++;
	// 0 -> 1
	//Hm, in the process of making center, we do add 0+1. That could be an optimization -- if it were not for the shifts.
	live_vt = &subdivided_points[sub_vert_cnt][0];
	live_vt[X] = (set->ptv[0][X] + set->ptv[1][X])>>1;
	live_vt[Y] = (set->ptv[0][Y] + set->ptv[1][Y])>>1;
	live_vt[Z] = (set->ptv[0][Z] + set->ptv[1][Z])>>1;
	
	set->poly_a[1] = sub_vert_cnt;
	set->poly_b[0] = sub_vert_cnt;
	sub_vert_cnt++;
	// 1 -> 2
	live_vt = &subdivided_points[sub_vert_cnt][0];
	live_vt[X] = (set->ptv[2][X] + set->ptv[1][X])>>1;
	live_vt[Y] = (set->ptv[2][Y] + set->ptv[1][Y])>>1;
	live_vt[Z] = (set->ptv[2][Z] + set->ptv[1][Z])>>1;
	
	set->poly_b[2] = sub_vert_cnt;
	set->poly_d[1] = sub_vert_cnt;
	sub_vert_cnt++;
	// 3 -> 2
	live_vt = &subdivided_points[sub_vert_cnt][0];
	live_vt[X] = (set->ptv[2][X] + set->ptv[3][X])>>1;
	live_vt[Y] = (set->ptv[2][Y] + set->ptv[3][Y])>>1;
	live_vt[Z] = (set->ptv[2][Z] + set->ptv[3][Z])>>1;
	
	set->poly_c[2] = sub_vert_cnt;
	set->poly_d[3] = sub_vert_cnt;
	sub_vert_cnt++;
	// 3 -> 0
	live_vt = &subdivided_points[sub_vert_cnt][0];
	live_vt[X] = (set->ptv[0][X] + set->ptv[3][X])>>1;
	live_vt[Y] = (set->ptv[0][Y] + set->ptv[3][Y])>>1;
	live_vt[Z] = (set->ptv[0][Z] + set->ptv[3][Z])>>1;
	
	set->poly_a[3] = sub_vert_cnt;
	set->poly_c[0] = sub_vert_cnt;
	sub_vert_cnt++;

	sub_poly_cnt += 3; //Only add 3, as there was already 1 polygon. It was split into four.
	
}

void	subdivide_y(_subdivision_settings * set)
{
	//////////////////////////////////////////////////////////////////
	// Subdivide between the edges 0->1 and 3->2 (""Vertically"")
	// (Splits the polygon such that new vertices are created between 0->3 and 1->2)
	//Turn 4 points into 6 points
	//Make the 2 new polygons
	//////////////////////////////////////////////////////////////////
	/*
	0A							1A		
					A
	3A--------------------------2A		
	0B--------------------------1B
					B
	3B							2B
	*/

	 //Initial Conditions
	set->poly_a[0] = set->poly_a[0];
	set->poly_a[1] = set->poly_a[1];
	set->poly_b[2] = set->poly_a[2];
	set->poly_b[3] = set->poly_a[3];
	
	// 1 -> 2
	int * live_vt = &subdivided_points[sub_vert_cnt][0];
	 live_vt[X] = (set->ptv[2][X] + set->ptv[1][X])>>1;
	 live_vt[Y] = (set->ptv[2][Y] + set->ptv[1][Y])>>1;
	 live_vt[Z] = (set->ptv[2][Z] + set->ptv[1][Z])>>1;
	
	 set->poly_a[2] = sub_vert_cnt;
	 set->poly_b[1] = sub_vert_cnt;
	 sub_vert_cnt++;
	// 3 -> 0
	live_vt = &subdivided_points[sub_vert_cnt][0];
	 live_vt[X] = (set->ptv[0][X] + set->ptv[3][X])>>1;
	 live_vt[Y] = (set->ptv[0][Y] + set->ptv[3][Y])>>1;
	 live_vt[Z] = (set->ptv[0][Z] + set->ptv[3][Z])>>1;
	
	 set->poly_a[3] = sub_vert_cnt;
	 set->poly_b[0] = sub_vert_cnt;
	 sub_vert_cnt++;
	
	 sub_poly_cnt += 1; //Only add 1, as there was already 1 polygon. It was split in two.
	
}

void	subdivide_x(_subdivision_settings * set)
{
	//////////////////////////////////////////////////////////////////
	// Subdivide between the edges 0->3 and 1->2 (""Horizontally"")
	// (Splits the polygon such that new vertices are created between 0->1 and 3->2)
	//Turn 4 points into 6 points
	//Make the 2 new polygons
	//////////////////////////////////////////////////////////////////
	/*
	0A			 1A | 0B			1B
					|		
			A		|		B
					|
	3A			 2A | 3B			2B
	*/
	// Initial Conditions
	set->poly_a[0] = set->poly_a[0];
	set->poly_a[3] = set->poly_a[3];
	set->poly_b[1] = set->poly_a[1];
	set->poly_b[2] = set->poly_a[2];
		
	// 0 -> 1
	int * live_vt = &subdivided_points[sub_vert_cnt][0];
	live_vt[X] = (set->ptv[0][X] + set->ptv[1][X])>>1;
	live_vt[Y] = (set->ptv[0][Y] + set->ptv[1][Y])>>1;
	live_vt[Z] = (set->ptv[0][Z] + set->ptv[1][Z])>>1;
	
	set->poly_a[1] = sub_vert_cnt;
	set->poly_b[0] = sub_vert_cnt;
	sub_vert_cnt++;
	// 3 -> 2
	live_vt = &subdivided_points[sub_vert_cnt][0];
	live_vt[X] = (set->ptv[2][X] + set->ptv[3][X])>>1;
	live_vt[Y] = (set->ptv[2][Y] + set->ptv[3][Y])>>1;
	live_vt[Z] = (set->ptv[2][Z] + set->ptv[3][Z])>>1;
	
	set->poly_a[2] = sub_vert_cnt;
	set->poly_b[3] = sub_vert_cnt;
	sub_vert_cnt++;
	
	sub_poly_cnt += 1; //Only add 1, as there was already 1 polygon. It was split in two.
	
}


void	subdivide_plane(short start_point, short overwritten_polygon, short num_divisions, short total_divisions)
{
	//if((start_point+4) >= MAX_IN_TILE) return;
	//if((overwritten_polygon+4) >= MAX_IN_TILE) return;
	//"Load" the original points (code shortening operation)
	FIXED * ptv[4];
	static char new_rule;
	static _subdivision_settings sub;

	ptv[0] = &subdivided_points[subdivided_polygons[overwritten_polygon][0]][X];
	ptv[1] = &subdivided_points[subdivided_polygons[overwritten_polygon][1]][X];
	ptv[2] = &subdivided_points[subdivided_polygons[overwritten_polygon][2]][X];
	ptv[3] = &subdivided_points[subdivided_polygons[overwritten_polygon][3]][X];

	new_rule = plane_rules[total_divisions];

	if((num_divisions <= 0 || plane_rules[total_divisions] == 0))
	{
		return;
	}
	
	//Because "sub" is a structure of localized memory common to all subdivided planes,
	//we have to store this data separately from the subdivision parameters to prevent data corruption on recursive calls.
	//This definitely blows some stack space.
	short semaphore_poly_a = overwritten_polygon;
	short semaphore_poly_b = sub_poly_cnt;
	short semaphore_poly_c = sub_poly_cnt+1;
	short semaphore_poly_d = sub_poly_cnt+2;
	
	sub.ptv[0] = ptv[0];
	sub.ptv[1] = ptv[1];
	sub.ptv[2] = ptv[2];
	sub.ptv[3] = ptv[3];
	sub.poly_a = &subdivided_polygons[semaphore_poly_a][0];
	sub.poly_b = &subdivided_polygons[semaphore_poly_b][0];
	sub.poly_c = &subdivided_polygons[semaphore_poly_c][0];
	sub.poly_d = &subdivided_polygons[semaphore_poly_d][0];
	
	///////////////////////////////////////////
	//	Recursively subdivide the polygon.
	///////////////////////////////////////////
	
	switch(new_rule)
	{
	case(SUBDIVIDE_XY):
	subdivide_xy(&sub);

	subdivide_plane(sub_vert_cnt, semaphore_poly_a, num_divisions-1, total_divisions+1);
	subdivide_plane(sub_vert_cnt, semaphore_poly_b, num_divisions-1, total_divisions+1);
	subdivide_plane(sub_vert_cnt, semaphore_poly_c, num_divisions-1, total_divisions+1);
	subdivide_plane(sub_vert_cnt, semaphore_poly_d, num_divisions-1, total_divisions+1);
		
	break;
	case(SUBDIVIDE_Y):
	subdivide_y(&sub);

	subdivide_plane(sub_vert_cnt, semaphore_poly_a, num_divisions-1, total_divisions+1);
	subdivide_plane(sub_vert_cnt, semaphore_poly_b, num_divisions-1, total_divisions+1);

	break;
	case(SUBDIVIDE_X):
	subdivide_x(&sub);

	subdivide_plane(sub_vert_cnt, semaphore_poly_a, num_divisions-1, total_divisions+1);
	subdivide_plane(sub_vert_cnt, semaphore_poly_b, num_divisions-1, total_divisions+1);

	break;
	default:
	break;
	}
	
}

void *	preprocess_planes_to_tiles_for_sector(_sector * sct, void * workAddress)
{
	//What we are going to do:
	//Subdivide every plane in the original mesh, as stored in the mesh, to the subdivision buffers.
	//When the plane is finished subdividing, we will dump the vertex and polygon buffers to 
	//sct->tltbl and sct->tvtbl with the vertex count for each plane being added to sct->nbTileVert and tile count to sct->nbTile 
	entity_t * ent = sct->ent;
	GVPLY * mesh = ent->pol;
	
	sct->nbTile = 0;
	sct->nbTileVert = 0;
	
	POINT * tile_vert_buf = (POINT *)dirty_buf;
	
	_quad * tile_poly_buf = (_quad *)dirtier_buf;
	
	static int tNew = 0;
	static int tvNew = 0;
	static int tPlane = 0;
	tPlane = 0;
	tNew = 0;
	tvNew = 0;
	
	workAddress = align_4(workAddress);
	
	sct->altbl = (unsigned short *)workAddress;
	
	//In this case, we are doing this in model-space.
	for(unsigned int i = 0; i < sct->nbPolygon; i++)
	{
		sub_vert_cnt = 0;
		sub_poly_cnt = 0;
		
		int alias = sct->pltbl[i];
		int base_tvNew = tvNew;
	
		plane_rules[0] = mesh->attbl[alias].plane_information & 0x3;
		plane_rules[1] = (mesh->attbl[alias].plane_information>>2) & 0x3;
		plane_rules[2] = (mesh->attbl[alias].plane_information>>4) & 0x3;
		plane_rules[3] = 0;
		
		//We have some special things to do...
		//We have to put the vertices of the polygon into:
		//subdivided polygons and subdivided vertices buffers.
		
		for(int k = 0; k < 4; k++)
		{
			subdivided_points[k][X] = mesh->pntbl[mesh->pltbl[alias].vertices[k]][X];
			subdivided_points[k][Y] = mesh->pntbl[mesh->pltbl[alias].vertices[k]][Y];
			subdivided_points[k][Z] = mesh->pntbl[mesh->pltbl[alias].vertices[k]][Z];
			subdivided_polygons[0][k] = k;
		}
		sub_vert_cnt += 4;
		sub_poly_cnt += 1;
		
		subdivide_plane(sub_vert_cnt, 0, 3, 0);
		
		//After this, the subdivided plane/polygon buffers should be full of tile data for this plane.
		//These points are in model-space / mesh-space.
		//We first want to dump these to the dirty buf in LWRAM before checking them in to the sector's list in HWRAM.
		
		for(int l = 0; l < sub_vert_cnt; l++)
		{
			tile_vert_buf[tvNew][X] = subdivided_points[l][X];
			tile_vert_buf[tvNew][Y] = subdivided_points[l][Y];
			tile_vert_buf[tvNew][Z] = subdivided_points[l][Z];
			tvNew++;
		}
		
		for(int l = 0; l < sub_poly_cnt; l++)
		{
			tile_poly_buf[tNew].vertices[0] = subdivided_polygons[l][0] + base_tvNew;
			tile_poly_buf[tNew].vertices[1] = subdivided_polygons[l][1] + base_tvNew;
			tile_poly_buf[tNew].vertices[2] = subdivided_polygons[l][2] + base_tvNew;
			tile_poly_buf[tNew].vertices[3] = subdivided_polygons[l][3] + base_tvNew;
			sct->altbl[tNew] = alias;
			tNew++;
		}
		//With tvNew and tNew, we now have a count of the number of polygons and planes made so far IN TOTAL.
	}
	

	sct->nbTileVert = tvNew;
	//To account for the size of the alias table, we have to push workAddress forward.
	workAddress += tNew * sizeof(short);
	//nbTile table : stores the number of tiles per plane
	sct->nbTile = (unsigned short *)workAddress;
	//Set forward by short * number of planes
	workAddress += sct->nbPolygon * sizeof(unsigned short);
	//plStart table : stores first tile ID # in each plane
	sct->plStart = (unsigned short *)workAddress;
	//Set forward by short * number of planes
	workAddress += sct->nbPolygon * sizeof(unsigned short);
	//Align address
	workAddress = align_4(workAddress);
	//Set address of tile polygon table
	sct->tltbl = (_quad *)workAddress;
	//We know its size already; there are no duplicate polygons.
	workAddress += tNew * sizeof(_quad);
	//Find & write the number of tiles per polygon
	for(unsigned int i = 0; i < sct->nbPolygon; i++)
	{
		tPlane = 0;
		int first_tile = -1;
		for(int t = 0; t < tNew; t++)
		{
			if(sct->altbl[t] == sct->pltbl[i])
			{
				first_tile = (first_tile == -1) ? t : first_tile;
				tPlane++;
			}
		}
		sct->plStart[i] = first_tile;
		sct->nbTile[i] = tPlane;
	}
	//Write a copy of an unmodified (no duplicates removed) tile table
	for(int i = 0; i < tNew; i++)
	{
		for(int k = 0; k < 4; k++)
		{
			sct->tltbl[i].vertices[k] = tile_poly_buf[i].vertices[k];
		}
	}
	//Align address for tile vertex table
	workAddress = align_4(workAddress);
	//Set address of tile vertex table
	sct->tvtbl = (POINT *)workAddress;
	
	tvNew = 0;
	for(unsigned int i = 0; i < sct->nbTileVert; i++)
	{
		//First, check all vertices to see if this is a duplicate or not.
		//Throw out the low order bits when doing this. Just cuz.

		//In case of something found as a duplicate, it is marked with zero. This indicates a duplicate.
		//We won't be adding this.
		if(tile_vert_buf[i][X] == 0 && tile_vert_buf[i][Y] == 0 && tile_vert_buf[i][Z] == 0) continue;
		
		for(unsigned int d = 0; d < sct->nbTileVert; d++)
		{
			//Don't mark the same vertex as a duplicate.
			if(d == i) continue;
			//In case of being marked with zero, it would have been a duplicate. Do not check it.
			if(tile_vert_buf[d][X] == 0 && tile_vert_buf[d][Y] == 0 && tile_vert_buf[d][Z] == 0) continue;
			if((tile_vert_buf[i][X]>>16) == (tile_vert_buf[d][X]>>16) &&
			(tile_vert_buf[i][Y]>>16) == (tile_vert_buf[d][Y]>>16) &&
			(tile_vert_buf[i][Z]>>16) == (tile_vert_buf[d][Z]>>16))
			{
				//Okay, we have a duplicate. i is the same vertex as d.
				//First thing we have to do is go through every polygon and find ones which use the index d.
				//Replace that index with i.
				for(int p = 0; p < tNew; p++)
				{
					for(int k = 0; k < 4; k++)
					{
						if(tile_poly_buf[p].vertices[k] == d) tile_poly_buf[p].vertices[k] = i;
					}
				}
				//Now we must denote the duplicate vertex so it is not added again.
				//We will do this by zeroing it out.
				tile_vert_buf[d][X] = 0;
				tile_vert_buf[d][Y] = 0;
				tile_vert_buf[d][Z] = 0;
			}
			
		}
		

		for(int k = 0; k < 3; k++)
		{
			sct->tvtbl[tvNew][k] = tile_vert_buf[i][k];
		}
		//Add to the new duplicate-removed vertex count.
		tvNew++;
	}
	sct->nbTileVert = tvNew;
	workAddress += sct->nbTileVert * sizeof(POINT);
	
	//Now that we have removed duplicates from the vertex list, the vertex list size has changed.
	//Doing so has unpredictably changed the index of each vertex in the list, meaning the polygon list is now invalid.
	//We have allocated a copy of the original list to sct->tltbl and have the duplicates-removed list at tile_poly_buf.
	//We also have the original in-order list in tile_vert_buf with duplicates marked zero.
	//What we must do is check every polygon in tile_poly_buf to find the original values of its verices in tile_vert_buf.
	//We must then find that vertex in sct->tvtbl, and change the index of the polygon in sct->tltbl to index vertex at sct->tvtbl.
	POINT overt = {0,0,0};
	for(int i = 0; i < tNew; i++)
	{
		for(int k = 0; k < 4; k++)
		{
			overt[X] = tile_vert_buf[tile_poly_buf[i].vertices[k]][X];
			overt[Y] = tile_vert_buf[tile_poly_buf[i].vertices[k]][Y];
			overt[Z] = tile_vert_buf[tile_poly_buf[i].vertices[k]][Z];
			
			for(unsigned int v = 0; v < sct->nbTileVert; v++)
			{
				if(overt[X] == sct->tvtbl[v][X] && overt[Y] == sct->tvtbl[v][Y] && overt[Z] == sct->tvtbl[v][Z])
				{
					//We've found, through process of elimination, the matching vertex.
					//We want the new table in sct->tltbl to index this vertex.
					sct->tltbl[i].vertices[k] = v;
				}
			}
		}
	}

	return align_4(workAddress);

}

void	subdivide_tile(short overwritten_polygon, short num_divisions, short total_divisions, short rootTex)
{
	//"Load" the original points (code shortening operation)
	int new_rule;
	static _subdivision_settings sub;
	
	short * semaphore_poly_a = &subdivided_polygons[overwritten_polygon][0];

	sub.ptv[0] = &subdivided_points[semaphore_poly_a[0]][X];
	sub.ptv[1] = &subdivided_points[semaphore_poly_a[1]][X];
	sub.ptv[2] = &subdivided_points[semaphore_poly_a[2]][X];
	sub.ptv[3] = &subdivided_points[semaphore_poly_a[3]][X];
	
	if(sub.ptv[0][Z] < 0 && sub.ptv[1][Z] < 0 && sub.ptv[2][Z] < 0 && sub.ptv[3][Z] < 0) return;
	new_rule = tile_rules[total_divisions];
	rootTex = rootTex-1;
	used_textures[overwritten_polygon] = rootTex;
	//////////////////////////////////////////////////////////////////
	// Quick check: If we are subdividing a polygon above the z level, stop further subdivision.
	// This is mostly useful in cases where a large polygon is being recursively subdivided and parts of it may be far away.
	//////////////////////////////////////////////////////////////////
	int polygon_minimum = JO_MIN(JO_MIN(sub.ptv[0][Z], sub.ptv[1][Z]),  JO_MIN(sub.ptv[2][Z], sub.ptv[3][Z]));
	///////////
	// Don't try and add an exception to let things draw closer to the screen,
	// even as untextured polygons.
	// There's no solution that is compatible with the way the screen transform math works.
	// You'd need to draw things completely differently.
	if(num_divisions <= 0 || tile_rules[total_divisions] == 0 || polygon_minimum > z_rules[total_divisions])
	{
		return;
	}

	//Because "sub" is a structure of localized memory common to all subdivided planes,
	//we have to store this data separately from the subdivision parameters to prevent data corruption on recursive calls.
	//This definitely blows some stack space.

	short semaphore_poly_b = sub_poly_cnt;
	short semaphore_poly_c = sub_poly_cnt+1;
	short semaphore_poly_d = sub_poly_cnt+2;
	
	sub.poly_a = semaphore_poly_a;
	sub.poly_b = &subdivided_polygons[semaphore_poly_b][0];
	sub.poly_c = &subdivided_polygons[semaphore_poly_c][0];
	sub.poly_d = &subdivided_polygons[semaphore_poly_d][0];
	
	// mov @(R0, set);
	// R0 being a byte offset, and "set" being an absolute address
	// subdivided_polygons is 8 bytes per entry, so <<3
	// subdivided_points is 16 bytes per entry
	// BENCHMARKED: The assembly IS faster, but it is micro-faster; like 1 or 2% faster; maybe not faster (or slower).
	// (to be clear I seriously don't expect to do better than the compiler at such simple tasks)
	// Some more potential optimizations is to get rid of the middleman struct "sub"
	switch(new_rule)
	{
	case(SUBDIVIDE_XY):
	// subdivide_xy(&sub);
	asm(
		"mov.l @(16,%[set]),r2;" //Move &poly_a to r2
		"mov.l @(20,%[set]),r3;" //Move &poly_b to r3
		"mov.l @(24,%[set]),r4;" //Move &poly_c to r4
		"mov.l @(28,%[set]),r5;" //Move &poly_d to r5
		"mov.w @(2,r2),r0;"
		"mov.w r0,@(2,r3);"		//Copy poly_a[1] to poly_b[1]
		"mov.w @(4,r2),r0;"
		"mov.w r0,@(4,r5);"		//Copy poly_a[2] to poly_c[2]
		"mov.w @(6,r2),r0;"
		"mov.w r0,@(6,r4);"		//Copy poly_a[3] to poly_d[3]
		"mov.w @%[pnt_cnt],r0;" //Copy sub_vert_cnt to r0
		"mov r0,r1;"
		"shll2 r1;"
		"shll2 r1;"				//Perform sub_vert_cnt<<2 in r1
		"add %[sub_pnt],r1;"	//Add subdivided_points to r1 to get address of: subdivided_points[sub_vert_cnt] in r1
		"mov.l @%[set],r6;"		//Move ptv[0] to r6
		"mov.l @r6,r7;"
		"mov.l @(4,r6),r8;"
		"mov.l @(8,r6),r9;"		//Move ptv[0][xyz] to r7,r8,r9
		"mov.l @(4,%[set]),r6;"	//Move ptv[1] to r6
		"mov.l @r6,r10;"
		"add r10,r7;"			//ptv[0][X] + ptv[1][X]
		"mov.l @(4,r6),r10;"
		"add r10,r8;"			//ptv[0][Y] + ptv[1][Y]
		"mov.l @(8,r6),r10;"
		"add r10,r9;"			//ptv[0][Z] + ptv[1][Z] -- this code will repeat for ptv[2] and ptv[3] to be added.
		"mov.l @(8,%[set]),r6;" //Move ptv[2] to r6
		"mov.l @r6,r10;"
		"add r10,r7;"
		"mov.l @(4,r6),r10;"
		"add r10,r8;"
		"mov.l @(8,r6),r10;"
		"add r10,r9;"
		"mov.l @(12,%[set]),r6;" //Move ptv[3] to r6
		"mov.l @r6,r10;"
		"add r10,r7;"
		"mov.l @(4,r6),r10;"
		"add r10,r8;"
		"mov.l @(8,r6),r10;"
		"add r10,r9;"			//ptv[0,1,2,3] have been added together
		"shar r7;"
		"shar r7;"
		"shar r8;"
		"shar r8;"
		"shar r9;"
		"shar r9;"				//Shifts each of xyz right by two (arithmetic shift)
		"mov.l r7,@r1;"
		"mov.l r8,@(4,r1);"
		"mov.l r9,@(8,r1);"		//Set this point to subdivided_points[sub_vert_cnt]
		"add #16,r1;"			//Increment the pointer in subdivided_points
		"mov.w r0,@(4,r2);"		//Set poly_a[2] = sub_vert_cnt (in r0)
		"mov.w r0,@(6,r3);"		//Set poly_b[3] = sub_vert_cnt
		"mov.w r0,@(2,r4);"		//Set poly_c[1] = sub_vert_cnt
		"mov.w r0,@r5;"			//Set poly_d[0] = sub_vert_cnt
		"add #1,r0;"				//End calculating new vertex, add 1 to vertex count
		"mov.l @%[set],r6;"		//Move ptv[0] to r6 (for (ptv[0]+ptv[1])>>1
		"mov.l @r6,r7;"
		"mov.l @(4,r6),r8;"
		"mov.l @(8,r6),r9;"		//Move ptv[0][xyz] to r7,r8,r9
		"mov.l @(4,%[set]),r6;"	//Move ptv[1] to r6
		"mov.l @r6,r10;"
		"add r10,r7;"			//ptv[0][X] + ptv[1][X]
		"mov.l @(4,r6),r10;"
		"add r10,r8;"			//ptv[0][Y] + ptv[1][Y]
		"mov.l @(8,r6),r10;"
		"add r10,r9;"			//ptv[0][Z] + ptv[1][Z] -- this code segment is only ptv[0]+ptv[1].
		"shar r7;"
		"shar r8;"
		"shar r9;"
		"mov.l r7,@r1;"
		"mov.l r8,@(4,r1);"
		"mov.l r9,@(8,r1);"		//Set this point to subdivided_points[sub_vert_cnt]
		"add #16,r1;"			//Increment the pointer in subdivided_points
		"mov.w r0,@(2,r2);"		//Set poly_a[1] = sub_vert_cnt (in r0)
		"mov.w r0,@r3;"			//Set poly_b[0] = sub_vert_cnt
		"add #1,r0;"				//End calculating new vertex, add 1 to vertex count
		"mov.l @(4,%[set]),r6;"	//Move ptv[1] to r6 (for (ptv[2]+ptv[1])>>1
		"mov.l @r6,r7;"
		"mov.l @(4,r6),r8;"
		"mov.l @(8,r6),r9;"		//Move ptv[1][xyz] to r7,r8,r9
		"mov.l @(8,%[set]),r6;"	//Move ptv[2] to r6
		"mov.l @r6,r10;"
		"add r10,r7;"			//ptv[2][X] + ptv[1][X]
		"mov.l @(4,r6),r10;"
		"add r10,r8;"			//ptv[2][Y] + ptv[1][Y]
		"mov.l @(8,r6),r10;"
		"add r10,r9;"			//ptv[2][Z] + ptv[1][Z] -- this code segment is only ptv[0]+ptv[1].
		"shar r7;"
		"shar r8;"
		"shar r9;"
		"mov.l r7,@r1;"
		"mov.l r8,@(4,r1);"
		"mov.l r9,@(8,r1);"		//Set this point to subdivided_points[sub_vert_cnt]
		"add #16,r1;"			//Increment the pointer in subdivided_points
		"mov.w r0,@(4,r3);"		//Set poly_b[2] = sub_vert_cnt (in r0)
		"mov.w r0,@(2,r5);"		//Set poly_d[1] = sub_vert_cnt
		"add #1,r0;"				//End calculating new vertex, add 1 to vertex count
		"mov.l @(8,%[set]),r6;"	//Move ptv[2] to r6 (for (ptv[2]+ptv[3])>>1
		"mov.l @r6,r7;"
		"mov.l @(4,r6),r8;"
		"mov.l @(8,r6),r9;"		//Move ptv[3][xyz] to r7,r8,r9
		"mov.l @(12,%[set]),r6;"	//Move ptv[3] to r6
		"mov.l @r6,r10;"
		"add r10,r7;"			//ptv[2][X] + ptv[3][X]
		"mov.l @(4,r6),r10;"
		"add r10,r8;"			//ptv[2][Y] + ptv[3][Y]
		"mov.l @(8,r6),r10;"
		"add r10,r9;"			//ptv[2][Z] + ptv[3][Z] -- this code segment is only ptv[0]+ptv[1].
		"shar r7;"
		"shar r8;"
		"shar r9;"
		"mov.l r7,@r1;"
		"mov.l r8,@(4,r1);"
		"mov.l r9,@(8,r1);"		//Set this point to subdivided_points[sub_vert_cnt]
		"add #16,r1;"			//Increment the pointer in subdivided_points
		"mov.w r0,@(4,r4);"		//Set poly_c[2] = sub_vert_cnt (in r0)
		"mov.w r0,@(6,r5);"		//Set poly_d[3] = sub_vert_cnt
		"add #1,r0;"				//End calculating new vertex, add 1 to vertex count
		"mov.l @%[set],r6;"		//Move ptv[0] to r6 (for (ptv[0]+ptv[3])>>1
		"mov.l @r6,r7;"
		"mov.l @(4,r6),r8;"
		"mov.l @(8,r6),r9;"		//Move ptv[3][xyz] to r7,r8,r9
		"mov.l @(12,%[set]),r6;"	//Move ptv[3] to r6
		"mov.l @r6,r10;"
		"add r10,r7;"			//ptv[0][X] + ptv[3][X]
		"mov.l @(4,r6),r10;"
		"add r10,r8;"			//ptv[0][Y] + ptv[3][Y]
		"mov.l @(8,r6),r10;"
		"add r10,r9;"			//ptv[0][Z] + ptv[3][Z] -- this code segment is only ptv[0]+ptv[1].
		"shar r7;"
		"shar r8;"
		"shar r9;"
		"mov.l r7,@r1;"
		"mov.l r8,@(4,r1);"
		"mov.l r9,@(8,r1);"		//Set this point to subdivided_points[sub_vert_cnt]
		"add #16,r1;"			//Increment the pointer in subdivided_points
		"mov.w r0,@(6,r2);"		//Set poly_a[3] = sub_vert_cnt (in r0)
		"mov.w r0,@r4;"			//Set poly_c[0] = sub_vert_cnt
		"add #1,r0;"				//End calculating new vertex, add 1 to vertex count
		"mov.w r0,@%[pnt_cnt];" //Move the new sub_vert_cnt (in r0) to the address of sub_vert_cnt
		"mov.w @%[ply_cnt],r0;"	//Move ply_cnt to r0
		"add #3,r0;"			//We added three polygons in this process; increase the polygon count by 3.
		"mov.w r0,@%[ply_cnt];"
		: 																//OUT
		:	[set] "p" (&sub), [sub_pnt] "p" (subdivided_points), [pnt_cnt] "p" (&sub_vert_cnt), [ply_cnt] "p" (&sub_poly_cnt)	//IN
		:	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10"		//CLOBBERS
	);
	break;
	case(SUBDIVIDE_Y):
	// subdivide_y(&sub);
	asm(
		"mov.l @(16,%[set]),r3;" //poly_a's pointer to r3
		"mov.l @(20,%[set]),r4;" //poly_b's pointer to r4
		"mov.w @(4,r3),r0;" //Copy poly_a[2] to r0 (the syntax here doesn't line up with the manual)
		"mov.w r0,@(4,r4);" //Copy r0 to poly_b[2] (GCC expects the literal byte offset, and converts it)
		"mov.w @(6,r3),r0;" //Copy poly_a[3] to r0 (the manual states the offset is x1/x2/x4 for mov.b/w/l)
		"mov.w r0,@(6,r4);" //Copy r0 to poly_b[3]
		"mov.w @%[pnt_cnt],r0;" //Copy sub_vert_cnt to r0
		"mov.w r0,@(4,r3);" //Copy sub_vert_cnt in r0 to poly_a[2]
		"mov.w r0,@(2,r4);" //copy sub_vert_cnt in r0 to poly_b[1]
		"mov r0, r1;"		//copy sub_vert_cnt in r0 to r1
		"shll2 r1;"
		"shll2 r1;"			// sub_vert_cnt<<2 in r1
		"add %[sub_pnt],r1;" //makes pointer to sub_pnt[sub_vert_cnt] in r1
		"mov.l @(4,%[set]),r2;" //copy set->ptv[1] to r2
		"mov.l @r2,r5;"			
		"mov.l @(4,r2),r6;"
		"mov.l @(8,r2),r7;"		//copy set->ptv[1][xyz] to r5,r6,r7
		"mov.l @(8,%[set]),r2;" //copy set->ptv[2] to r2
		"mov.l @r2,r8;"		//set->ptv[2][X] in r8
		"add r8,r5;"
		"shar r5;"			//(set->ptv[1][X] + set->ptv[2][X]) >> 1
		"mov.l @(4,r2),r8;"//set->ptv[2][Y] in r8
		"add r8,r6;"       
		"shar r6;"         //(set->ptv[1][Y] + set->ptv[2][Y]) >> 1
		"mov.l @(8,r2),r8;"//set->ptv[2][Z] in r8
		"add r8,r7;"       
		"shar r7;"         //(set->ptv[1][Z] + set->ptv[2][Z]) >> 1
		"mov.l r5,@r1;"
		"mov.l r6,@(4,r1);"
		"mov.l r7,@(8,r1);" //copies (ptv[1][xyz]+ptv[2][xyz])>>1 to sub_pnt[sub_vert_cnt][xyz]
		"add #1,r0;" //add 1 to sub_vert_cnt in r0
		"mov.w r0,@(6,r3);" //poly_a[3] = sub_vert_cnt+1
		"mov.w r0,@r4;"	//poly_b[0] = sub_vert_cnt+1
		"add #16,r1;" //add 16 to r1 to reach the next array entry
		"mov.l @(12,%[set]),r2;" //move set->ptv[3] to r2
		"mov.l @r2,r5;"
		"mov.l @(4,r2),r6;"
		"mov.l @(8,r2),r7;"	//move set->ptv[3][xyz] to r5,r7,r7
		"mov.l @%[set],r2;" //move set->ptv[0] to r2
		"mov.l @r2,r8;"		//set->ptv[0][X] in r8
		"add r8,r5;"
		"shar r5;"			//(set->ptv[0][X] + set->ptv[3][X]) >> 1
		"mov.l @(4,r2),r8;"//set->ptv[0][Y] in r8
		"add r8,r6;"       
		"shar r6;"         //(set->ptv[0][Y] + set->ptv[3][Y]) >> 1
		"mov.l @(8,r2),r8;"//set->ptv[0][Z] in r8
		"add r8,r7;"       
		"shar r7;"         //(set->ptv[0][Z] + set->ptv[3][Z]) >> 1
		"mov.l r5,@r1;"
		"mov.l r6,@(4,r1);"
		"mov.l r7,@(8,r1);" //copies (ptv[0][xyz]+ptv[3][xyz])>>1 to sub_pnt[tgt_pnt+1][xyz]
		"add #1,r0;"			//add 1 to sub_vert_cnt in r0
		"mov.w r0,@%[pnt_cnt];"	//move sub_vert_cnt+2 to sub_vert_cnt
		"mov.w @%[ply_cnt],r0;" //move sub_poly_cnt to r0
		"add #1,r0;"			//add 1 to sub_poly_cnt
		"mov.w r0,@%[ply_cnt];" //move sub_poly_cnt+1 to sub_poly_cnt
		: 																//OUT
		:	[set] "p" (&sub), [sub_pnt] "p" (subdivided_points), [pnt_cnt] "p" (&sub_vert_cnt), [ply_cnt] "p" (&sub_poly_cnt)	//IN
		:	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8"		//CLOBBERS
	);
	break;
	case(SUBDIVIDE_X):
	// subdivide_x(&sub);
	asm(
		"mov.l @(16,%[set]),r3;" //poly_a's pointer to r3
		"mov.l @(20,%[set]),r4;" //poly_b's pointer to r4
		"mov.w @(2,r3),r0;" //Copy poly_a[1] to r0 (the syntax here doesn't line up with the manual)
		"mov.w r0,@(2,r4);" //Copy r0 to poly_b[1] (GCC expects the literal byte offset, and converts it)
		"mov.w @(4,r3),r0;" //Copy poly_a[2] to r0 (the manual states the offset is x1/x2/x4 for mov.b/w/l)
		"mov.w r0,@(4,r4);" //Copy r0 to poly_b[2]
		"mov.w @%[pnt_cnt],r0;" //Copy sub_vert_cnt to r0
		"mov.w r0,@(2,r3);" //Copy sub_vert_cnt in r0 to poly_a[1]
		"mov.w r0,@r4;" //copy sub_vert_cnt in r0 to poly_b[0]
		"mov r0, r1;"		//copy sub_vert_cnt in r0 to r1
		"shll2 r1;"
		"shll2 r1;"			// sub_vert_cnt<<2 in r1
		"add %[sub_pnt],r1;" //makes pointer to sub_pnt[sub_vert_cnt] in r1
		"mov.l @(4,%[set]),r2;" //copy set->ptv[1] to r2
		"mov.l @r2,r5;"			
		"mov.l @(4,r2),r6;"
		"mov.l @(8,r2),r7;"		//copy set->ptv[1][xyz] to r5,r6,r7
		"mov.l @%[set],r2;" //copy set->ptv[0] to r2
		"mov.l @r2,r8;"		//set->ptv[0][X] in r8
		"add r8,r5;"
		"shar r5;"			//(set->ptv[1][X] + set->ptv[0][X]) >> 1
		"mov.l @(4,r2),r8;"//set->ptv[0][Y] in r8
		"add r8,r6;"       
		"shar r6;"         //(set->ptv[1][Y] + set->ptv[0][Y]) >> 1
		"mov.l @(8,r2),r8;"//set->ptv[0][Z] in r8
		"add r8,r7;"       
		"shar r7;"         //(set->ptv[1][Z] + set->ptv[0][Z]) >> 1
		"mov.l r5,@r1;"
		"mov.l r6,@(4,r1);"
		"mov.l r7,@(8,r1);" //copies (ptv[0][xyz]+ptv[1][xyz])>>1 to sub_pnt[sub_vert_cnt][xyz]
		"add #1,r0;" //add 1 to sub_vert_cnt in r0
		"mov.w r0,@(4,r3);" //poly_a[2] = sub_vert_cnt+1
		"mov.w r0,@(6,r4);"	//poly_b[3] = sub_vert_cnt+1
		"add #16,r1;" //add 16 to r1 to reach the next array entry
		"mov.l @(12,%[set]),r2;" //move set->ptv[3] to r2
		"mov.l @r2,r5;"
		"mov.l @(4,r2),r6;"
		"mov.l @(8,r2),r7;"	//move set->ptv[3][xyz] to r5,r7,r7
		"mov.l @(8,%[set]),r2;" //move set->ptv[2] to r2
		"mov.l @r2,r8;"		//set->ptv[2][X] in r8
		"add r8,r5;"
		"shar r5;"			//(set->ptv[2][X] + set->ptv[3][X]) >> 1
		"mov.l @(4,r2),r8;"//set->ptv[2][Y] in r8
		"add r8,r6;"       
		"shar r6;"         //(set->ptv[2][Y] + set->ptv[3][Y]) >> 1
		"mov.l @(8,r2),r8;"//set->ptv[2][Z] in r8
		"add r8,r7;"       
		"shar r7;"         //(set->ptv[2][Z] + set->ptv[3][Z]) >> 1
		"mov.l r5,@r1;"
		"mov.l r6,@(4,r1);"
		"mov.l r7,@(8,r1);" //copies (ptv[2][xyz]+ptv[3][xyz])>>1 to sub_pnt[tgt_pnt+1][xyz]
		"add #1,r0;"			//add 1 to sub_vert_cnt in r0
		"mov.w r0,@%[pnt_cnt];"	//move sub_vert_cnt+2 to sub_vert_cnt
		"mov.w @%[ply_cnt],r0;" //move sub_poly_cnt to r0
		"add #1,r0;"			//add 1 to sub_poly_cnt
		"mov.w r0,@%[ply_cnt];" //move sub_poly_cnt+1 to sub_poly_cnt
		: 																//OUT
		:	[set] "p" (&sub), [sub_pnt] "p" (subdivided_points), [pnt_cnt] "p" (&sub_vert_cnt), [ply_cnt] "p" (&sub_poly_cnt)	//IN
		:	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8"		//CLOBBERS
	);
	break;
	default:
	break;
	}
	
	subdivide_tile(overwritten_polygon, num_divisions-1, total_divisions+1, texIDs_cut_from_texID[rootTex][0]);
	subdivide_tile(semaphore_poly_b, num_divisions-1, total_divisions+1, texIDs_cut_from_texID[rootTex][1]);
	//All subdivision methods use the above two recursive calls; subdivide_xy is unique. 
	//In case subdivide_xy was not used, do not continue.
	if(new_rule != SUBDIVIDE_XY) return;
	subdivide_tile(semaphore_poly_c, num_divisions-1, total_divisions+1, texIDs_cut_from_texID[rootTex][2]);
	subdivide_tile(semaphore_poly_d, num_divisions-1, total_divisions+1, texIDs_cut_from_texID[rootTex][3]);
	
}


void	plane_rendering_with_subdivision(entity_t * ent)
{
	///////////////////////////////////////////
	// If the file is not yet loaded, do not try and render it.
	// If the entity type is not 'B' for BUILDING, do not try and render it as it won't have proper textures.
	///////////////////////////////////////////
	if(ent->file_done != true) return;
	if(ent->type != MODEL_TYPE_BUILDING) return;
	GVPLY * mesh = ent->pol;
	
	sub_poly_cnt = 0;
	sub_vert_cnt = 0;
	unsigned short	colorBank = 0;
	int		inverseZ = 0;
	int 	luma = 0;
	int 	zDepthTgt = 0;
	unsigned short	flags = 0;
	unsigned short	flip = 0;
	unsigned short	pclp = 0;

	vertex_t * ptv[5];
	int * stv[4];

    static MATRIX newMtx;
	static FIXED m0x[4];
	static FIXED m1y[4];
	static FIXED m2z[4];
	
	//These can't have an orienation... eh, we'll do it anyway.
	slMultiMatrix((POINT *)ent->prematrix);
    slGetMatrix(newMtx);
	
	m0x[0] = newMtx[X][X];
	m0x[1] = newMtx[Y][X];
	m0x[2] = newMtx[Z][X];
	m0x[3] = newMtx[3][X];
	
	m1y[0] = newMtx[X][Y];
	m1y[1] = newMtx[Y][Y];
	m1y[2] = newMtx[Z][Y];
	m1y[3] = newMtx[3][Y];
	
	m2z[0] = newMtx[X][Z];
	m2z[1] = newMtx[Y][Z];
	m2z[2] = newMtx[Z][Z];
	m2z[3] = newMtx[3][Z];

	/**
	Rendering Planes
	Right now, this is slow. Very slow.
	**/


	
	int specific_texture = 0;
	int dual_plane = 0;
	int cue;
	////////////////////////////////////////////////////
	// Transform each light source position by the matrix parameters.
	////////////////////////////////////////////////////
	// POINT relative_light_pos = {0, 0, 0};
	// static POINT tx_light_pos[MAX_DYNAMIC_LIGHTS];
	// FIXED * mesh_position = &ent->prematrix[9];
	// int inverted_proxima;
	
	// for(int l = 0; l < MAX_DYNAMIC_LIGHTS; l++)
	// {
		// if(active_lights[l].pop == 1)
		// {
			// relative_light_pos[X] = -active_lights[l].pos[X] - mesh_position[X];
			// relative_light_pos[Y] = -active_lights[l].pos[Y] - mesh_position[Y];
			// relative_light_pos[Z] = -active_lights[l].pos[Z] - mesh_position[Z];
			// tx_light_pos[l][X] = trans_pt_by_component(relative_light_pos, m0x);
			// tx_light_pos[l][Y] = trans_pt_by_component(relative_light_pos, m1y);
			// tx_light_pos[l][Z] = trans_pt_by_component(relative_light_pos, m2z);
		// }
	// }
	
//First: Transform all of the vertices of the mesh to a buffer.
for(unsigned int i = 0; i < mesh->nbPoint; i++)
{
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Vertice 3D Transformation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /**calculate z**/
        screen_transform_buffer[i].pnt[Z] = trans_pt_by_component(mesh->pntbl[i], m2z);
		sub_transform_buffer[i][Z] = screen_transform_buffer[i].pnt[Z];
		screen_transform_buffer[i].pnt[Z] = (screen_transform_buffer[i].pnt[Z] > NEAR_PLANE_DISTANCE) ? screen_transform_buffer[i].pnt[Z] : NEAR_PLANE_DISTANCE;
         /**Starts the division**/
      //  SetFixDiv(scrn_dist, screen_transform_buffer[i].pnt[Z]);

        /**Calculates X and Y while waiting for screenDist/z **/
        sub_transform_buffer[i][Y] = trans_pt_by_component(mesh->pntbl[i], m1y);
        sub_transform_buffer[i][X] = trans_pt_by_component(mesh->pntbl[i], m0x);
		
        /** Retrieves the result of the division **/
		//inverseZ = *DVDNTL;
		inverseZ = zTable[(screen_transform_buffer[i].pnt[Z]>>16)];
		//inverseZ = fxdiv(scrn_dist, screen_transform_buffer[i].pnt[Z]);
        /**Transform X and Y to screen space**/
       // screen_transform_buffer[i].pnt[X] = fxm(sub_transform_buffer[i][X], inverseZ)>>SCR_SCALE_X;
       // screen_transform_buffer[i].pnt[Y] = fxm(sub_transform_buffer[i][Y], inverseZ)>>SCR_SCALE_Y;
        screen_transform_buffer[i].pnt[X] = fxm(sub_transform_buffer[i][X], inverseZ)>>SCR_SCALE_X;
        screen_transform_buffer[i].pnt[Y] = fxm(sub_transform_buffer[i][Y], inverseZ)>>SCR_SCALE_Y;
 
        //Screen Clip Flags for on-off screen decimation
		screen_transform_buffer[i].clipFlag = ((screen_transform_buffer[i].pnt[X]) > TV_HALF_WIDTH) ? SCRN_CLIP_X : 0; 
		screen_transform_buffer[i].clipFlag |= ((screen_transform_buffer[i].pnt[X]) < -TV_HALF_WIDTH) ? SCRN_CLIP_NX : screen_transform_buffer[i].clipFlag; 
		screen_transform_buffer[i].clipFlag |= ((screen_transform_buffer[i].pnt[Y]) > TV_HALF_HEIGHT) ? SCRN_CLIP_Y : screen_transform_buffer[i].clipFlag;
		screen_transform_buffer[i].clipFlag |= ((screen_transform_buffer[i].pnt[Y]) < -TV_HALF_HEIGHT) ? SCRN_CLIP_NY : screen_transform_buffer[i].clipFlag;
		screen_transform_buffer[i].clipFlag |= ((screen_transform_buffer[i].pnt[Z]) <= SUBDIVISION_NEAR_PLANE) ? CLIP_Z : screen_transform_buffer[i].clipFlag;

}
transVerts[0] += mesh->nbPoint;

for(unsigned int i = 0; i < mesh->nbPolygon; i++)
{
		
	flags = mesh->attbl[i].render_data_flags;
		
	//////////////////////////////////////////////////////////////
	// Load the points
	// This is a small optimization to avoid doing the address calculation for each point multiple times
	//////////////////////////////////////////////////////////////
	ptv[0] = &screen_transform_buffer[mesh->pltbl[i].vertices[0]];
	ptv[1] = &screen_transform_buffer[mesh->pltbl[i].vertices[1]];
	ptv[2] = &screen_transform_buffer[mesh->pltbl[i].vertices[2]];
	ptv[3] = &screen_transform_buffer[mesh->pltbl[i].vertices[3]];

	if(ptv[0]->clipFlag & ptv[1]->clipFlag & ptv[2]->clipFlag & ptv[3]->clipFlag) continue;
	
	stv[0] = &sub_transform_buffer[mesh->pltbl[i].vertices[0]][X];
	stv[1] = &sub_transform_buffer[mesh->pltbl[i].vertices[1]][X];
	stv[2] = &sub_transform_buffer[mesh->pltbl[i].vertices[2]][X];
	stv[3] = &sub_transform_buffer[mesh->pltbl[i].vertices[3]][X];
	
	//At this point, we know we have at least four vertices.
	sub_vert_cnt = 4;
	//At this point, we know we have at least one polygon.
	sub_poly_cnt = 1;	
	
	subdivided_polygons[0][0] = 0;
	subdivided_points[0][X] = stv[0][X];
	subdivided_points[0][Y] = stv[0][Y];
	subdivided_points[0][Z] = stv[0][Z];
	
	subdivided_polygons[0][1] = 1;
	subdivided_points[1][X] = stv[1][X];
	subdivided_points[1][Y] = stv[1][Y];
	subdivided_points[1][Z] = stv[1][Z];
	
	subdivided_polygons[0][2] = 2;
	subdivided_points[2][X] = stv[2][X];
	subdivided_points[2][Y] = stv[2][Y];
	subdivided_points[2][Z] = stv[2][Z];
	
	subdivided_polygons[0][3] = 3;
	subdivided_points[3][X] = stv[3][X];
	subdivided_points[3][Y] = stv[3][Y];
	subdivided_points[3][Z] = stv[3][Z];

	if(!(flags & GV_FLAG_DISPLAY)) continue;
		
	//////////////////////////////////////////////////////////////
	// Screen-space back face culling segment. Will also avoid if the plane is flagged as dual-plane.
	//////////////////////////////////////////////////////////////
	if(flags & GV_FLAG_SINGLE)
	{
		dual_plane = 0;
		//Why does this work? What is this doing?
		//This is pretty much following Wikipedia's article on backface culling. Note the other method used in render.c kind of also does.
		//The difference is this is view-space, but not screenspace.
		//In screenspace, you can use a cross product to find the normal of the polygon. This fails for polygons distorted by the near plane.
		//Instead, we can do the math in view-space. However, doing the math in view-space requires us to transform the normal.
		//That is why this method is technically inferior, but it is a very similar idea: is the normal facing towards or away the polygon.
		//In screen-space, that polygon in/out vector coincides with the screen's Z axis.
		//In view-space, that polygon in/out vector coincides with the plane of the polygon. The normal, you understand?
		//So we are just trying to find out if the normal, in VIEW-SPACE, is facing away or towards the polygon.
		//If it is facing away, that polygon is therefore facing away from the perspective, and should be culled.
		int t_norm[3] = {0, 0, 0};
		t_norm[0] = fxdot(mesh->nmtbl[i], m0x);
		t_norm[1] = fxdot(mesh->nmtbl[i], m1y);
		t_norm[2] = fxdot(mesh->nmtbl[i], m2z);
		int tdot = fxdot(t_norm, stv[0]);
		
		if(tdot > 0) continue;
	
	} else {
		dual_plane = 1;
	}
	
	///////////////////////////////////////////
	// Just a side note:
	// Doing subdivision in screen-space **does not work**.
	// Well, technically it works, but warping is experienced. It looks pretty awful.
	///////////////////////////////////////////
	used_textures[0] = mesh->attbl[i].uv_id;

	tile_rules[0] = mesh->attbl[i].tile_information & 0x3;
	tile_rules[1] = (mesh->attbl[i].tile_information>>2) & 0x3;
	tile_rules[2] = (mesh->attbl[i].tile_information>>4) & 0x3;
	
	register int min_z = JO_MIN(JO_MIN(stv[0][Z], stv[1][Z]), JO_MIN(stv[2][Z], stv[3][Z]));
	if(!tile_rules[0] || (flags & GV_FLAG_NDIV) || min_z > z_rules[0])
	{
		//In case subdivision was not enabled, we need to copy from screen_transform_buffer to ssh2_vert_area.
		ssh2VertArea[0].pnt[X] = ptv[0]->pnt[X];
		ssh2VertArea[0].pnt[Y] = ptv[0]->pnt[Y];
		ssh2VertArea[0].pnt[Z] = ptv[0]->pnt[Z];
		ssh2VertArea[1].pnt[X] = ptv[1]->pnt[X];
		ssh2VertArea[1].pnt[Y] = ptv[1]->pnt[Y];
		ssh2VertArea[1].pnt[Z] = ptv[1]->pnt[Z];
		ssh2VertArea[2].pnt[X] = ptv[2]->pnt[X];
		ssh2VertArea[2].pnt[Y] = ptv[2]->pnt[Y];
		ssh2VertArea[2].pnt[Z] = ptv[2]->pnt[Z];
		ssh2VertArea[3].pnt[X] = ptv[3]->pnt[X];
		ssh2VertArea[3].pnt[Y] = ptv[3]->pnt[Y];
		ssh2VertArea[3].pnt[Z] = ptv[3]->pnt[Z];
		ssh2VertArea[0].clipFlag = 0;
		//Because I fucked up when transcribing the texture tables, we gotta -1.
		used_textures[0] -= 1;
		//Subdivision disabled end stub
	} else {
		///////////////////////////////////////////
		// The subdivision rules were pre-calculated by the converter tool.
		// In addition, the base texture (the uv_id) was also pre-calculated by the tool.
		///////////////////////////////////////////
		subdivide_tile(0, 3, 0, mesh->attbl[i].uv_id);
		///////////////////////////////////////////
		// Screenspace Transform of SUBDIVIDED Vertices
		// v = subdivided point index
		for(int v = 0; v < sub_vert_cnt; v++)
		{
			//Push to near-plane for CURRENT vertex
			ssh2VertArea[v].pnt[Z] = (subdivided_points[v][Z] > SUBDIVISION_NEAR_PLANE) ? subdivided_points[v][Z] : SUBDIVISION_NEAR_PLANE;
			//Set division for CURRENT vertex
			//SetFixDiv(scrn_dist, ssh2VertArea[v].pnt[Z]);
			//Get 1/z for CURRENT vertex
			//inverseZ = *DVDNTL;
			inverseZ = zTable[ssh2VertArea[v].pnt[Z]>>16];
			//Transform to screen-space
			ssh2VertArea[v].pnt[X] = fxm(subdivided_points[v][X], inverseZ)>>SCR_SCALE_X;
			ssh2VertArea[v].pnt[Y] = fxm(subdivided_points[v][Y], inverseZ)>>SCR_SCALE_Y;
			//Screen Clip Flags for on-off screen decimation
			ssh2VertArea[v].clipFlag = ((ssh2VertArea[v].pnt[X]) > TV_HALF_WIDTH) ? SCRN_CLIP_X : 0; 
			ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[X]) < -TV_HALF_WIDTH) ? SCRN_CLIP_NX : 0; 
			ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) > TV_HALF_HEIGHT) ? SCRN_CLIP_Y : 0;
			ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) < -TV_HALF_HEIGHT) ? SCRN_CLIP_NY : 0;
			ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Z]) <= SUBDIVISION_NEAR_PLANE) ? CLIP_Z : 0;
			// clipping(&ssh2VertArea[v], USER_CLIP_INSIDE);
		}
		transVerts[0] += sub_vert_cnt;
	//Subdivision activation end stub
	}
	///////////////////////////////////////////
	//
	// Z-sort Insertion & Command Arrangement of Polygons
	// j = subdivided polygon index
	//
	///////////////////////////////////////////
	if(ssh2SentPolys[0] + sub_poly_cnt > MAX_SSH2_SENT_POLYS) return;
	//Used to account for the texture # being offset when UV cut textures are generated
	int texno_offset = ((mesh->attbl[i].texno - ent->base_texture) * UV_CUT_COUNT) + mesh->attbl[i].texno;
	unsigned short usedCMDCTRL = (flags & GV_FLAG_POLYLINE) ? VDP1_POLYLINE_CMDCTRL : VDP1_BASE_CMDCTRL;
	for(int j = 0; j < sub_poly_cnt; j++)
	{
		transPolys[0]++;
		ptv[0] = &ssh2VertArea[subdivided_polygons[j][0]];
		ptv[1] = &ssh2VertArea[subdivided_polygons[j][1]];
		ptv[2] = &ssh2VertArea[subdivided_polygons[j][2]];
		ptv[3] = &ssh2VertArea[subdivided_polygons[j][3]];
		flags = mesh->attbl[i].render_data_flags;
		 int offScrn = (ptv[0]->clipFlag & ptv[1]->clipFlag & ptv[2]->clipFlag & ptv[3]->clipFlag);
		///////////////////////////////////////////
		// Z-Sorting Stuff	
		// Uses weighted max
		///////////////////////////////////////////
		zDepthTgt = (JO_MAX(
		JO_MAX(ptv[0]->pnt[Z], ptv[2]->pnt[Z]),
		JO_MAX(ptv[1]->pnt[Z], ptv[3]->pnt[Z])) + 
		((ptv[0]->pnt[Z] + ptv[1]->pnt[Z] + ptv[2]->pnt[Z] + ptv[3]->pnt[Z])>>2))>>1;

		if(offScrn || zDepthTgt < NEAR_PLANE_DISTANCE || zDepthTgt > FAR_PLANE_DISTANCE) continue;
		///////////////////////////////////////////
		// These use UV-cut textures now, so it's like this.
		///////////////////////////////////////////
		specific_texture = texno_offset + used_textures[j];
		specific_texture = (flags & GV_FLAG_ANIM) ? mesh->attbl[i].texno : specific_texture;
		///////////////////////////////////////////
		// Flipping polygon such that vertice 0 is on-screen, or disable pre-clipping
		///////////////////////////////////////////
		flip = GET_FLIP_DATA(flags);
		preclipping(ptv, &flip, &pclp);
		///////////////////////////////////////////
		// Lighting Math
		// Using some approximation of an inverse squared law
		// The position of the polygon is treated as the average of points 0 and 2.
		///////////////////////////////////////////
		luma = 0;
		luma += fxdot(mesh->nmtbl[i], active_lights[0].ambient_light);
		//If the plane is dual-plane, add the absolute luma, instead of the signed luma.
		luma = (dual_plane) ? JO_ABS(luma) : luma;
		luma += active_lights[0].min_bright; 
		determine_colorbank(&colorBank, &luma);
		//Shift the color bank code to the appropriate bits
		colorBank<<=6;
		//Added later: In case of a polyline (or really, any untextured command),
		// the color for the draw command is defined by the draw command's "texno" or texture number data.
		// this texture number data however is inserted in the wrong parts of the draw command to be the color.
		// So here, we insert it into the correct place in the command table to be the drawn color.
		flags = (((flags & GV_FLAG_MESH)>>1) | ((flags & GV_FLAG_DARK)<<4))<<8;
		colorBank += (usedCMDCTRL == VDP1_BASE_CMDCTRL) ? 0 : mesh->attbl[i].texno;
		
		//depth cueing experiments
		depth_cueing(&zDepthTgt, &cue);
		
      ssh2SetCommand(ptv[0]->pnt, ptv[1]->pnt, ptv[2]->pnt, ptv[3]->pnt,
		usedCMDCTRL | flip, (VDP1_BASE_PMODE | flags) | pclp, //Reads flip value, mesh enable, and msb bit
		pcoTexDefs[specific_texture].SRCA, colorBank | cue, pcoTexDefs[specific_texture].SIZE, 0, zDepthTgt);
	}

}
	//////////////////////////////////////////////////////////////
	// Planar polygon subdivision rendering end stub
	//////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////
//Sector drawing routine
// Warning: Assembly ahead
// The assembly is a sad but necessary optimization; it saves like 15% on the frametime.
//////////////////////////////////////////////////////////////////
void	draw_sector(entity_t * ent, int sector_number, int viewport_sector)
{
	///////////////////////////////////////////
	// If the file is not yet loaded, do not try and render it.
	// If the entity type is not 'B' for BUILDING, do not try and render it as it won't have proper textures.
	///////////////////////////////////////////
	_sector * sct = &sectors[sector_number];
	if(ent->file_done != true) return;
	if(ent->type != MODEL_TYPE_SECTORED) return;
	if(!sct->nbPoint || !sct->pntbl) return; 
	GVPLY * mesh = ent->pol;
	
	sub_poly_cnt = 0;
	sub_vert_cnt = 0;
	unsigned short	colorBank = 0;
	int 	luma = 0;
	int 	zDepthTgt = 0;
	unsigned short	flags = 0;
	unsigned short	flip = 0;
	unsigned short	pclp = 0;

	vertex_t * ptv[5];
	int * stv[4];

    static MATRIX newMtx;
	static FIXED mmtx[12];
	
	//These can't have an orienation... eh, we'll do it anyway.
	slMultiMatrix((POINT *)ent->prematrix);
    slGetMatrix(newMtx);
	
	mmtx[0] = newMtx[X][X];
	mmtx[1] = newMtx[Y][X];
	mmtx[2] = newMtx[Z][X];
	mmtx[3] = newMtx[3][X];
	
	mmtx[4] = newMtx[X][Y];
	mmtx[5] = newMtx[Y][Y];
	mmtx[6] = newMtx[Z][Y];
	mmtx[7] = newMtx[3][Y];
	
	mmtx[8] = newMtx[X][Z];
	mmtx[9] = newMtx[Y][Z];
	mmtx[10] = newMtx[Z][Z];
	mmtx[11] = newMtx[3][Z];

	/**
	Rendering Planes
	Right now, this is slow. Very slow.
	**/
	
	int specific_texture = 0;
	int dual_plane = 0;
	int cue;
	
	//nbg_sprintf(2, 4, "pnts(%i)", sct->nbPoint);
	//nbg_sprintf(2, 5, "ply(%i)", sct->nbPoint);
	//nbg_sprintf(2, 6, "addr_pnt(%i)", sct->pntbl);
	//nbg_sprintf(2, 7, "addr_ply(%i)", sct->pltbl);
//First: Transform all of the vertices of the mesh to a buffer.
//We must use the sector's pntbl with an extra layer of deference to get the right vertex in the mesh's pntbl.


transVerts[0]+= sct->nbTileVert;
//	int		inverseZ = 0;
//for(unsigned int i = 0; i < sct->nbTileVert; i++)
//{
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////Vertice 3D Transformation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		//Vertex ID aliasing
//		int * v = sct->tvtbl[i];
//        /**calculate z**/
//        screen_transform_buffer[i].pnt[Z] = trans_pt_by_component(v, m2z);
//		sub_transform_buffer[i][Z] = screen_transform_buffer[i].pnt[Z];
//		screen_transform_buffer[i].pnt[Z] = (screen_transform_buffer[i].pnt[Z] > NEAR_PLANE_DISTANCE) ? screen_transform_buffer[i].pnt[Z] : NEAR_PLANE_DISTANCE;
//         /**Starts the division**/
//      //  SetFixDiv(scrn_dist, screen_transform_buffer[i].pnt[Z]);
//
//        /**Calculates X and Y while waiting for screenDist/z **/
//        sub_transform_buffer[i][Y] = trans_pt_by_component(v, m1y);
//        sub_transform_buffer[i][X] = trans_pt_by_component(v, m0x);
//		
//        /** Retrieves the result of the division **/
//		//inverseZ = *DVDNTL;
//		inverseZ = zTable[(screen_transform_buffer[i].pnt[Z]>>16)];
//		//inverseZ = fxdiv(scrn_dist, screen_transform_buffer[i].pnt[Z]);
//        /**Transform X and Y to screen space**/
//       // screen_transform_buffer[i].pnt[X] = fxm(sub_transform_buffer[i][X], inverseZ)>>SCR_SCALE_X;
//       // screen_transform_buffer[i].pnt[Y] = fxm(sub_transform_buffer[i][Y], inverseZ)>>SCR_SCALE_Y;
//        screen_transform_buffer[i].pnt[X] = fxm(sub_transform_buffer[i][X], inverseZ)>>SCR_SCALE_X;
//        screen_transform_buffer[i].pnt[Y] = fxm(sub_transform_buffer[i][Y], inverseZ)>>SCR_SCALE_Y;
// 
//        //Screen Clip Flags for on-off screen decimation
//		screen_transform_buffer[i].clipFlag = ((screen_transform_buffer[i].pnt[X]) > TV_HALF_WIDTH) ? SCRN_CLIP_X : 0; 
//		screen_transform_buffer[i].clipFlag |= ((screen_transform_buffer[i].pnt[X]) < -TV_HALF_WIDTH) ? SCRN_CLIP_NX : 0; 
//		screen_transform_buffer[i].clipFlag |= ((screen_transform_buffer[i].pnt[Y]) > TV_HALF_HEIGHT) ? SCRN_CLIP_Y : 0;
//		screen_transform_buffer[i].clipFlag |= ((screen_transform_buffer[i].pnt[Y]) < -TV_HALF_HEIGHT) ? SCRN_CLIP_NY : 0;
//		screen_transform_buffer[i].clipFlag |= ((screen_transform_buffer[i].pnt[Z]) <= SUBDIVISION_NEAR_PLANE) ? CLIP_Z : 0;
//}
	clip_settings[5] = (unsigned int)&mmtx[0];
	asm(
	"mov.w @(8,%[sct]),r0;" //Move sct->nbTileVert to r0. Used as loop count. (Decrement method)
	"mov r0,r1;"		//move sct->nbTileVert to r1 in preparation for address offset calculation in sct->tvtbl[i]
	"mov #12,r8;"		//Move #12 to r8 in preparation for address offset calculation in sct->tvtbl (12 bytes per entry)
	"mulu.w r8,r1;"		//Multiply, 16-bit, unsigned #12 by r1 (number of vertices); 12 bytes offset
	"sts macl,r1;"		//Move result back to r1 (clobber r1)
	"mov.l @(40,%[sct]),r8;" //Move sct->tvtbl to r8 in preparation for calculation of sct->tvtbl[i]
	"add r1,r8;"		//Add address offset in r1 to sct->tvtbl in r8 to get sct->tvtbl[sct->nbTileVert]
	"mov r0,r1;"		//Move r0 to r1 for address calculation in screen_transform_buffer AND subdivided_points 
	"shll2 r1;"
	"shll2 r1;"
	"mov r1,r2;"		//Move r1<<4 to r2 for address calculation for sub_transform_buffer; r1 is for screen_transform_buffer
	"add %[scrn_pnts],r1;" //Add address of screen_transform_buffer to address offset in r1
	"add %[sub_pnts],r2;" //Add address of sub_transform_buffer to address offset in r2
	//A decrement must happen on the pointers, but NOT on the loop counter.
	//The loop counts down from (sct->nbTileVert) to (0); when it equals zero, it will exit. So 0 is not included in the count.
	//However, zero MUST be included in the processed quantities; e.g. screen_transform_buffer[0] and sub_transform_buffer[0] are valid,
	//and in addition to that, screen_transform_buffer[sct->nbTileVert] and sub_transform_buffer[sct->nbTileVert.] are NOT valid.
	//So if we just decrement the pointers by 1, we'll land at the right spot.
	"add #-16,r1;" //screen_transform_buffer[i] = screen_transform_buffer[i-1]			
	"add #-16,r2;" //sub_transform_buffer[i] = sub_transform_buffer[i-1]	
	"add #-12,r8;" //sct->tvtbl[i] = sct->tvtbl[i-1]
	"mov.l @(16,%[clpSet]),r4;" //Move SUBDIVISION_NEAR_PLANE to r4
	"FORI:"
	"mov.l @(20,%[clpSet]),r3;" //Move &mmtx[0] to r3 (from clpSet[5])
	//I can use r5, r6, and r7 for the components
	"clrmac;"		//Clear the accumulator
	"mac.l @r3+,@r8+;" //mtx[0] * tvtbl[i][X] +
	"mac.l @r3+,@r8+;" //mtx[1] * tvtbl[i][Y] +
	"mac.l @r3+,@r8+;" //mtx[2] * tvtbl[i][Z] 
	"sts mach,r5;"
	"sts macl,r6;"		//(fixed point extraction from 64-bits)
	"xtrct r5,r6;"		//new X vector in r6
	"mov.l @r3+,r5;"	//move mtx[3] to r5
	"add r5,r6;"		//add mtx[3] to X vector to get X position
	"mov.l r6,@r2;"		//move to sub_transform_buffer[i][X]
	"clrmac;"
	"add #-12,r8;" //Reset the pointer in r8
	"mac.l @r3+,@r8+;"	//mtx[4] * tvtbl[i][X] +
	"mac.l @r3+,@r8+;"	//mtx[5] * tvtbl[i][Y] +
	"mac.l @r3+,@r8+;"	//mtx[6] * tvtbl[i][Z] 
	"sts mach,r5;"
	"sts macl,r6;"		//(fixed point extraction from 64-bits)
	"xtrct r5,r6;"		//new Y vector in r6
	"mov.l @r3+,r5;"	//move mtx[7] to r5
	"add r5,r6;"		//add mtx[7] to Y vector to get Y position
	"mov.l r6,@(4,r2);"	//move to sub_transform_buffer[i][Y]
	"clrmac;"
	"add #-12,r8;" //Reset the pointer in r8
	"mac.l @r3+,@r8+;"	//mtx[8] * tvtbl[i][X] +
	"mac.l @r3+,@r8+;"	//mtx[9] * tvtbl[i][Y] +
	"mac.l @r3+,@r8+;"	//mtx[10] * tvtbl[i][Z] 
	"sts mach,r5;"
	"sts macl,r6;"		//(fixed point extraction from 64-bits)
	"xtrct r5,r6;"		//new Z vector in r6
	"mov.l @r3+,r5;"	//move mtx[11] to r5
	"add r5,r6;"		//add mtx[11] to Z vector to get Z position
	"mov.l r6,@(8,r2);"	//move to sub_transform_buffer[i][Z]
	
	"mov #0,r5;" //Move literal 0 to r5 in preparation for clipFlag calculation
	"mov.l @(8,r2),r3;" //Move sub_transform_buffer[i][Z] to r3

	"cmp/gt r3,r4;" //If r4 > r3 (otherwise read as r3 < r4); if SUBDIVISION_NEAR_PLANE > pnt[Z]; T bit to 1
	"bf GUDZA;" //If T bit is 0,jump to HIGHZ
	"mov r4,r3;" //Set used Z in r3 to SUBDIVISION_NEAR_PLANE
	"mov #16,r5;" //Move CLIP_Z (literal #16) to clipFlag
	"GUDZA:"
	
	"mov.l r3,@(8,r1);" //Move sub_transform_buffer[i][Z] to screen_transform_buffer[i].pnt[Z] 
	"shlr16 r3;" //Shift sub_transform_buffer[i][Z] right 16 times in preparation for calculation of address offset
	"shll2 r3;" //Shift sub_transform_buffer[i][Z] left twice in calculation for address offset in zTbl
	"add %[zTbl],r3;" //Add address of zTable to address offset in r3
	"mov.l @r3,r6;" //Obtain inverse_z in r6
	
	"mov.l @r2,r7;" //Move sub_pnts[i][X] to r7
	"dmuls.l r7,r6;" // pnt[X] * inverse_z -> MACH/MACL
	"sts mach,r7;"	//we only want the upper 32 bits (screen coordinates)
	"mov.l r7,@r1;" //Move pnt[X] to screen_transform_buffer.pnt[X]
	
	"mov.l @%[clpSet],r3;" //Move clpSet[0] to r3 (TV_HALF_WIDTH)
	"cmp/gt r3,r7;" //Perform if(pnt[X] > TV_HALF_WIDTH), T = 1
	"bf NOCLIPXA;" //If T = 0, do not clip X; jump ahead.
	"mov #1,r3;" //Clobber R3; replace with constant #1 to represent SCRN_CLIP_X
	"or r3,r5;" //OR SCRN_CLIP_X with r5 (clipFlag)
	"NOCLIPXA:" //(Proceeding to test if X is < -TV_HALF_WIDTH)
	"mov.l @(4,%[clpSet]),r3;" //Move -TV_HALF_WIDTH from clip_settings to r3
	"cmp/gt r7,r3;" //Perform if(-TV_HALF_WIDTH > pnt[X]), T = 1
	"bf NOCLIPNXA;" //If T = 0, do not clip NX; jump ahead.
	"mov #2,r3;" //Clobber R3; replace with constant #2 to represent SCRN_CLIP_NX
	"or r3,r5;" //OR SCRN_CLIP_NX with r5 (clipFlag)
	"NOCLIPNXA:"

	"mov.l @(4,r2),r7;" //Move sub_pnts[i][Y] to r7
	"dmuls.l r7,r6;" //pnt[Y] * inverse_z -> MACH/MACL
	"sts mach,r7;" //get upper 32 bits of that operation (should not be more than 16, to be fair)
	"mov.l r7,@(4,r1);"//Move pnt[Y] to screen_transform_buffer.pnt[Y]
	
	"mov.l @(8,%[clpSet]),r3;" //Move TV_HALF_HEIGHT from clpSet[2] to r3
	"cmp/gt r3,r7;" //Perform if(pnt[Y] > TV_HALF_HEIGHT), T = 1
	"bf NOCLIPYA;" //If T = 0, do not clip X; jump ahead.
	"mov #4,r3;" //Clobber R3; replace with constant #4 to represent SCRN_CLIP_Y
	"or r3,r5;" //OR SCRN_CLIP_X with r5 (clipFlag)
	"NOCLIPYA:" //(Proceeding to test if Y is < -TV_HALF_HEIGHT)
	"mov.l @(12,%[clpSet]),r3;"
	"cmp/gt r7,r3;" //Perform if(-TV_HALF_HEIGHT > pnt[X]), T = 1
	"bf NOCLIPNYA;" //If T = 0, do not clip NX; jump ahead.
	"mov #8,r3;" //Clobber R3; replace with constant #8 to represent SCRN_CLIP_NY
	"or r3,r5;" //OR SCRN_CLIP_NX with r5 (clipFlag)
	"NOCLIPNYA:"
	
	"mov.l r5,@(12,r1);" //Move clipFlag to screen_transform_buffer[i].clipFlag
	"add #-16,r1;" //screen_transform_buffer[i] = screen_transform_buffer[i-1]
	"add #-16,r2;" //sub_transform_buffer[i] = sub_transform_buffer[i-1]
	"add #-24,r8;" //sct->tvtbl[i+1] = sct->tvtbl[i-1]
	"dt r0;" //Decrement R0 (r0 = r0 - 1)
	"bf FORI;" //In case R0 >= 0, branch to loop
	"nop;"
	: 																//OUT
	:	[scrn_pnts] "p" (&screen_transform_buffer[0]), [sub_pnts] "p" (&sub_transform_buffer[0]), [zTbl] "p" (&zTable[0]), [sct] "p" (sct), [clpSet] "p" (&clip_settings[0]) //IN
	:	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "mach", "macl"										//CLOBBERS
	); //5 input registers

//Copy the portal list 
//If we are in sector A, we should disable the portals when drawing sector A.
int intersecting_portal = 0;
int adjacent_has_portal = 0;
int is_adjacent = adjacentSectors[sector_number];

for(int  i = 0; i < (*current_portal_count); i++)
{
	_portal * scene_port = &scene_portals[i];
	_portal * used_port = &used_portals[i];
	
	if(sector_number == viewport_sector)
	{
		used_port->type = 0;
		continue;
	}
	
	if(((scene_port->sectorB == sector_number && scene_port->sectorA == viewport_sector)
		|| (scene_port->sectorB == viewport_sector && scene_port->sectorA == sector_number))
		&& scene_port->type & PORTAL_INTERSECTING)
	{
		intersecting_portal = 1;
	}
	
	if(scene_port->sectorA == INVALID_SECTOR || scene_port->sectorB == INVALID_SECTOR)
	{
		used_port->type = 0;
		continue;
	}
	
	if(scene_port->sectorA == viewport_sector && sector_number == scene_port->sectorA)
	{
		used_port->type = 0;
		continue;
	}
	if(scene_port->sectorB == viewport_sector && sector_number == scene_port->sectorB)
	{
		used_port->type = 0;
		continue;
	}
	//If we are in the sector the portal belongs to and the portal is NOT backfaced, disable it.
	if(scene_port->sectorA == viewport_sector && scene_port->backface == 1)
	{
		used_port->type = 0;
		continue;
	}
	//If we are NOT in the sector belongs to and the portal IS backfaced, disable it.
	if(scene_port->sectorA != viewport_sector && scene_port->backface == 0)
	{
		used_port->type = 0;
		continue;
	}
	
	//Now, for branches in which the sector drawn IS a primary adjacent:
	if(is_adjacent)
	{
		//Bro, this is really, really complicated.
		//We have built a negatively-biased system; there are conditions which DISABLE the portal, otherwise it's turned on.
		//This might not have been the smartest way to do it...
		//These conditions check if the adjacent sector has any portals which border the two sectors.
		//In case there aren't any which border an adjacent sector, no portals should be used to draw that sector.
		if(scene_port->sectorA == sector_number && scene_port->sectorB == viewport_sector)
		{
			adjacent_has_portal = 1;
		}
		if(scene_port->sectorB == sector_number && scene_port->sectorA == viewport_sector)
		{
			adjacent_has_portal = 1;
		}
		
		//For a primary adjacent sector, what conditions DISABLE the portal?
		//If the sector is not sectorA and not sectorB.
		//
		if(scene_port->sectorA != sector_number && scene_port->sectorB != sector_number)
		{
			used_port->type = 0;
			continue;
		}

	}		
	
	
	*used_port = *scene_port;
}

//If this sector has a portal that is intersecting the view plane, and we are in sectorA or sectorB of the portal,
//disable all portals which pertain to drawing sectorA and sectorB.
//This is explicitly structured so it does not disable portals for sectors that pertain to sectorA OR sectorB;
//only those which border sectorA AND sectorB, wherein the viewport_sector and drawn sector must be A and B.
//This also disables portals in case we are drawing an adjacent sector and there are no portals which border viewport_sector<->sector_number.
if(intersecting_portal || (is_adjacent && !adjacent_has_portal))
{
	for(int  i = 0; i < (*current_portal_count); i++)
	{
		_portal * used_port = &used_portals[i];
		used_port->type = 0;
	}
}


run_winder_prog(sct->nbTileVert, current_portal_count, (void*)&screen_transform_buffer[0]);

//Draw Route:
// Per Plane
//		Per Tile
//			Per Subdivided Polygon
//				Process into draw commands
 
for(unsigned int p = 0; p < sct->nbPolygon; p++)
{
	//Polygon ID alias
	int alias = sct->pltbl[p];
	
	flags = mesh->attbl[alias].render_data_flags;
	if(!(flags & GV_FLAG_DISPLAY)) continue;
	
	//////////////////////////////////////////////////////////////
	// Screen-space back face culling segment. Will also avoid if the plane is flagged as dual-plane.
	//////////////////////////////////////////////////////////////
	if(flags & GV_FLAG_SINGLE)
	{
		dual_plane = 0;
		//Why does this work? What is this doing?
		//This is pretty much following Wikipedia's article on backface culling. Note the other method used in render.c kind of also does.
		//The difference is this is view-space, but not screenspace.
		//In screenspace, you can use a cross product to find the normal of the polygon. This fails for polygons distorted by the near plane.
		//Instead, we can do the math in view-space. However, doing the math in view-space requires us to transform the normal.
		//That is why this method is technically inferior, but it is a very similar idea: is the normal facing towards or away the polygon.
		//In screen-space, that polygon in/out vector coincides with the screen's Z axis.
		//In view-space, that polygon in/out vector coincides with the plane of the polygon. The normal, you understand?
		//So we are just trying to find out if the normal, in VIEW-SPACE, is facing away or towards the polygon.
		//If it is facing away, that polygon is therefore facing away from the perspective, and should be culled.
		int t_norm[3] = {0, 0, 0};
		t_norm[0] = fxdot(mesh->nmtbl[alias], &mmtx[0]);
		t_norm[1] = fxdot(mesh->nmtbl[alias], &mmtx[4]);
		t_norm[2] = fxdot(mesh->nmtbl[alias], &mmtx[8]);
		int tdot = fxdot(t_norm, sub_transform_buffer[sct->tltbl[sct->plStart[p]].vertices[0]]);
		if(tdot > 0) continue;
	} else {
		dual_plane = 1;
	}
	
	///////////////////////////////////////////
	// Just a side note:
	// Doing subdivision in screen-space **does not work**.
	// Well, technically it works, but warping is experienced. It looks pretty awful.
	///////////////////////////////////////////
	tile_rules[0] = mesh->attbl[alias].tile_information & 0x3;
	tile_rules[1] = (mesh->attbl[alias].tile_information>>2) & 0x3;
	tile_rules[2] = (mesh->attbl[alias].tile_information>>4) & 0x3;
	
	used_textures[0] = mesh->attbl[alias].uv_id;
	
	//Used to account for the texture # being offset when UV cut textures are generated
	int texno_offset = ((mesh->attbl[alias].texno - ent->base_texture) * UV_CUT_COUNT) + mesh->attbl[alias].texno;
	unsigned short usedCMDCTRL = ((flags & GV_FLAG_POLYLINE)) ? VDP1_POLYLINE_CMDCTRL : VDP1_BASE_CMDCTRL;
	
	for(unsigned int i = 0; i < sct->nbTile[p]; i++)
	{
		_quad * tile = &sct->tltbl[sct->plStart[p] + i];
		//////////////////////////////////////////////////////////////
		// Load the points
		// These use aliased polygons in the sector because they deal with memory in the transform buffer.
		// This is a small optimization to avoid doing the address calculation for each point multiple times
		//////////////////////////////////////////////////////////////
		ptv[0] = &screen_transform_buffer[tile->vertices[0]];
		ptv[1] = &screen_transform_buffer[tile->vertices[1]];
		ptv[2] = &screen_transform_buffer[tile->vertices[2]];
		ptv[3] = &screen_transform_buffer[tile->vertices[3]];

		//////////////////////////////////////////////////////////////
		// This looks ugly, but I'm surprised it works... this is pretty efficient.
		// (Hypothetically, anyway; that address calculation is probably pretty ugly)
		volatile int * flag0 = (volatile int *)(((unsigned int)&ptv[0]->clipFlag)|UNCACHE);
		volatile int * flag1 = (volatile int *)(((unsigned int)&ptv[1]->clipFlag)|UNCACHE);
		volatile int * flag2 = (volatile int *)(((unsigned int)&ptv[2]->clipFlag)|UNCACHE);
		volatile int * flag3 = (volatile int *)(((unsigned int)&ptv[3]->clipFlag)|UNCACHE);
		volatile int check;
		do{
			check = *flag0 & *flag1 & *flag2 & *flag3;
			__asm__("nop;");
		}while((!(check & DSP_CLIP_CHECK)) && *current_portal_count);
		
		//If the tile is off-screen, don't draw it or any part of it.
		if(check & JUST_CLIP_FLAGS) continue;
		
		stv[0] = &sub_transform_buffer[tile->vertices[0]][X];
		stv[1] = &sub_transform_buffer[tile->vertices[1]][X];
		stv[2] = &sub_transform_buffer[tile->vertices[2]][X];
		stv[3] = &sub_transform_buffer[tile->vertices[3]][X];
		
		//At this point, we know we have at least four vertices.
		sub_vert_cnt = 4;
		//At this point, we know we have at least one polygon.
		sub_poly_cnt = 1;	
		
		subdivided_polygons[0][0] = 0;
		subdivided_points[0][X] = stv[0][X];
		subdivided_points[0][Y] = stv[0][Y];
		subdivided_points[0][Z] = stv[0][Z];
		
		subdivided_polygons[0][1] = 1;
		subdivided_points[1][X] = stv[1][X];
		subdivided_points[1][Y] = stv[1][Y];
		subdivided_points[1][Z] = stv[1][Z];
		
		subdivided_polygons[0][2] = 2;
		subdivided_points[2][X] = stv[2][X];
		subdivided_points[2][Y] = stv[2][Y];
		subdivided_points[2][Z] = stv[2][Z];
		
		subdivided_polygons[0][3] = 3;
		subdivided_points[3][X] = stv[3][X];
		subdivided_points[3][Y] = stv[3][Y];
		subdivided_points[3][Z] = stv[3][Z];
			
		register int min_z = JO_MIN(JO_MIN(stv[0][Z], stv[1][Z]), JO_MIN(stv[2][Z], stv[3][Z]));
		if(!tile_rules[0] || (flags & GV_FLAG_NDIV) || min_z > z_rules[0])
		{
			//In case subdivision was not enabled, we need to copy from screen_transform_buffer to ssh2_vert_area.
			ssh2VertArea[0].pnt[X] = ptv[0]->pnt[X];
			ssh2VertArea[0].pnt[Y] = ptv[0]->pnt[Y];
			ssh2VertArea[0].pnt[Z] = ptv[0]->pnt[Z];
			ssh2VertArea[1].pnt[X] = ptv[1]->pnt[X];
			ssh2VertArea[1].pnt[Y] = ptv[1]->pnt[Y];
			ssh2VertArea[1].pnt[Z] = ptv[1]->pnt[Z];
			ssh2VertArea[2].pnt[X] = ptv[2]->pnt[X];
			ssh2VertArea[2].pnt[Y] = ptv[2]->pnt[Y];
			ssh2VertArea[2].pnt[Z] = ptv[2]->pnt[Z];
			ssh2VertArea[3].pnt[X] = ptv[3]->pnt[X];
			ssh2VertArea[3].pnt[Y] = ptv[3]->pnt[Y];
			ssh2VertArea[3].pnt[Z] = ptv[3]->pnt[Z];
			//We only need to set one clip flag to prevent it from being decimated.
			//We already know the polygon is on-screen from earlier.
			ssh2VertArea[0].clipFlag = 0;
			//Because I fucked up when transcribing the texture tables, we gotta -1.
			used_textures[0] = mesh->attbl[alias].uv_id - 1;
			//Subdivision disabled end stub
		} else {
			///////////////////////////////////////////
			// The subdivision rules were pre-calculated by the converter tool.
			// In addition, the base texture (the uv_id) was also pre-calculated by the tool.
			///////////////////////////////////////////
			subdivide_tile(0, 3, 0, mesh->attbl[alias].uv_id);
			///////////////////////////////////////////
			// Screenspace Transform of SUBDIVIDED Vertices
			// v = subdivided point index
			// testing_planes[i] = plane data index
			//for(int v = 0; v < sub_vert_cnt; v++)
			//{
			//	//Push to near-plane for CURRENT vertex
			//	ssh2VertArea[v].pnt[Z] = (subdivided_points[v][Z] > SUBDIVISION_NEAR_PLANE) ? subdivided_points[v][Z] : SUBDIVISION_NEAR_PLANE;
			//	//Set division for CURRENT vertex
			//	//SetFixDiv(scrn_dist, ssh2VertArea[v].pnt[Z]);
			//	//Get 1/z for CURRENT vertex
			//	//inverseZ = *DVDNTL;
			//	inverseZ = zTable[ssh2VertArea[v].pnt[Z]>>16];
			//	//Transform to screen-space
			//	ssh2VertArea[v].pnt[X] = fxm(subdivided_points[v][X], inverseZ)>>SCR_SCALE_X;
			//	ssh2VertArea[v].pnt[Y] = fxm(subdivided_points[v][Y], inverseZ)>>SCR_SCALE_Y;
			//	//Screen Clip Flags for on-off screen decimation
			//	ssh2VertArea[v].clipFlag = ((ssh2VertArea[v].pnt[X]) > TV_HALF_WIDTH) ? SCRN_CLIP_X : 0; 
			//	ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[X]) < -TV_HALF_WIDTH) ? SCRN_CLIP_NX : 0; 
			//	ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) > TV_HALF_HEIGHT) ? SCRN_CLIP_Y : 0;
			//	ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) < -TV_HALF_HEIGHT) ? SCRN_CLIP_NY : 0;
			//	ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Z]) <= SUBDIVISION_NEAR_PLANE) ? CLIP_Z : 0;
			//	// clipping(&ssh2VertArea[v], USER_CLIP_INSIDE);
			//}
			asm(
			"mov.w @%[subCnt],r0;" //Move sub_vert_cnt to r0. Used as loop count. (Decrement method)
			"mov r0,r1;"		//Move r0 to r1 for address calculation in ssh2VertArea AND subdivided_points
			"shll2 r1;"
			"shll2 r1;"
			"mov r1,r2;"		//Move r1<<4 to r2 for address calculation for subdivided_points; r1 is for ssh2VertArea
			"add %[scrn_pnts],r1;" //Add address of ssh2VertArea to address offset in r1
			"add %[sub_pnts],r2;" //Add address of subdivided_points to address offset in r2
			//A decrement must happen on the pointers, but NOT on the loop counter.
			//The loop counts down from (sub_vert_cnt) to (0); when it equals zero, it will exit. So 0 is not included in the count.
			//However, zero MUST be included in the processed quantities; e.g. ssh2VertArea[0] and subdivided_points[0] are valid,
			//and in addition to that, ssh2VertArea[sub_vert_cnt] and subdivided_points[sub_vert_cnt] are NOT valid.
			//So if we just decrement the pointers by 1, we'll land at the right spot.
			"add #-16,r1;" //ssh2VertArea[v] = ssh2VertArea[v-1]			
			"add #-16,r2;" //subdivided_points[v] = subdivided_points[v-1]	
			"mov.l @(16,%[clpSet]),r4;" //Move SUBDIVISION_NEAR_PLANE to r4
			"FORV:"
			"mov #0,r5;" //Move literal 0 to r5 in preparation for clipFlag calculation
			"mov.l @(8,r2),r3;" //Move subdivided_points[v][Z] to r3

			"cmp/gt r3,r4;" //If r4 > r3 (otherwise read as r3 < r4); if SUBDIVISION_NEAR_PLANE > pnt[Z]; T bit to 1
			"bf GUDZ;" //If T bit is 0,jump to HIGHZ
			"mov r4,r3;" //Set used Z in r3 to SUBDIVISION_NEAR_PLANE
			"mov #16,r5;" //Move CLIP_Z (literal #16) to clipFlag
			"GUDZ:"
			
			"mov.l r3,@(8,r1);" //Move subdivided_points[v][Z] to ssh2VertArea[v].pnt[Z] 
			"shlr16 r3;" //Shift subdivided_points[v][Z] right 16 times in preparation for calculation of address offset
			"shll2 r3;" //Shift subdivided_points[v][Z] left twice in calculation for address offset in zTbl
			"add %[zTbl],r3;" //Add address of zTable to address offset in r3
			"mov.l @r3,r6;" //Obtain inverse_z in r6
			
			"mov.l @r2,r7;" //Move sub_pnts[v][X] to r7
			"dmuls.l r7,r6;" // pnt[X] * inverse_z -> MACH/MACL
			"sts mach,r7;"	//we only want the upper 32 bits (screen coordinates)
			"mov.l r7,@r1;" //Move pnt[X] to ssh2VertArea.pnt[X]
			
			"mov.l @%[clpSet],r3;" //Move clpSet[0] to r3 (TV_HALF_WIDTH)
			"cmp/gt r3,r7;" //Perform if(pnt[X] > TV_HALF_WIDTH), T = 1
			"bf NOCLIPX;" //If T = 0, do not clip X; jump ahead.
			"mov #1,r3;" //Clobber R3; replace with constant #1 to represent SCRN_CLIP_X
			"or r3,r5;" //OR SCRN_CLIP_X with r5 (clipFlag)
			"NOCLIPX:" //(Proceeding to test if X is < -TV_HALF_WIDTH)
			"mov.l @(4,%[clpSet]),r3;" //Move -TV_HALF_WIDTH from clip_settings to r3
			"cmp/gt r7,r3;" //Perform if(-TV_HALF_WIDTH > pnt[X]), T = 1
			"bf NOCLIPNX;" //If T = 0, do not clip NX; jump ahead.
			"mov #2,r3;" //Clobber R3; replace with constant #2 to represent SCRN_CLIP_NX
			"or r3,r5;" //OR SCRN_CLIP_NX with r5 (clipFlag)
			"NOCLIPNX:"

			"mov.l @(4,r2),r7;" //Move sub_pnts[v][Y] to r7
			"dmuls.l r7,r6;" //pnt[Y] * inverse_z -> MACH/MACL
			"sts mach,r7;" //get upper 32 bits of that operation (should not be more than 16, to be fair)
			"mov.l r7,@(4,r1);"//Move pnt[Y] to ssh2VertArea.pnt[Y]
			
			"mov.l @(8,%[clpSet]),r3;" //Move TV_HALF_HEIGHT from clpSet[2] to r3
			"cmp/gt r3,r7;" //Perform if(pnt[Y] > TV_HALF_HEIGHT), T = 1
			"bf NOCLIPY;" //If T = 0, do not clip X; jump ahead.
			"mov #4,r3;" //Clobber R3; replace with constant #4 to represent SCRN_CLIP_Y
			"or r3,r5;" //OR SCRN_CLIP_X with r5 (clipFlag)
			"NOCLIPY:" //(Proceeding to test if Y is < -TV_HALF_HEIGHT)
			"mov.l @(12,%[clpSet]),r3;"
			"cmp/gt r7,r3;" //Perform if(-TV_HALF_HEIGHT > pnt[X]), T = 1
			"bf NOCLIPNY;" //If T = 0, do not clip NX; jump ahead.
			"mov #8,r3;" //Clobber R3; replace with constant #8 to represent SCRN_CLIP_NY
			"or r3,r5;" //OR SCRN_CLIP_NX with r5 (clipFlag)
			"NOCLIPNY:"
			
			"mov.l r5,@(12,r1);" //Move clipFlag to ssh2VertArea[v].clipFlag
			"add #-16,r1;" //ssh2VertArea[v] = ssh2VertArea[v-1]
			"add #-16,r2;" //subdivided_points[v] = subdivided_points[v-1]
			"dt r0;" //Decrement R0 (r0 = r0 - 1)
			"bf FORV;" //In case R0 >= 0, branch to loop
			"nop;"
			: 																//OUT
			:	[scrn_pnts] "p" (&ssh2VertArea[0]), [sub_pnts] "p" (&subdivided_points[0]), [zTbl] "p" (&zTable[0]), [subCnt] "p" (&sub_vert_cnt), [clpSet] "p" (&clip_settings[0]) //IN
			:	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "mach", "macl"										//CLOBBERS
			); //5 input registers
			transVerts[0] += sub_vert_cnt;
		//Subdivision activation end stub
		}

		///////////////////////////////////////////
		//
		// Z-sort Insertion & Command Arrangement of Polygons
		// j = subdivided polygon index
		//
		// For future reference, portal implementation of in-out culling will be FASTER on a per polygon basis.
		// The reason is we only have to purge & copy four flags instead of all in a tile at once.
		// Then the DSP can be set to work on those flags, and as a VDP1 optimization, we only care about it insofar as its result,
		// before the draw command is sent.
		// We are of course doing more total purges than we otherwise would, but the CPU shouldn't have to wait.
		//
		///////////////////////////////////////////
		if(ssh2SentPolys[0] + sub_poly_cnt > MAX_SSH2_SENT_POLYS) return;
		for(int j = 0; j < sub_poly_cnt; j++)
		{
			transPolys[0]++;
			ptv[0] = &ssh2VertArea[subdivided_polygons[j][0]];
			ptv[1] = &ssh2VertArea[subdivided_polygons[j][1]];
			ptv[2] = &ssh2VertArea[subdivided_polygons[j][2]];
			ptv[3] = &ssh2VertArea[subdivided_polygons[j][3]];
			int offScrn = (ptv[0]->clipFlag & ptv[1]->clipFlag & ptv[2]->clipFlag & ptv[3]->clipFlag & JUST_CLIP_FLAGS);
			if(offScrn) continue;
			///////////////////////////////////////////
			// Z-Sorting Stuff	
			// Uses weighted max
			///////////////////////////////////////////
			zDepthTgt = (JO_MAX(
			JO_MAX(ptv[0]->pnt[Z], ptv[2]->pnt[Z]),
			JO_MAX(ptv[1]->pnt[Z], ptv[3]->pnt[Z])) + 
			((ptv[0]->pnt[Z] + ptv[1]->pnt[Z] + ptv[2]->pnt[Z] + ptv[3]->pnt[Z])>>2))>>1;
		
			if(zDepthTgt < NEAR_PLANE_DISTANCE || zDepthTgt > FAR_PLANE_DISTANCE) continue;
			///////////////////////////////////////////
			// These use UV-cut textures now, so it's like this.
			///////////////////////////////////////////
			specific_texture = texno_offset + used_textures[j];
			specific_texture = (flags & GV_FLAG_ANIM) ? mesh->attbl[alias].texno : specific_texture;
			///////////////////////////////////////////
			// Flipping polygon such that vertice 0 is on-screen, or disable pre-clipping
			// Subdivided polygon drawing is incompatible with flip in the draw command; to flip, rotate the polygon.
			///////////////////////////////////////////
			flip = 0;
			preclipping(ptv, &flip, &pclp);
			///////////////////////////////////////////
			// Lighting Math
			// Using some approximation of an inverse squared law
			// The position of the polygon is treated as the average of points 0 and 2.
			///////////////////////////////////////////
			luma = 0;
			luma += fxdot(mesh->nmtbl[alias], active_lights[0].ambient_light);
			//If the plane is dual-plane, add the absolute luma, instead of the signed luma.
			luma = (dual_plane) ? JO_ABS(luma) : luma;
			luma += active_lights[0].min_bright; 
			determine_colorbank(&colorBank, &luma);
			//Shift the color bank code to the appropriate bits
			colorBank<<=6;
			//Added later: In case of a polyline (or really, any untextured command),
			// the color for the draw command is defined by the draw command's "texno" or texture number data.
			// this texture number data however is inserted in the wrong parts of the draw command to be the color.
			// So here, we insert it into the correct place in the command table to be the drawn color.
			int vdp1_flags = (((flags & GV_FLAG_MESH)>>1) | ((flags & GV_FLAG_DARK)<<4))<<8;
			colorBank += (usedCMDCTRL == VDP1_BASE_CMDCTRL) ? 0 : mesh->attbl[alias].texno;
			
			//depth cueing experiments
			depth_cueing(&zDepthTgt, &cue);
			
			ssh2SetCommand(ptv[0]->pnt, ptv[1]->pnt, ptv[2]->pnt, ptv[3]->pnt,
			usedCMDCTRL | flip, (VDP1_BASE_PMODE | vdp1_flags) | pclp, //Reads flip value, mesh enable, and msb bit
			pcoTexDefs[specific_texture].SRCA, colorBank | cue, pcoTexDefs[specific_texture].SIZE, 0, zDepthTgt);
		}
	
	}

}

}




