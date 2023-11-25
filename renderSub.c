//subrender.c
//This file compiled separately

#include <SL_DEF.H>
#include "def.h"
#include "mymath.h"
#include "render.h"

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
	
	
	
	short		rule_to_texture[4] = {0, 1, 4, 5};

	#define UV_CUT_COUNT (224)
	#define SUBDIVISION_NEAR_PLANE (15<<16)

		POINT		sub_transform_buffer[512];
		vertex_t	screen_transform_buffer[512];
		POINT		subdivided_points[128];
		short		subdivided_polygons[128][4]; //4 Vertex IDs of the subdivided_points
		short		used_textures[128];
		short	sub_poly_cnt = 0;
		short	sub_vert_cnt = 0;
		short	subdivision_rules[4]	= {0, 0, 0, 0};
		short	texture_rules[4]		= {16, 16, 16, 0};
		// **really** trying to squeeze the performance here
		int		z_rules[4]				= {500<<16, 66<<16, 33<<16, 0};

void	subdivide_plane(short start_point, short overwritten_polygon, short num_divisions, short total_divisions, short rootTex)
{

	//"Load" the original points (code shortening operation)
	FIXED * ptv[4];
	static char new_rule;

	ptv[0] = &subdivided_points[subdivided_polygons[overwritten_polygon][0]][X];
	ptv[1] = &subdivided_points[subdivided_polygons[overwritten_polygon][1]][X];
	ptv[2] = &subdivided_points[subdivided_polygons[overwritten_polygon][2]][X];
	ptv[3] = &subdivided_points[subdivided_polygons[overwritten_polygon][3]][X];
	
	if(ptv[0][Z] < 0 && ptv[1][Z] < 0 && ptv[2][Z] < 0 && ptv[3][Z] < 0) return;
	new_rule = subdivision_rules[total_divisions];
	rootTex = rootTex-1;
	used_textures[overwritten_polygon] = rootTex;
	if(num_divisions == 0 || subdivision_rules[total_divisions] == 0)
	{
		return;
	}

	//////////////////////////////////////////////////////////////////
	// Warning: There is no extreme polygon smallness exit.
	// It will mess with texture assignment so I got rid of it.
	// Unfortunately I think this needs to come back in some way to handle trapezoids.
	//////////////////////////////////////////////////////////////////
	//if(minLen <= 50 && maxLen <= 50)
	//{
		//return;
	//} 
	//////////////////////////////////////////////////////////////////
	// Quick check: If we are subdividing a polygon above the z level, stop further subdivision.
	// This is mostly useful in cases where a large polygon is being recursively subdivided and parts of it may be far away.
	//////////////////////////////////////////////////////////////////
	int polygon_minimum = JO_MIN(JO_MIN(ptv[0][Z], ptv[1][Z]),  JO_MIN(ptv[2][Z], ptv[3][Z]));
	if(polygon_minimum > z_rules[total_divisions])
	{
		return;
	}

	short tgt_pnt = start_point;
	
	short poly_a = overwritten_polygon; //Polygon A is a polygon ID we will overwrite (replace the original polygon)
	short poly_b = sub_poly_cnt;
	short poly_c = sub_poly_cnt+1;
	short poly_d = sub_poly_cnt+2;
	
	switch(new_rule)
	{
	case(SUBDIVIDE_XY):
	//////////////////////////////////////////////////////////////////
	// Subdivide by all rules / Subdivide polygon into four new quads
	//Turn 4 points into 9 points
	//Make the 4 new polygons
	//////////////////////////////////////////////////////////////////
	/*
	//Why break chirality? to comply with the texture coordinate system (more easily, anyway)
	0A			1A | 0B			1B
							
			A				B
							
	3A			2A | 3B			2B		
	
	0D			1D | 0C			1C
	
			C				D

	3D			2D | 3C			2C
	*/
	// Initial Conditions
	subdivided_polygons[poly_a][0] = subdivided_polygons[overwritten_polygon][0];
	subdivided_polygons[poly_b][1] = subdivided_polygons[overwritten_polygon][1];
	subdivided_polygons[poly_d][2] = subdivided_polygons[overwritten_polygon][2];
	subdivided_polygons[poly_c][3] = subdivided_polygons[overwritten_polygon][3];

	// Center
	// 
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[1][X] + 
										ptv[2][X] + ptv[3][X])>>2;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[1][Y] + 
										ptv[2][Y] + ptv[3][Y])>>2;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[1][Z] + 
										ptv[2][Z] + ptv[3][Z])>>2;
	

	subdivided_polygons[poly_a][2] = tgt_pnt;
	subdivided_polygons[poly_b][3] = tgt_pnt;
	subdivided_polygons[poly_d][0] = tgt_pnt;
	subdivided_polygons[poly_c][1] = tgt_pnt;

	tgt_pnt++;
	// 0 -> 1
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[1][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[1][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[1][Z])>>1;
	
	subdivided_polygons[poly_a][1] = tgt_pnt;
	subdivided_polygons[poly_b][0] = tgt_pnt;
	tgt_pnt++;
	// 1 -> 2
	subdivided_points[tgt_pnt][X] = (ptv[2][X] + ptv[1][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[2][Y] + ptv[1][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[2][Z] + ptv[1][Z])>>1;
	
	subdivided_polygons[poly_b][2] = tgt_pnt;
	subdivided_polygons[poly_d][1] = tgt_pnt;
	tgt_pnt++;
	// 3 -> 2
	subdivided_points[tgt_pnt][X] = (ptv[2][X] + ptv[3][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[2][Y] + ptv[3][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[2][Z] + ptv[3][Z])>>1;
	
	subdivided_polygons[poly_d][3] = tgt_pnt;
	subdivided_polygons[poly_c][2] = tgt_pnt;
	tgt_pnt++;
	// 3 -> 0
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[3][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[3][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[3][Z])>>1;
	
	subdivided_polygons[poly_a][3] = tgt_pnt;
	subdivided_polygons[poly_c][0] = tgt_pnt;
	tgt_pnt++;
	sub_vert_cnt = tgt_pnt;
	sub_poly_cnt += 3; //Only add 3, as there was already 1 polygon. It was split into four.
	
	///////////////////////////////////////////
	//	Recursively subdivide the polygon.
	//	Check the maximum Z of every new polygon.
	// 	If the maximum Z is less than zero, it's not on screen. No point in subdividing it any further.
	///////////////////////////////////////////
	subdivide_plane(sub_vert_cnt, poly_a, num_divisions-1, total_divisions+1, texIDs_cut_from_texID[rootTex][0]);
	subdivide_plane(sub_vert_cnt, poly_b, num_divisions-1, total_divisions+1, texIDs_cut_from_texID[rootTex][1]);
	subdivide_plane(sub_vert_cnt, poly_c, num_divisions-1, total_divisions+1, texIDs_cut_from_texID[rootTex][2]);
	subdivide_plane(sub_vert_cnt, poly_d, num_divisions-1, total_divisions+1, texIDs_cut_from_texID[rootTex][3]);
		
	break;
	case(SUBDIVIDE_Y):
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
	// Initial Conditions
	subdivided_polygons[poly_a][0] = subdivided_polygons[overwritten_polygon][0];
	subdivided_polygons[poly_a][1] = subdivided_polygons[overwritten_polygon][1];
	subdivided_polygons[poly_b][2] = subdivided_polygons[overwritten_polygon][2];
	subdivided_polygons[poly_b][3] = subdivided_polygons[overwritten_polygon][3];
	
	// 1 -> 2
	subdivided_points[tgt_pnt][X] = (ptv[2][X] + ptv[1][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[2][Y] + ptv[1][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[2][Z] + ptv[1][Z])>>1;
	
	subdivided_polygons[poly_a][2] = tgt_pnt;
	subdivided_polygons[poly_b][1] = tgt_pnt;
	tgt_pnt++;
	// 3 -> 0
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[3][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[3][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[3][Z])>>1;
	
	subdivided_polygons[poly_a][3] = tgt_pnt;
	subdivided_polygons[poly_b][0] = tgt_pnt;
	tgt_pnt++;
		
	sub_vert_cnt = tgt_pnt;
	sub_poly_cnt += 1; //Only add 1, as there was already 1 polygon. It was split in two.
	
	///////////////////////////////////////////
	//	Recursively subdivide the polygon.
	//	Check the maximum Z of every new polygon.
	// 	If the maximum Z is less than zero, it's not on screen. No point in subdividing it any further.
	///////////////////////////////////////////
	subdivide_plane(sub_vert_cnt, poly_a, num_divisions-1, total_divisions+1, texIDs_cut_from_texID[rootTex][0]);
	subdivide_plane(sub_vert_cnt, poly_b, num_divisions-1, total_divisions+1, texIDs_cut_from_texID[rootTex][1]);

	break;
	case(SUBDIVIDE_X):
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
	subdivided_polygons[poly_a][0] = subdivided_polygons[overwritten_polygon][0];
	subdivided_polygons[poly_a][3] = subdivided_polygons[overwritten_polygon][3];
	subdivided_polygons[poly_b][1] = subdivided_polygons[overwritten_polygon][1];
	subdivided_polygons[poly_b][2] = subdivided_polygons[overwritten_polygon][2];
		
	// 0 -> 1
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[1][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[1][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[1][Z])>>1;
	
	subdivided_polygons[poly_a][1] = tgt_pnt;
	subdivided_polygons[poly_b][0] = tgt_pnt;
	tgt_pnt++;
	// 3 -> 2
	subdivided_points[tgt_pnt][X] = (ptv[2][X] + ptv[3][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[2][Y] + ptv[3][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[2][Z] + ptv[3][Z])>>1;
	
	subdivided_polygons[poly_a][2] = tgt_pnt;
	subdivided_polygons[poly_b][3] = tgt_pnt;
	tgt_pnt++;
	
	sub_vert_cnt = tgt_pnt;
	sub_poly_cnt += 1; //Only add 1, as there was already 1 polygon. It was split in two.
	
	///////////////////////////////////////////
	//	Recursively subdivide the polygon.
	//	Check the maximum Z of every new polygon.
	// 	If the maximum Z is less than zero, it's not on screen. No point in subdividing it any further.
	///////////////////////////////////////////
	subdivide_plane(sub_vert_cnt, poly_a, num_divisions-1, total_divisions+1, texIDs_cut_from_texID[rootTex][0]);
	subdivide_plane(sub_vert_cnt, poly_b, num_divisions-1, total_divisions+1, texIDs_cut_from_texID[rootTex][1]);

	break;
	default:
	break;
	}
}

void	plane_rendering_with_subdivision(entity_t * ent)
{
	///////////////////////////////////////////
	// If the file is not yet loaded, do not try and render it.
	// If the entity type is not 'B' for BUILDING, do not try and render it as it won't have proper textures.
	///////////////////////////////////////////
	if(ent->file_done != true) return;
	if(ent->type != 'B') return;
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
	unsigned short vids[4];

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

	//int max_z = 0;
	//int min_z = 0;
	
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
        SetFixDiv(scrn_dist, screen_transform_buffer[i].pnt[Z]);

        /**Calculates X and Y while waiting for screenDist/z **/
        sub_transform_buffer[i][Y] = trans_pt_by_component(mesh->pntbl[i], m1y);
        sub_transform_buffer[i][X] = trans_pt_by_component(mesh->pntbl[i], m0x);
		
        /** Retrieves the result of the division **/
		inverseZ = *DVDNTL;

        /**Transform X and Y to screen space**/
        screen_transform_buffer[i].pnt[X] = fxm(sub_transform_buffer[i][X], inverseZ)>>SCR_SCALE_X;
        screen_transform_buffer[i].pnt[Y] = fxm(sub_transform_buffer[i][Y], inverseZ)>>SCR_SCALE_Y;
 
        //Screen Clip Flags for on-off screen decimation
		screen_transform_buffer[i].clipFlag = ((screen_transform_buffer[i].pnt[X]) > TV_HALF_WIDTH) ? SCRN_CLIP_X : 0; 
		screen_transform_buffer[i].clipFlag |= ((screen_transform_buffer[i].pnt[X]) < -TV_HALF_WIDTH) ? SCRN_CLIP_NX : screen_transform_buffer[i].clipFlag; 
		screen_transform_buffer[i].clipFlag |= ((screen_transform_buffer[i].pnt[Y]) > TV_HALF_HEIGHT) ? SCRN_CLIP_Y : screen_transform_buffer[i].clipFlag;
		screen_transform_buffer[i].clipFlag |= ((screen_transform_buffer[i].pnt[Y]) < -TV_HALF_HEIGHT) ? SCRN_CLIP_NY : screen_transform_buffer[i].clipFlag;
		screen_transform_buffer[i].clipFlag |= ((screen_transform_buffer[i].pnt[Z]) <= SUBDIVISION_NEAR_PLANE) ? CLIP_Z : screen_transform_buffer[i].clipFlag;
		transVerts[0]++;
}

for(unsigned int i = 0; i < mesh->nbPolygon; i++)
{
		sub_vert_cnt = 0;
		sub_poly_cnt = 0;
		
	flags = mesh->attbl[i].render_data_flags;
		
	//////////////////////////////////////////////////////////////
	// Load the points
	//////////////////////////////////////////////////////////////
		vids[0] = mesh->pltbl[i].vertices[0];
		vids[1] = mesh->pltbl[i].vertices[1];
		vids[2] = mesh->pltbl[i].vertices[2];
		vids[3] = mesh->pltbl[i].vertices[3];
		
		subdivided_polygons[0][0] = 0;
		subdivided_polygons[0][1] = 1;
		subdivided_polygons[0][2] = 2;
		subdivided_polygons[0][3] = 3;
		subdivided_points[0][X] = sub_transform_buffer[vids[0]][X];
		subdivided_points[0][Y] = sub_transform_buffer[vids[0]][Y];
		subdivided_points[0][Z] = sub_transform_buffer[vids[0]][Z];

		subdivided_points[1][X] = sub_transform_buffer[vids[1]][X];
		subdivided_points[1][Y] = sub_transform_buffer[vids[1]][Y];
		subdivided_points[1][Z] = sub_transform_buffer[vids[1]][Z];
		
		subdivided_points[2][X] = sub_transform_buffer[vids[2]][X];
		subdivided_points[2][Y] = sub_transform_buffer[vids[2]][Y];
		subdivided_points[2][Z] = sub_transform_buffer[vids[2]][Z];
		
		subdivided_points[3][X] = sub_transform_buffer[vids[3]][X];
		subdivided_points[3][Y] = sub_transform_buffer[vids[3]][Y];
		subdivided_points[3][Z] = sub_transform_buffer[vids[3]][Z];

		if(screen_transform_buffer[vids[0]].clipFlag
		& screen_transform_buffer[vids[1]].clipFlag
		& screen_transform_buffer[vids[2]].clipFlag
		& screen_transform_buffer[vids[3]].clipFlag) continue;
		 
	///////////////////////////////////////////
	//	Check the maximum Z of every new polygon.
	// 	This is the first polygon. So, if its maximum Z is too low, just discard it.
	///////////////////////////////////////////
	// max_z = JO_MAX(JO_MAX(subdivided_points[subdivided_polygons[0][0]][Z], subdivided_points[subdivided_polygons[0][1]][Z]),
			// JO_MAX(subdivided_points[subdivided_polygons[0][2]][Z], subdivided_points[subdivided_polygons[0][3]][Z]));
	// if(max_z <= SUBDIVISION_NEAR_PLANE) continue;
	
	//////////////////////////////////////////////////////////////
	// Portal stuff
	// This plane rendering really has a lot of garbage in it, doesn't it?
	//////////////////////////////////////////////////////////////
	if(flags & GV_FLAG_PORTAL && current_portal_count < MAX_SCENE_PORTALS)
	{
		scene_portals[current_portal_count].type = PORTAL_TYPE_ACTIVE;
		scene_portals[current_portal_count].type |= (flags & GV_FLAG_PORT_TYPE) ? PORTAL_OR_OCCLUDE : 0;
		scene_portals[current_portal_count].type |= PORTAL_TYPE_DUAL;
		for(int u = 0; u < 4; u++)
		{
		scene_portals[current_portal_count].verts[u][X] = screen_transform_buffer[vids[u]].pnt[X];
		scene_portals[current_portal_count].verts[u][Y] = screen_transform_buffer[vids[u]].pnt[Y];
		scene_portals[current_portal_count].verts[u][Z] = screen_transform_buffer[vids[u]].pnt[Z];
		}
		current_portal_count++;
	}
	if(!(flags & GV_FLAG_DISPLAY)) continue;
		 
	//////////////////////////////////////////////////////////////
	// Screen-space back face culling segment. Will also avoid if the plane is flagged as dual-plane.
	//////////////////////////////////////////////////////////////
	if(flags & GV_FLAG_SINGLE)
	{
		 int cross0 = (screen_transform_buffer[vids[1]].pnt[X] - screen_transform_buffer[vids[3]].pnt[X])
							* (screen_transform_buffer[vids[0]].pnt[Y] - screen_transform_buffer[vids[2]].pnt[Y]);
		 int cross1 = (screen_transform_buffer[vids[1]].pnt[Y] - screen_transform_buffer[vids[3]].pnt[Y])
							* (screen_transform_buffer[vids[0]].pnt[X] - screen_transform_buffer[vids[2]].pnt[X]);
		dual_plane = 0;
		if(cross0 >= cross1) continue;
	} else {
		dual_plane = 1;
	}
	
	//////////////////////////////////////////////////////////////
	// We have at least four vertices, and at least one polygon (the plane's data itself).
	//////////////////////////////////////////////////////////////
		sub_vert_cnt += 4;
		sub_poly_cnt += 1;
	
	int min_z = JO_MIN(JO_MIN(subdivided_points[subdivided_polygons[0][0]][Z], subdivided_points[subdivided_polygons[0][1]][Z]),
			JO_MIN(subdivided_points[subdivided_polygons[0][2]][Z], subdivided_points[subdivided_polygons[0][3]][Z]));
	///////////////////////////////////////////
	// Just a side note:
	// Doing subdivision in screen-space **does not work**.
	// Well, technically it works, but warping is experienced. It looks pretty awful.
	///////////////////////////////////////////
		used_textures[0] = mesh->attbl[i].uv_id;

		subdivision_rules[0] = mesh->attbl[i].plane_information & 0x3;
		subdivision_rules[1] = (mesh->attbl[i].plane_information>>2) & 0x3;
		subdivision_rules[2] = (mesh->attbl[i].plane_information>>4) & 0x3;
		subdivision_rules[3] = 0;
		
		if(!subdivision_rules[0] || subdivision_rules[3] || (flags & GV_FLAG_NDIV) || min_z > z_rules[0])
		{
			//In case subdivision was not enabled, we need to copy from screen_transform_buffer to ssh2_vert_area.
			ssh2VertArea[0].pnt[X] = screen_transform_buffer[vids[0]].pnt[X];
			ssh2VertArea[0].pnt[Y] = screen_transform_buffer[vids[0]].pnt[Y];
			ssh2VertArea[0].pnt[Z] = screen_transform_buffer[vids[0]].pnt[Z];
			ssh2VertArea[1].pnt[X] = screen_transform_buffer[vids[1]].pnt[X];
			ssh2VertArea[1].pnt[Y] = screen_transform_buffer[vids[1]].pnt[Y];
			ssh2VertArea[1].pnt[Z] = screen_transform_buffer[vids[1]].pnt[Z];
			ssh2VertArea[2].pnt[X] = screen_transform_buffer[vids[2]].pnt[X];
			ssh2VertArea[2].pnt[Y] = screen_transform_buffer[vids[2]].pnt[Y];
			ssh2VertArea[2].pnt[Z] = screen_transform_buffer[vids[2]].pnt[Z];
			ssh2VertArea[3].pnt[X] = screen_transform_buffer[vids[3]].pnt[X];
			ssh2VertArea[3].pnt[Y] = screen_transform_buffer[vids[3]].pnt[Y];
			ssh2VertArea[3].pnt[Z] = screen_transform_buffer[vids[3]].pnt[Z];
			ssh2VertArea[0].clipFlag = screen_transform_buffer[vids[0]].clipFlag;
			ssh2VertArea[1].clipFlag = screen_transform_buffer[vids[1]].clipFlag;
			ssh2VertArea[2].clipFlag = screen_transform_buffer[vids[2]].clipFlag;
			ssh2VertArea[3].clipFlag = screen_transform_buffer[vids[3]].clipFlag;
			//Because I fucked up when transcribing the texture tables, we gotta -1.
			used_textures[0] -= 1;
			//Subdivision disabled end stub
		} else {
			///////////////////////////////////////////
			// The subdivision rules were pre-calculated by the converter tool.
			// In addition, the base texture (the uv_id) was also pre-calculated by the tool.
			///////////////////////////////////////////
			subdivide_plane(sub_vert_cnt, 0, 3, 0, mesh->attbl[i].uv_id);
			///////////////////////////////////////////
			//
			// Screenspace Transform of SUBDIVIDED Vertices
			// v = subdivided point index
			// testing_planes[i] = plane data index
			///////////////////////////////////////////
			// Pre-loop: Set near-plane clip for first vertex, then set the division unit to work
			///////////////////////////////////////////
			ssh2VertArea[0].pnt[Z] = (subdivided_points[0][Z] > SUBDIVISION_NEAR_PLANE) ? subdivided_points[0][Z] : SUBDIVISION_NEAR_PLANE;
			SetFixDiv(scrn_dist, ssh2VertArea[0].pnt[Z]);
			for(int v = 0; v < sub_vert_cnt; v++)
			{
				//Push to near-plane for NEXT vertex
				ssh2VertArea[v+1].pnt[Z] = (subdivided_points[v+1][Z] > SUBDIVISION_NEAR_PLANE) ? subdivided_points[v+1][Z] : SUBDIVISION_NEAR_PLANE;
				//Get 1/z for CURRENT vertex
				inverseZ = *DVDNTL;
				//Set division for NEXT vertex
				SetFixDiv(scrn_dist, ssh2VertArea[v+1].pnt[Z]);
				//Transform to screen-space
				ssh2VertArea[v].pnt[X] = fxm(subdivided_points[v][X], inverseZ)>>SCR_SCALE_X;
				ssh2VertArea[v].pnt[Y] = fxm(subdivided_points[v][Y], inverseZ)>>SCR_SCALE_Y;
				//Screen Clip Flags for on-off screen decimation
				ssh2VertArea[v].clipFlag = ((ssh2VertArea[v].pnt[X]) > TV_HALF_WIDTH) ? SCRN_CLIP_X : 0; 
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[X]) < -TV_HALF_WIDTH) ? SCRN_CLIP_NX : ssh2VertArea[v].clipFlag; 
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) > TV_HALF_HEIGHT) ? SCRN_CLIP_Y : ssh2VertArea[v].clipFlag;
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) < -TV_HALF_HEIGHT) ? SCRN_CLIP_NY : ssh2VertArea[v].clipFlag;
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Z]) <= SUBDIVISION_NEAR_PLANE) ? CLIP_Z : ssh2VertArea[v].clipFlag;
				transVerts[0]++;
				// clipping(&ssh2VertArea[v], USER_CLIP_INSIDE);
			}
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
/* 		for(int l = 0; l < MAX_DYNAMIC_LIGHTS; l++)
		{
			if(active_lights[l].pop == 1)
			{
				//This should be tabled for speed.
				//A 3D relative pos table should be used. 
				//Each entry is 10-bit precise.
				//The output for each entry is the dot product of the three entries divided into one (inverse).
				
				relative_light_pos[X] = (tx_light_pos[l][X] - ((subdivided_points[subdivided_polygons[j][0]][X]
														+ subdivided_points[subdivided_polygons[j][2]][X])>>1))>>12;
				relative_light_pos[Y] = (tx_light_pos[l][Y] - ((subdivided_points[subdivided_polygons[j][0]][Y]
														+ subdivided_points[subdivided_polygons[j][2]][Y])>>1))>>12;
				relative_light_pos[Z] = (tx_light_pos[l][Z] - ((subdivided_points[subdivided_polygons[j][0]][Z]
														+ subdivided_points[subdivided_polygons[j][2]][Z])>>1))>>12;
				inverted_proxima = ((relative_light_pos[X] * relative_light_pos[X]) +
									(relative_light_pos[Y] * relative_light_pos[Y]) +
									(relative_light_pos[Z] * relative_light_pos[Z]))>>8;
				inverted_proxima = (inverted_proxima < 65536) ? division_table[inverted_proxima] : 0;
						
				luma += inverted_proxima * (int)active_lights[l].bright;
			}
		//	if(luma > 0) break; // Early exit
		}

		luma = (luma < 0) ? 0 : luma; 
 */		luma += fxdot(mesh->nmtbl[i], active_lights[0].ambient_light);
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


/*

//Saved for posterity, to demonstrate how the subdvision rules are determined.
void	TEMP_process_mesh_for_subdivision_rules(GVPLY * mesh)
{
	
	for(unsigned int i = 0; i < mesh->nbPolygon; i++)
	{
		int * pl_pt0 = mesh->pntbl[mesh->pltbl[i].vertices[0]];
		int * pl_pt1 = mesh->pntbl[mesh->pltbl[i].vertices[1]];
		int * pl_pt2 = mesh->pntbl[mesh->pltbl[i].vertices[2]];
		int * pl_pt3 = mesh->pntbl[mesh->pltbl[i].vertices[3]];
								
		int len01 = unfix_length(pl_pt0, pl_pt1);
		int len12 = unfix_length(pl_pt1, pl_pt2);
		int len23 = unfix_length(pl_pt2, pl_pt3);
		int len30 = unfix_length(pl_pt3, pl_pt0);
		int perimeter = len01 + len12 + len23 + len30;

		int len_w = JO_MAX(len01, len23);//(len01 + len23)>>1; 
		int len_h = JO_MAX(len12, len30);//(len12 + len30)>>1;
	
		subdivision_rules[0] = 0;
		subdivision_rules[1] = 0;
		subdivision_rules[2] = 0;
		subdivision_rules[3] = (perimeter > 1200) ? 1 : 0;
	
			if(len_w >= SUBDIVISION_SCALE)
			{
				subdivision_rules[0] = SUBDIVIDE_X;
			}
			if(len_w >= SUBDIVISION_SCALE<<1)
			{
				subdivision_rules[1] = SUBDIVIDE_X;
			}
			if(len_w >= SUBDIVISION_SCALE<<2)
			{
				subdivision_rules[2] = SUBDIVIDE_X;
			}
			
			if(len_h >= SUBDIVISION_SCALE)
			{
				subdivision_rules[0] = (subdivision_rules[0] == SUBDIVIDE_X) ? SUBDIVIDE_XY : SUBDIVIDE_Y;
			}
			if(len_h >= SUBDIVISION_SCALE<<1)
			{
				subdivision_rules[1] = (subdivision_rules[1] == SUBDIVIDE_X) ? SUBDIVIDE_XY : SUBDIVIDE_Y;
			}
			if(len_h >= SUBDIVISION_SCALE<<2)
			{
				subdivision_rules[2] = (subdivision_rules[2] == SUBDIVIDE_X) ? SUBDIVIDE_XY : SUBDIVIDE_Y;
			}
			unsigned char subrules = subdivision_rules[0];
			subrules |= subdivision_rules[1]<<2;
			subrules |= subdivision_rules[2]<<4;
			subrules |= subdivision_rules[3]<<6;
			mesh->attbl[i].plane_information = subrules;
	}	
	
}
		
*/




