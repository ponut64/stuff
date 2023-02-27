//subrender.c
//This file compiled separately

#include <SL_DEF.H>
#include "def.h"
#include "mymath.h"
#include "render.h"


	/*
	
	TEXTURE ID MATRIX:
	1	- full size
	2	- full y, 1/2 x
	3	- full y, 1/4 x
	4	- full y, 1/8 x
	5	- 1/2 y, full x
	6	- 1/2 y, 1/2 x
	7	- 1/2 y, 1/4 x
	8	- 1/2 y, 1/8 x
	9	- 1/4 y, full x
	10	- 1/4 y, 1/2 x
	11	- 1/4 y, 1/4 x
	12	- 1/4 y, 1/8 x
	13	- 1/8 y, full x
	14	- 1/8 y, 1/2 x
	15	- 1/8 y, 1/4 x
	16	- 1/8 y, 1/8 x

	*/
	#define SUBDIVIDE_W		(1)
	#define SUBDIVIDE_H		(4)
	#define SUBDIVIDE_HV	(5)

	#define TEXTS_GENERATED_PER_TEXTURE_LOADED (16)
	#define SUBDIVISION_NEAR_PLANE (15<<16)

		POINT	subdivided_points[512];
		short	subdivided_polygons[512][4]; //4 Vertex IDs of the subdivided_points
		short	used_textures[512];
		short	sub_poly_cnt = 0;
		short	sub_vert_cnt = 0;
		char	subdivision_rules[4]	= {'N', 'N', 'N', 'N'};
		short	texture_rules[4]		= {16, 16, 16, 16};
		int		z_rules[4]				= {200<<16, 150<<16, 100<<16, 50<<16};
void	subdivide_plane(short start_point, short overwritten_polygon, short num_divisions, short total_divisions)
{

	//"Load" the original points (code shortening operation)
	FIXED * ptv[4];
	static int max_z;
	static char new_rule;
	for(int u = 0; u < 4; u++)
	{
		ptv[u] = &subdivided_points[subdivided_polygons[overwritten_polygon][u]][X];
	}
	
	//if(ptv[0][Z] < 0 && ptv[1][Z] < 0 && ptv[2][Z] < 0 && ptv[3][Z] < 0) return;
	new_rule = subdivision_rules[total_divisions];

	used_textures[overwritten_polygon] = texture_rules[total_divisions];
	if(num_divisions == 0)
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
	
	if(new_rule == SUBDIVIDE_HV) 
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
	
			D				C

	3D			2D | 3C			2C
	*/
	// Initial Conditions
	subdivided_polygons[poly_a][0] = subdivided_polygons[overwritten_polygon][0];
	subdivided_polygons[poly_b][1] = subdivided_polygons[overwritten_polygon][1];
	subdivided_polygons[poly_c][2] = subdivided_polygons[overwritten_polygon][2];
	subdivided_polygons[poly_d][3] = subdivided_polygons[overwritten_polygon][3];

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
	subdivided_polygons[poly_c][0] = tgt_pnt;
	subdivided_polygons[poly_d][1] = tgt_pnt;

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
	subdivided_polygons[poly_c][1] = tgt_pnt;
	tgt_pnt++;
	// 3 -> 2
	subdivided_points[tgt_pnt][X] = (ptv[2][X] + ptv[3][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[2][Y] + ptv[3][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[2][Z] + ptv[3][Z])>>1;
	
	subdivided_polygons[poly_c][3] = tgt_pnt;
	subdivided_polygons[poly_d][2] = tgt_pnt;
	tgt_pnt++;
	// 3 -> 0
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[3][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[3][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[3][Z])>>1;
	
	subdivided_polygons[poly_a][3] = tgt_pnt;
	subdivided_polygons[poly_d][0] = tgt_pnt;
	tgt_pnt++;
	sub_vert_cnt = tgt_pnt;
	sub_poly_cnt += 3; //Only add 3, as there was already 1 polygon. It was split into four.
	
		if(num_divisions > 0)
		{
		///////////////////////////////////////////
		//	Recursively subdivide the polygon.
		//	Check the maximum Z of every new polygon.
		// 	If the maximum Z is less than zero, it's not on screen. No point in subdividing it any further.
		///////////////////////////////////////////
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][0]][Z], subdivided_points[subdivided_polygons[poly_a][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][2]][Z], subdivided_points[subdivided_polygons[poly_a][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_a, num_divisions-1, total_divisions+1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][0]][Z], subdivided_points[subdivided_polygons[poly_b][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][2]][Z], subdivided_points[subdivided_polygons[poly_b][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_b, num_divisions-1, total_divisions+1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_c][0]][Z], subdivided_points[subdivided_polygons[poly_c][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_c][2]][Z], subdivided_points[subdivided_polygons[poly_c][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_c, num_divisions-1, total_divisions+1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_d][0]][Z], subdivided_points[subdivided_polygons[poly_d][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_d][2]][Z], subdivided_points[subdivided_polygons[poly_d][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_d, num_divisions-1, total_divisions+1);
		}
	
	} else if(new_rule == SUBDIVIDE_H) 
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
	
		if(num_divisions > 0)
		{
		///////////////////////////////////////////
		//	Recursively subdivide the polygon.
		//	Check the maximum Z of every new polygon.
		// 	If the maximum Z is less than zero, it's not on screen. No point in subdividing it any further.
		///////////////////////////////////////////
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][0]][Z], subdivided_points[subdivided_polygons[poly_a][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][2]][Z], subdivided_points[subdivided_polygons[poly_a][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_a, num_divisions-1, total_divisions+1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][0]][Z], subdivided_points[subdivided_polygons[poly_b][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][2]][Z], subdivided_points[subdivided_polygons[poly_b][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_b, num_divisions-1, total_divisions+1);
		}
		
	} else if(new_rule == SUBDIVIDE_W)
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
	
		if(num_divisions > 0)
		{
		///////////////////////////////////////////
		//	Recursively subdivide the polygon.
		//	Check the maximum Z of every new polygon.
		// 	If the maximum Z is less than zero, it's not on screen. No point in subdividing it any further.
		///////////////////////////////////////////
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][0]][Z], subdivided_points[subdivided_polygons[poly_a][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][2]][Z], subdivided_points[subdivided_polygons[poly_a][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_a, num_divisions-1, total_divisions+1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][0]][Z], subdivided_points[subdivided_polygons[poly_b][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][2]][Z], subdivided_points[subdivided_polygons[poly_b][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_b, num_divisions-1, total_divisions+1);
		}
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
		
		FIXED * mesh_position = &ent->prematrix[9];

		POINT	pl_pts[4];

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
	With polygon subdivision based on the Z (depth)

	Load up a plane.
	Transform its vertices by the matrix, but don't explicitly screenspace transform it.
	These vertices will be "screen-centered", so now we can subdivide the plane.
	Check the span of the plane to see if it is large.
	If it large in one of two particular ways, subdivide it by its longitude or its latitude (make two from one).
	If it is large in both ways, subdivide it both ways (make four from one).
	At this point, subdivision will occur recursively up to a limit arbitrated from the plane's Z,
	will cease subdivision on polygons with a high Z, and continue subdivision on polygons with a low Z.
	
	All but the lowest subdivision of a polygon will receive a combined texture.
	The texture is either a X*2, Y*2, or an X*2 & Y*2 combination -- the same as the subdivision pattern of the polygon.
	**/

	int max_z = 0;
	int min_z = 0;
	
	int max_subdivisions = 0;
	int specific_texture = 0;
	
	////////////////////////////////////////////////////
	// Transform each light source position by the matrix parameters.
	////////////////////////////////////////////////////
	POINT relative_light_pos = {0, 0, 0};
	static POINT tx_light_pos[MAX_DYNAMIC_LIGHTS];
	int inverted_proxima;
	
	for(int l = 0; l < MAX_DYNAMIC_LIGHTS; l++)
	{
		if(active_lights[l].pop == 1)
		{
			relative_light_pos[X] = -active_lights[l].pos[X] - mesh_position[X];
			relative_light_pos[Y] = -active_lights[l].pos[Y] - mesh_position[Y];
			relative_light_pos[Z] = -active_lights[l].pos[Z] - mesh_position[Z];
			tx_light_pos[l][X] = trans_pt_by_component(relative_light_pos, m0x);
			tx_light_pos[l][Y] = trans_pt_by_component(relative_light_pos, m1y);
			tx_light_pos[l][Z] = trans_pt_by_component(relative_light_pos, m2z);
		}
	}

for(unsigned int i = 0; i < mesh->nbPolygon; i++)
{
		sub_vert_cnt = 0;
		sub_poly_cnt = 0;
	for(int u = 0; u < 4; u++)
	{
	//////////////////////////////////////////////////////////////
	// Load the points
	//////////////////////////////////////////////////////////////
		pl_pts[u][X] = (mesh->pntbl[mesh->pltbl[i].Vertices[u]][X]);
		pl_pts[u][Y] = (mesh->pntbl[mesh->pltbl[i].Vertices[u]][Y]);
		pl_pts[u][Z] = (mesh->pntbl[mesh->pltbl[i].Vertices[u]][Z]);
	//////////////////////////////////////////////////////////////
	// Matrix transformation of the plane's points
	// Note: Does not yet transform to screenspace, clip by screen or portal, or push out to near plane.
	//////////////////////////////////////////////////////////////
        subdivided_points[u][Z] = trans_pt_by_component(pl_pts[u], m2z);
        subdivided_points[u][Y] = trans_pt_by_component(pl_pts[u], m1y);
        subdivided_points[u][X] = trans_pt_by_component(pl_pts[u], m0x);

		subdivided_polygons[0][u] = u;
	//////////////////////////////////////////////////////////////
	// Early screenspace transform to throw out off-screen planes
	//////////////////////////////////////////////////////////////
		//Push to near-plane
		ssh2VertArea[u].pnt[Z] = (subdivided_points[u][Z] > SUBDIVISION_NEAR_PLANE) ? subdivided_points[u][Z] : SUBDIVISION_NEAR_PLANE;
		//Get 1/z
		inverseZ = fxdiv(scrn_dist, ssh2VertArea[u].pnt[Z]);
        //Transform to screen-space
        ssh2VertArea[u].pnt[X] = fxm(subdivided_points[u][X], inverseZ)>>SCR_SCALE_X;
        ssh2VertArea[u].pnt[Y] = fxm(subdivided_points[u][Y], inverseZ)>>SCR_SCALE_Y;
        //Screen Clip Flags for on-off screen decimation
		ssh2VertArea[u].clipFlag = ((ssh2VertArea[u].pnt[X]) > TV_HALF_WIDTH) ? SCRN_CLIP_X : 0; 
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[X]) < -TV_HALF_WIDTH) ? SCRN_CLIP_NX : ssh2VertArea[u].clipFlag; 
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[Y]) > TV_HALF_HEIGHT) ? SCRN_CLIP_Y : ssh2VertArea[u].clipFlag;
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[Y]) < -TV_HALF_HEIGHT) ? SCRN_CLIP_NY : ssh2VertArea[u].clipFlag;
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[Z]) <= SUBDIVISION_NEAR_PLANE) ? CLIP_Z : ssh2VertArea[u].clipFlag;
		// clipping(&ssh2VertArea[u], USER_CLIP_INSIDE);
	}
		 if(ssh2VertArea[0].clipFlag
		 & ssh2VertArea[1].clipFlag
		 & ssh2VertArea[2].clipFlag
		 & ssh2VertArea[3].clipFlag) continue;
		 
	flags = mesh->attbl[i].render_data_flags;
	zDepthTgt = GET_SORT_DATA(flags);
		 
	//////////////////////////////////////////////////////////////
	// Screen-space back face culling segment. Will also avoid if the plane is flagged as dual-plane.
	//////////////////////////////////////////////////////////////
	if(flags & GV_FLAG_SINGLE)
	{
		 int cross0 = (ssh2VertArea[1].pnt[X] - ssh2VertArea[3].pnt[X])
							* (ssh2VertArea[0].pnt[Y] - ssh2VertArea[2].pnt[Y]);
		 int cross1 = (ssh2VertArea[1].pnt[Y] - ssh2VertArea[3].pnt[Y])
							* (ssh2VertArea[0].pnt[X] - ssh2VertArea[2].pnt[X]);
		if(cross0 >= cross1) continue;
	}
	//
	//////////////////////////////////////////////////////////////
	// We have at least four vertices, and at least one polygon (the plane's data itself).
	//////////////////////////////////////////////////////////////
		sub_vert_cnt += 4;
		sub_poly_cnt += 1;
	///////////////////////////////////////////
	//	Check the maximum Z of every new polygon.
	// 	This is the first polygon. So, if its maximum Z is too low, just discard it.
	///////////////////////////////////////////
	max_z = JO_MAX(JO_MAX(subdivided_points[subdivided_polygons[0][0]][Z], subdivided_points[subdivided_polygons[0][1]][Z]),
			JO_MAX(subdivided_points[subdivided_polygons[0][2]][Z], subdivided_points[subdivided_polygons[0][3]][Z]));
	if(max_z <= SUBDIVISION_NEAR_PLANE) continue;
	min_z = JO_MIN(JO_MIN(subdivided_points[subdivided_polygons[0][0]][Z], subdivided_points[subdivided_polygons[0][1]][Z]),
			JO_MIN(subdivided_points[subdivided_polygons[0][2]][Z], subdivided_points[subdivided_polygons[0][3]][Z]));
	///////////////////////////////////////////
	// Just a side note:
	// Doing subdivision in screen-space **does not work**.
	// Well, technically it works, but affine warping is experienced. It looks pretty awful.
	///////////////////////////////////////////
	
	if(flags & GV_FLAG_NDIV)
	{ 
		max_subdivisions = 0;
		used_textures[0] = 0;
	} else {
	//////////////////////////////////////////////////////////////
	// Check: Find the polygon's scale and thus subdivision scale.
	// We find the true perimeter of the polygon.
	//////////////////////////////////////////////////////////////
		int len01 = unfix_length(pl_pts[0], pl_pts[1]);
		int len12 = unfix_length(pl_pts[1], pl_pts[2]);
		int len23 = unfix_length(pl_pts[2], pl_pts[3]);
		int len30 = unfix_length(pl_pts[3], pl_pts[0]);
		int perimeter = len01 + len12 + len23 + len30;

		int len_w = JO_MAX(len01, len23);//(len01 + len23)>>1; 
		int len_h = JO_MAX(len12, len30);//(len12 + len30)>>1;

		// nbg_sprintf(1, 6, "len12(%i)", len12);
		// nbg_sprintf(1, 7, "sx(%i)", unfix12[X]);
		// nbg_sprintf(1, 8, "sy(%i)", unfix12[Y]);
		
		if(perimeter <= 100 || perimeter >= 1200)
		{
			//In this case the polygon is too small or too large.
			//Large polygons will be excepted by making it look obviously wrong.
			used_textures[0] = 0;
			max_subdivisions = 0;
		} else {
			///////////////////////////////////////////
			// Resolve the maximum # of subdivisions.
			///////////////////////////////////////////
			max_subdivisions = 0;
			// The idea is that the resulting polygons should be more than 25x25 but less than 50x50.
			subdivision_rules[0] = 0;
			subdivision_rules[1] = 0;
			subdivision_rules[2] = 0;
			subdivision_rules[3] = 0;
			// nbg_sprintf(1, 7, "lw(%i)", len_w);
			// nbg_sprintf(1, 8, "lh(%i)", len_h);
			if(len_w >= 50)
			{
				subdivision_rules[0] = SUBDIVIDE_W;
				max_subdivisions += 1;
			}
			if(len_w >= 100)
			{
				subdivision_rules[1] = SUBDIVIDE_W;
				max_subdivisions += 1;
			}
			if(len_w >= 200)
			{
				subdivision_rules[2] = SUBDIVIDE_W;
				max_subdivisions += 1;
			}
			
			if(len_h >= 50)
			{
				subdivision_rules[0] = (subdivision_rules[0] == SUBDIVIDE_W) ? SUBDIVIDE_HV : SUBDIVIDE_H;
				max_subdivisions += (max_subdivisions < 3) ? 1 : 0;
			}
			if(len_h >= 100)
			{
				subdivision_rules[1] = (subdivision_rules[1] == SUBDIVIDE_W) ? SUBDIVIDE_HV : SUBDIVIDE_H;
				max_subdivisions += (max_subdivisions < 3) ? 1 : 0;
			}
			if(len_w >= 200)
			{
				subdivision_rules[2] = (subdivision_rules[2] == SUBDIVIDE_W) ? SUBDIVIDE_HV : SUBDIVIDE_H;
				max_subdivisions += (max_subdivisions < 3) ? 1 : 0;
			}
			
			texture_rules[0] = 16 - (subdivision_rules[0] + subdivision_rules[1] + subdivision_rules[2]);
			texture_rules[1] = texture_rules[0] + subdivision_rules[0];
			texture_rules[2] = texture_rules[1] + subdivision_rules[1];
			
			// texture_rules[0] = 16;
			// texture_rules[1] = 16;
			// texture_rules[2] = 16;
			// texture_rules[3] = 16;
			subdivide_plane(sub_vert_cnt, 0, max_subdivisions, 0);
			///////////////////////////////////////////
			//
			// Screenspace Transform of Vertices
			// v = subdivided point index
			// testing_planes[i] = plane data index
			//
			///////////////////////////////////////////
			for(int v = 0; v < sub_vert_cnt; v++)
			{
				//Push to near-plane
				ssh2VertArea[v].pnt[Z] = (subdivided_points[v][Z] > SUBDIVISION_NEAR_PLANE) ? subdivided_points[v][Z] : SUBDIVISION_NEAR_PLANE;
				//Get 1/z
				inverseZ = fxdiv(scrn_dist, ssh2VertArea[v].pnt[Z]);
				//Transform to screen-space
				ssh2VertArea[v].pnt[X] = fxm(subdivided_points[v][X], inverseZ)>>SCR_SCALE_X;
				ssh2VertArea[v].pnt[Y] = fxm(subdivided_points[v][Y], inverseZ)>>SCR_SCALE_Y;
				//Screen Clip Flags for on-off screen decimation
				ssh2VertArea[v].clipFlag = ((ssh2VertArea[v].pnt[X]) > TV_HALF_WIDTH) ? SCRN_CLIP_X : 0; 
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[X]) < -TV_HALF_WIDTH) ? SCRN_CLIP_NX : ssh2VertArea[v].clipFlag; 
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) > TV_HALF_HEIGHT) ? SCRN_CLIP_Y : ssh2VertArea[v].clipFlag;
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) < -TV_HALF_HEIGHT) ? SCRN_CLIP_NY : ssh2VertArea[v].clipFlag;
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Z]) <= SUBDIVISION_NEAR_PLANE) ? CLIP_Z : ssh2VertArea[v].clipFlag;
				// clipping(&ssh2VertArea[v], USER_CLIP_INSIDE);
			}
		//Subdivision activation end stub
		}
	//Subdivision enabled end stub
	}
	///////////////////////////////////////////
	//
	// Z-sort Insertion & Command Arrangement of Polygons
	// j = subdivided polygon index
	//
	///////////////////////////////////////////
	if(ssh2SentPolys[0] + sub_poly_cnt > MAX_SSH2_SENT_POLYS) return;
	flags = (((flags & GV_FLAG_MESH)>>1) | ((flags & GV_FLAG_DARK)<<4))<<8;
	vertex_t * ptv[5] = {0, 0, 0, 0, 0};
	for(int j = 0; j < sub_poly_cnt; j++)
	{
		
		ptv[0] = &ssh2VertArea[subdivided_polygons[j][0]];
		ptv[1] = &ssh2VertArea[subdivided_polygons[j][1]];
		ptv[2] = &ssh2VertArea[subdivided_polygons[j][2]];
		ptv[3] = &ssh2VertArea[subdivided_polygons[j][3]];
		
		 int offScrn = (ptv[0]->clipFlag & ptv[1]->clipFlag & ptv[2]->clipFlag & ptv[3]->clipFlag);
		///////////////////////////////////////////
		// Z-Sorting Stuff	
		// Uses weighted max
		///////////////////////////////////////////
		//	if(JO_ABS(mesh->pltbl[i].norm[Y]) < 16384)
		//	{
		// zDepthTgt = (ptv[0]->pnt[Z] + ptv[2]->pnt[Z])>>1;
		//	} else {
		 zDepthTgt = (JO_MAX(
		JO_MAX(ptv[0]->pnt[Z], ptv[2]->pnt[Z]),
		JO_MAX(ptv[1]->pnt[Z], ptv[3]->pnt[Z])) + 
		((ptv[0]->pnt[Z] + ptv[1]->pnt[Z] + ptv[2]->pnt[Z] + ptv[3]->pnt[Z])>>2))>>1;
		//	}
			if(offScrn || zDepthTgt < NEAR_PLANE_DISTANCE || zDepthTgt > FAR_PLANE_DISTANCE) continue;
		///////////////////////////////////////////
		// Use a combined texture, if the subdivision system stated one should be used.
		// Otherwise, use the base texture.
		///////////////////////////////////////////
			if(used_textures[j] != 0)
			{
				specific_texture = ((mesh->attbl[i].texno - ent->base_texture) * TEXTS_GENERATED_PER_TEXTURE_LOADED)
				+ (ent->numTexture + ent->base_texture) + used_textures[j];
			} else {
				specific_texture = mesh->attbl[i].texno;
			}
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
		for(int l = 0; l < MAX_DYNAMIC_LIGHTS; l++)
		{
			if(active_lights[l].pop == 1)
			{
				
				/*
				This should be tabled for speed.
				A 3D relative pos table should be used. 
				Each entry is 10-bit precise.
				The output for each entry is the dot product of the three entries divided into one (inverse).
				*/
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
				
				// relative_light_pos[X] = (tx_light_pos[l][X] - ((subdivided_points[subdivided_polygons[j][0]][X]
														// + subdivided_points[subdivided_polygons[j][2]][X])>>1));
				// relative_light_pos[Y] = (tx_light_pos[l][Y] - ((subdivided_points[subdivided_polygons[j][0]][Y]
														// + subdivided_points[subdivided_polygons[j][2]][Y])>>1));
				// relative_light_pos[Z] = (tx_light_pos[l][Z] - ((subdivided_points[subdivided_polygons[j][0]][Z]
														// + subdivided_points[subdivided_polygons[j][2]][Z])>>1));
				// inverted_proxima = fxdot(relative_light_pos, relative_light_pos)>>16;
				
				// inverted_proxima = (inverted_proxima < 65536) ? division_table[inverted_proxima] : 0;
						
				luma += inverted_proxima * (int)active_lights[l].bright;
			}
		//	if(luma > 0) break; // Early exit
		}
		luma = (luma < 0) ? 0 : luma; 
		luma += fxdot(mesh->pltbl[i].norm, active_lights[0].ambient_light) + active_lights[0].min_bright; 
		determine_colorbank(&colorBank, &luma);
/* 		//Use transformed normal as shade determinant
		colorBank = (luma >= 98294) ? 0 : 1;
		colorBank = (luma < 49152) ? 2 : colorBank;
		colorBank = (luma < 32768) ? 3 : colorBank; 
		colorBank = (luma < 16384) ? 515 : colorBank; //Make really dark? use MSB shadow
 */			

      ssh2SetCommand(ptv[0]->pnt, ptv[1]->pnt,
					ptv[2]->pnt, ptv[3]->pnt,
		VDP1_BASE_CMDCTRL | flip, (VDP1_BASE_PMODE | flags) | pclp, //Reads flip value, mesh enable, and msb bit
		pcoTexDefs[specific_texture].SRCA, colorBank<<6, pcoTexDefs[specific_texture].SIZE, 0, zDepthTgt);
	}
    transVerts[0] += sub_vert_cnt;
	transPolys[0] += sub_poly_cnt;
}
	//////////////////////////////////////////////////////////////
	// Planar polygon subdivision rendering end stub
	//////////////////////////////////////////////////////////////

}








