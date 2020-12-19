//object_col.c
//contains per polygon collision work
#include "object_col.h"

#define MATH_TOLERANCE (16384)

int prntidx = 0;

//////////////////////////////////
// Line-to-plane projection function
// Line: p0->p1
// point_on_plane : a point on the plane
// unitNormal : the unit vector normal of the plane
// output : the point at which the line intersects the plane
// return value : whether or not the output point is between p0 and p1
//////////////////////////////////
bool	project_segment_to_plane_and_test(FIXED p0[XYZ], FIXED p1[XYZ], FIXED point_on_plane[XYZ], FIXED unitNormal[XYZ], FIXED output[XYZ])
{

	FIXED line_scalar = 0;
	FIXED vector_of_line[XYZ] = {0, 0, 0};
	FIXED vector_to_plane[XYZ] = {0, 0, 0};
	
	vector_of_line[X] = p0[X] - p1[X];
	vector_of_line[Y] = p0[Y] - p1[Y];
	vector_of_line[Z] = p0[Z] - p1[Z];

	vector_to_plane[X] = (point_on_plane[X] - p0[X]);
	vector_to_plane[Y] = (point_on_plane[Y] - p0[Y]);
	vector_to_plane[Z] = (point_on_plane[Z] - p0[Z]);
	

	line_scalar = fxdiv(fxdot(vector_to_plane, unitNormal), fxdot(vector_of_line, unitNormal));
	if(line_scalar > (1000<<16) || line_scalar < -(1000<<16)){
		return false;
	}

	output[X] = (p0[X] + fxm(vector_of_line[X], line_scalar));
	output[Y] = (p0[Y] + fxm(vector_of_line[Y], line_scalar));
	output[Z] = (p0[Z] + fxm(vector_of_line[Z], line_scalar));

	return isPointonSegment(output, p0, p1);
}




bool		edge_wind_test(POINT plane_p0, POINT plane_p1, POINT test_pt, int discard)
{
	
	int left = 0;
	int right = 0;
	int side = 0;
	
	/*
	
	Edge Winding test
	where p is the testing point and plane_p0 and plane_p1 are points of the polygon
	We ought to flatten these somehow.
	
	(y - y0) * (x1 - x0) - (x - x0) * (y1 - y0)
	left = (p[Y] - plane_p0[Y]) * (plane_p1[X] - plane_p0[X])
	
	right = (p[X] - plane_p0[X]) * (plane_p1[Y] - plane_p0[Y])
	
	side = left - right
	
	(this is a cross product)
	if side is > 0, the point is "left" of the winding. If it is < 0, it is "right" of the winding.
	Since all polygons have a known winding, this should work. Just make sure we're inside the winding of all edges.
	In other words - you have a good early-exit condition. If you are outside of one winding, you can exit the test.
	The caveat to the winding solution is that the winding will change depending on the facing of the normal.
	
	This is a 2D solution, but - it'll probably work if I drop the major axis of the normal in all the calculations.
	
	*/
	
	//Triangle exception handling
	// if(plane_p0[X] == plane_p1[X] &&
	// plane_p0[Y] == plane_p1[Y] &&
	// plane_p0[Z] == plane_p1[Z])
	// {
		// return true;
	// }
	
	/////////////////////////////////////////////////////////////////////////////////////////////
	//Using integer math. The precision of fixed point is not required, and this prevents overflows.
	/////////////////////////////////////////////////////////////////////////////////////////////
	if(discard == N_Xp)
	{
		// left = fxm((test_pt[Y] - plane_p0[Y]), (plane_p1[Z] - plane_p0[Z]));
		// right = fxm((test_pt[Z] - plane_p0[Z]), (plane_p1[Y] - plane_p0[Y]));
		left = ((test_pt[Y] - plane_p0[Y])>>16) * ((plane_p1[Z] - plane_p0[Z])>>16);
		right = ((test_pt[Z] - plane_p0[Z])>>16) * ((plane_p1[Y] - plane_p0[Y])>>16);
		//slPrint("Discard X+", slLocate(2, 5));
	} else if(discard == N_Zp)
	{
		// left = fxm((test_pt[X] - plane_p0[X]), (plane_p1[Y] - plane_p0[Y]));
		// right = fxm((test_pt[Y] - plane_p0[Y]), (plane_p1[X] - plane_p0[X]));
		left = ((test_pt[X] - plane_p0[X])>>16) * ((plane_p1[Y] - plane_p0[Y])>>16);
		right = ((test_pt[Y] - plane_p0[Y])>>16) * ((plane_p1[X] - plane_p0[X])>>16);
		//slPrint("Discard Z+", slLocate(2, 5));
	} else if(discard == N_Yn)
	{
		// left = fxm((test_pt[X] - plane_p0[X]), (plane_p1[Z] - plane_p0[Z]));
		// right = fxm((test_pt[Z] - plane_p0[Z]), (plane_p1[X] - plane_p0[X]));
		left = ((test_pt[X] - plane_p0[X])>>16) * ((plane_p1[Z] - plane_p0[Z])>>16);
		right = ((test_pt[Z] - plane_p0[Z])>>16) * ((plane_p1[X] - plane_p0[X])>>16);
		//slPrint("Discard Y+", slLocate(2, 5));
	} else if(discard == N_Xn)
	{
		// right = fxm((test_pt[Y] - plane_p0[Y]), (plane_p1[Z] - plane_p0[Z]));
		// left = fxm((test_pt[Z] - plane_p0[Z]), (plane_p1[Y] - plane_p0[Y]));
		right = ((test_pt[Y] - plane_p0[Y])>>16) * ((plane_p1[Z] - plane_p0[Z])>>16);
		left = ((test_pt[Z] - plane_p0[Z])>>16) * ((plane_p1[Y] - plane_p0[Y])>>16);
		//slPrint("Discard X-", slLocate(2, 5));
	} else if(discard == N_Zn)
	{
		// right = fxm((test_pt[X] - plane_p0[X]), (plane_p1[Y] - plane_p0[Y]));
		// left = fxm((test_pt[Y] - plane_p0[Y]), (plane_p1[X] - plane_p0[X]));
		right = ((test_pt[X] - plane_p0[X])>>16) * ((plane_p1[Y] - plane_p0[Y])>>16);
		left = ((test_pt[Y] - plane_p0[Y])>>16) * ((plane_p1[X] - plane_p0[X])>>16);
		//slPrint("Discard Z-", slLocate(2, 5));
	} else if(discard == N_Yp)
	{
		// right = fxm((test_pt[X] - plane_p0[X]), (plane_p1[Z] - plane_p0[Z]));
		// left = fxm((test_pt[Z] - plane_p0[Z]), (plane_p1[X] - plane_p0[X])); 
		right = ((test_pt[X] - plane_p0[X])>>16) * ((plane_p1[Z] - plane_p0[Z])>>16);
		left = ((test_pt[Z] - plane_p0[Z])>>16) * ((plane_p1[X] - plane_p0[X])>>16);
		//slPrint("Discard Y-", slLocate(2, 5));
	}
	// slPrint("Left:", slLocate(2, 7 + (prntidx * 2)));
	// slPrintFX(left, slLocate(2, 8 + (prntidx * 2)));
	
	// slPrint("Right:", slLocate(18, 7 + (prntidx * 2)));
	// slPrintFX(right, slLocate(18, 8 + (prntidx * 2)));
	//prntidx++;
	side = left - right;
	if(side >= 0)
	{
		return true;
	} else {
		return false;
	}
}


void	per_poly_collide(PDATA * mesh, POINT mesh_position, _boundBox * mover)
{

short testing_planes[128];
short total_planes = 0;
POINT discard_vector = {0, 0, 0};
bool hitY = false;
bool hitXZ = false;

prntidx = 0;

	/**
	This test can only be performed on un-rotated meshes.
	(DO NOT rotate the mesh!) (Rotate to desired orientation before exporting!)
	Summary:
	First, use a normal-based discard to throw out polygons/planes that face away from where the player is.
	That builds a list of which polygons/planes to test for collision.
	Then, draw a point from every major axis of the player's bounding box to every polygon.
	If no point is inside the box, move on to the next polygon.
	If a point is inside the box, also test that point against the polygon with a 2D edge winding test.
	If it is inside the polygon and inside the player's bounding box, collision has occurred.
	**/
	
	//////////////////////////////////////////////////////////////
	// Normal-based discard of planes
	// If the plane's normal is facing away from where the player is,
	// it will not be put into the pile of planes to collision test.
	// PDATA vector space is inverted, so we negate them
	//////////////////////////////////////////////////////////////
	for(unsigned int dst_poly = 0; dst_poly < mesh->nbPolygon; dst_poly++)
	{
				discard_vector[X] = -(mesh->pntbl[mesh->pltbl[dst_poly].Vertices[0]][X]) - mover->pos[X] - mesh_position[X];
				discard_vector[Y] = -(mesh->pntbl[mesh->pltbl[dst_poly].Vertices[0]][Y]) - mover->pos[Y] - mesh_position[Y];
				discard_vector[Z] = -(mesh->pntbl[mesh->pltbl[dst_poly].Vertices[0]][Z]) - mover->pos[Z] - mesh_position[Z];

				int normal_discard = fxdot(discard_vector, mesh->pltbl[dst_poly].norm);
				
	// slPrint("Discard vector:", slLocate(1, 9));
	// slPrintFX(discard_vector[X], slLocate(2, 10));
	// slPrintFX(discard_vector[Y], slLocate(2, 11));
	// slPrintFX(discard_vector[Z], slLocate(2, 12));
	// slPrint("Dot product:", slLocate(1, 13));
	// slPrintFX(normal_discard, slLocate(2, 14));
				
				if(normal_discard >= -(5<<16) && total_planes < 128){
					testing_planes[total_planes] = dst_poly;
					total_planes++;
				}
	}
	
	//jo_printf(1, 15, "Total planes: (%i)", total_planes);
	
	//////////////////////////////////////////////////////////////
	// Add the position of the mover's box centre-faces to the mover's world position
	// "Get world-space point position"
	//////////////////////////////////////////////////////////////
_lineTable moverCFs = {
	.xp0[X] = mover->Xplus[X] + mover->pos[X],
	.xp0[Y] = mover->Xplus[Y] + mover->pos[Y],
	.xp0[Z] = mover->Xplus[Z] + mover->pos[Z],
	.xp1[X] = mover->Xneg[X] + mover->pos[X],
	.xp1[Y] = mover->Xneg[Y] + mover->pos[Y],
	.xp1[Z] = mover->Xneg[Z] + mover->pos[Z],
	.yp0[X] = mover->Yplus[X] + mover->pos[X],
	.yp0[Y] = mover->Yplus[Y] + mover->pos[Y],
	.yp0[Z] = mover->Yplus[Z] + mover->pos[Z],
	.yp1[X] = mover->Yneg[X] + mover->pos[X],
	.yp1[Y] = mover->Yneg[Y] + mover->pos[Y],
	.yp1[Z] = mover->Yneg[Z] + mover->pos[Z],
	.zp0[X] = mover->Zplus[X] + mover->pos[X],
	.zp0[Y] = mover->Zplus[Y] + mover->pos[Y],
	.zp0[Z] = mover->Zplus[Z] + mover->pos[Z],
	.zp1[X] = mover->Zneg[X] + mover->pos[X],
	.zp1[Y] = mover->Zneg[Y] + mover->pos[Y],
	.zp1[Z] = mover->Zneg[Z] + mover->pos[Z]
}; 

POINT plane_points[4];
VECTOR used_normal;
int dominant_axis = N_Yp;
bool lineChecks[3];
POINT lineEnds[3];

for(int i = 0; i < total_planes; i++)
{
	//////////////////////////////////////////////////////////////
	// Add the position of the mesh to the position of its points
	// PDATA vector space is inverted, so we negate them
	// "Get world-space point position"
	//////////////////////////////////////////////////////////////
	for(int u = 0; u < 4; u++)
	{
	plane_points[u][X] = -(mesh->pntbl[mesh->pltbl[testing_planes[i]].Vertices[u]][X]) - mesh_position[X];
	plane_points[u][Y] = -(mesh->pntbl[mesh->pltbl[testing_planes[i]].Vertices[u]][Y]) - mesh_position[Y];
	plane_points[u][Z] = -(mesh->pntbl[mesh->pltbl[testing_planes[i]].Vertices[u]][Z]) - mesh_position[Z];
	}
	// slPrint("Sample point 0:", slLocate(1, 9));
	// slPrintFX(plane_points[0][X], slLocate(2, 10));
	// slPrintFX(plane_points[0][Y], slLocate(2, 11));
	// slPrintFX(plane_points[0][Z], slLocate(2, 12));
	
	// slPrint("Sample point 2:", slLocate(19, 9));
	// slPrintFX(plane_points[2][X], slLocate(19, 10));
	// slPrintFX(plane_points[2][Y], slLocate(19, 11));
	// slPrintFX(plane_points[2][Z], slLocate(19, 12));
	
	// slPrint("Sample point 0:", slLocate(1, 9));
	// slPrintFX(mesh->pntbl[mesh->pltbl[testing_planes[i]].Vertices[0]][X], slLocate(2, 10));
	// slPrintFX(mesh->pntbl[mesh->pltbl[testing_planes[i]].Vertices[0]][Y], slLocate(2, 11));
	// slPrintFX(mesh->pntbl[mesh->pltbl[testing_planes[i]].Vertices[0]][Z], slLocate(2, 12));
	
	// slPrint("Sample point 2:", slLocate(19, 9));
	// slPrintFX(mesh->pntbl[mesh->pltbl[testing_planes[i]].Vertices[2]][X], slLocate(19, 10));
	// slPrintFX(mesh->pntbl[mesh->pltbl[testing_planes[i]].Vertices[2]][Y], slLocate(19, 11));
	// slPrintFX(mesh->pntbl[mesh->pltbl[testing_planes[i]].Vertices[2]][Z], slLocate(19, 12));
				
	//////////////////////////////////////////////////////////////
	// Grab the absolute normal used for finding the dominant axis
	//////////////////////////////////////////////////////////////
	used_normal[X] = JO_ABS(mesh->pltbl[testing_planes[i]].norm[X]);
	used_normal[Y] = JO_ABS(mesh->pltbl[testing_planes[i]].norm[Y]);
	used_normal[Z] = JO_ABS(mesh->pltbl[testing_planes[i]].norm[Z]);
	FIXED max_axis = JO_MAX(JO_MAX((used_normal[X]), (used_normal[Y])), (used_normal[Z]));
	dominant_axis = ((used_normal[X]) == max_axis) ? N_Xp : dominant_axis;
	dominant_axis = ((used_normal[Y]) == max_axis) ? N_Yp : dominant_axis;
	dominant_axis = ((used_normal[Z]) == max_axis) ? N_Zp : dominant_axis;
	//////////////////////////////////////////////////////////////
	// Grab the normal used for point-to-plane projection
	// This also retains the sign, which we use to check if the axis is + or -.
	//////////////////////////////////////////////////////////////
	used_normal[X] = (mesh->pltbl[testing_planes[i]].norm[X]);
	used_normal[Y] = (mesh->pltbl[testing_planes[i]].norm[Y]);
	used_normal[Z] = (mesh->pltbl[testing_planes[i]].norm[Z]);
	if(dominant_axis == N_Xp && used_normal[X] < 0) dominant_axis = N_Xn;
	if(dominant_axis == N_Yp && used_normal[Y] < 0) dominant_axis = N_Yn;
	if(dominant_axis == N_Zp && used_normal[Z] < 0) dominant_axis = N_Zn;
	//////////////////////////////////////////////////////////////
	// Project the lines to the plane
	// Y first, then Z, then X
	// We separate the Y axis collision check with the X-Z axis collision checks.
	// The Y axis check is the only one which will plant the player on a floor,
	// so we want to continue checking after the first positive collision check of Y to see if we hit a wall or not.
	// Conversely, if we have hit a wall, it's possible we're hitting the floor on a different plane too.
	//////////////////////////////////////////////////////////////
	if(!hitY)
	{
	lineChecks[Y] = project_segment_to_plane_and_test(moverCFs.yp0, moverCFs.yp1, plane_points[0], used_normal, lineEnds[Y]);
	} else {
	lineChecks[Y] = false;
	}
	if(!hitXZ)
	{
	lineChecks[Z] = project_segment_to_plane_and_test(moverCFs.zp0, moverCFs.zp1, plane_points[0], used_normal, lineEnds[Z]);
	lineChecks[X] = project_segment_to_plane_and_test(moverCFs.xp0, moverCFs.xp1, plane_points[0], used_normal, lineEnds[X]);	
	} else {
	lineChecks[Z] = false;
	lineChecks[X] = false;
	}
	
	// slPrint("Line Y:", slLocate(19, 9));
	// slPrintFX(lineEnds[Y][X], slLocate(19, 10));
	// slPrintFX(lineEnds[Y][Y], slLocate(19, 11));
	// slPrintFX(lineEnds[Y][Z], slLocate(19, 12));
	
	// slPrint("y0:", slLocate(1, 9));
	// slPrintFX(moverCFs.yp0[X], slLocate(1, 10));
	// slPrintFX(moverCFs.yp0[Y], slLocate(1, 11));
	// slPrintFX(moverCFs.yp0[Z], slLocate(1, 12));
	
	// slPrint("y1:", slLocate(1, 13));
	// slPrintFX(moverCFs.yp1[X], slLocate(1, 14));
	// slPrintFX(moverCFs.yp1[Y], slLocate(1, 15));
	// slPrintFX(moverCFs.yp1[Z], slLocate(1, 16));
	
	// slPrint("Line Z:", slLocate(19, 9));
	// slPrintFX(lineEnds[Z][X], slLocate(19, 10));
	// slPrintFX(lineEnds[Z][Y], slLocate(19, 11));
	// slPrintFX(lineEnds[Z][Z], slLocate(19, 12));
	
	// slPrint("z0:", slLocate(1, 9));
	// slPrintFX(moverCFs.zp0[X], slLocate(1, 10));
	// slPrintFX(moverCFs.zp0[Y], slLocate(1, 11));
	// slPrintFX(moverCFs.zp0[Z], slLocate(1, 12));
	
	// slPrint("z1:", slLocate(1, 13));
	// slPrintFX(moverCFs.zp1[X], slLocate(1, 14));
	// slPrintFX(moverCFs.zp1[Y], slLocate(1, 15));
	// slPrintFX(moverCFs.zp1[Z], slLocate(1, 16));
	
	if(!lineChecks[0] && !lineChecks[1] && !lineChecks[2]) { continue; }
	//////////////////////////////////////////////////////////////
	// If we reach this point, at least one of the lines project to a point that is inside the player's bounding box.
	// Since we know that, we can start with a winding test comparing the point to the plane.
	// When we do this, we will discard the major axis of the normal to make it 2D.
	//////////////////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////
	// Line Checks Y
	//////////////////////////////////////////////////////////////
	if(!hitY)
	{
		if(lineChecks[Y]){
			//slPrint("Testing Y", slLocate(2, 6));
			if(edge_wind_test(plane_points[0], plane_points[1], lineEnds[Y], dominant_axis))
			{
				if(edge_wind_test(plane_points[1], plane_points[2], lineEnds[Y], dominant_axis))
				{
					if(edge_wind_test(plane_points[2], plane_points[3], lineEnds[Y], dominant_axis))
					{
						if(edge_wind_test(plane_points[3], plane_points[0], lineEnds[Y], dominant_axis))
						{
							if(dominant_axis == N_Yn)
							{
								used_normal[X] = -used_normal[X]; 
								used_normal[Y] = -used_normal[Y];
								used_normal[Z] = -used_normal[Z];
								you.floorNorm[X] = used_normal[X]; 
								you.floorNorm[Y] = used_normal[Y];
								you.floorNorm[Z] = used_normal[Z];
								
								sort_angle_to_domain(used_normal, alwaysLow, you.rot);
								
								you.floorPos[X] = ((lineEnds[Y][X]) - (mover->Yneg[X]));
								you.floorPos[Y] = ((lineEnds[Y][Y]) - (mover->Yneg[Y]));
								you.floorPos[Z] = ((lineEnds[Y][Z]) - (mover->Yneg[Z]));
								you.shadowPos[X] = lineEnds[Y][X];
								you.shadowPos[Y] = lineEnds[Y][Y];
								you.shadowPos[Z] = lineEnds[Y][Z];
								
								you.hitSurface = true;
							} else {
								you.wallNorm[X] = used_normal[X];
								you.wallNorm[Y] = used_normal[Y];
								you.wallNorm[Z] = used_normal[Z];
								you.wallPos[X] = -lineEnds[Y][X];
								you.wallPos[Y] = -lineEnds[Y][Y];
								you.wallPos[Z] = -lineEnds[Y][Z];
								
								you.hitWall = true;
							}
							you.hitObject = true;
							hitY = true;
						}
					}
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////
	// Line Checks Z
	//////////////////////////////////////////////////////////////
	if(!hitXZ)
	{
		if(lineChecks[Z]){
			//slPrint("Testing Z", slLocate(2, 6));
			if(edge_wind_test(plane_points[0], plane_points[1], lineEnds[Z], dominant_axis))
			{
				if(edge_wind_test(plane_points[1], plane_points[2], lineEnds[Z], dominant_axis))
				{
					if(edge_wind_test(plane_points[2], plane_points[3], lineEnds[Z], dominant_axis))
					{
						if(edge_wind_test(plane_points[3], plane_points[0], lineEnds[Z], dominant_axis))
						{

								you.wallNorm[X] = used_normal[X];
								you.wallNorm[Y] = used_normal[Y];
								you.wallNorm[Z] = used_normal[Z];
								you.wallPos[X] = -lineEnds[Z][X];
								you.wallPos[Y] = -lineEnds[Z][Y];
								you.wallPos[Z] = -lineEnds[Z][Z];
								
								you.hitWall = true;

							you.hitObject = true;
							hitXZ = true;
						}
					}
				}
			}
		}
		
	//////////////////////////////////////////////////////////////
	// Line Checks X
	//////////////////////////////////////////////////////////////
		if(lineChecks[X]){
			//slPrint("Testing X", slLocate(2, 6));
			if(edge_wind_test(plane_points[0], plane_points[1], lineEnds[X], dominant_axis))
			{
				if(edge_wind_test(plane_points[1], plane_points[2], lineEnds[X], dominant_axis))
				{
					if(edge_wind_test(plane_points[2], plane_points[3], lineEnds[X], dominant_axis))
					{
						if(edge_wind_test(plane_points[3], plane_points[0], lineEnds[X], dominant_axis))
						{
							
							you.wallNorm[X] = used_normal[X];
							you.wallNorm[Y] = used_normal[Y];
							you.wallNorm[Z] = used_normal[Z];
							you.wallPos[X] = -lineEnds[X][X];
							you.wallPos[Y] = -lineEnds[X][Y];
							you.wallPos[Z] = -lineEnds[X][Z];
								
							you.hitWall = true;
							
							you.hitObject = true;
							hitXZ = true;
						}
					}
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////
	// If we've hit both XZ (a wall) and Y (the floor), stop testing further.
	//////////////////////////////////////////////////////////////
	if(hitXZ && hitY) return;
	//////////////////////////////////////////////////////////////
	// Per testing plane loop end stub
	//////////////////////////////////////////////////////////////
}
	if(!hitY && !hitXZ)
	{
	you.hitObject = false;
	}
	//////////////////////////////////////////////////////////////
	// Per polygon collision function end stub
	//////////////////////////////////////////////////////////////
}

		POINT	subdivided_points[128];
		short	subdivided_polygons[72][4]; //4 Vertex IDs of the subdivided_points
		short	sub_poly_cnt = 0;
		short	sub_vert_cnt = 0;

void	subdivide_plane(short start_point, short overwritten_polygon, char division_rule, unsigned char num_divisions)
{

	short tgt_pnt = start_point;
	
	short poly_a = overwritten_polygon; //Polygon A is a polygon ID we will overwrite (replace the original polygon)
	short poly_b = sub_poly_cnt;
	short poly_c = sub_poly_cnt+1;
	short poly_d = sub_poly_cnt+2;
	
	int max_z = -(32767<<16);
	
	//"Load" the original points (code shortening operation)
	FIXED * ptv[4];
	for(int u = 0; u < 4; u++)
	{
		ptv[u] = &subdivided_points[subdivided_polygons[overwritten_polygon][u]];
	}
	if(division_rule == '+') 
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
	
		if(num_divisions != 0)
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
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_a, '+', num_divisions-1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][0]][Z], subdivided_points[subdivided_polygons[poly_b][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][2]][Z], subdivided_points[subdivided_polygons[poly_b][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_b, '+', num_divisions-1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_c][0]][Z], subdivided_points[subdivided_polygons[poly_c][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_c][2]][Z], subdivided_points[subdivided_polygons[poly_c][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_c, '+', num_divisions-1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_d][0]][Z], subdivided_points[subdivided_polygons[poly_d][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_d][2]][Z], subdivided_points[subdivided_polygons[poly_d][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_d, '+', num_divisions-1);
		}
	
	} else if(division_rule == '-') 
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
		if(num_divisions != 0)
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
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_a, '-', num_divisions-1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][0]][Z], subdivided_points[subdivided_polygons[poly_b][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][2]][Z], subdivided_points[subdivided_polygons[poly_b][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_b, '-', num_divisions-1);
		}
	} else if(division_rule == '|')
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
	
		if(num_divisions != 0)
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
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_a, '|', num_divisions-1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][0]][Z], subdivided_points[subdivided_polygons[poly_b][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][2]][Z], subdivided_points[subdivided_polygons[poly_b][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_b, '|', num_divisions-1);
		}
	}
	
}




void	plane_rendering_with_subdivision(PDATA * mesh, POINT mesh_position)
{
		sub_poly_cnt = 0;
		sub_vert_cnt = 0;
		short	testing_planes[128];
		short	total_planes = 0;
		int		inverseZ = 0;
		POINT	discard_vector = {0, 0, 0};

		POINT	pl_pts[4];

    static MATRIX newMtx;
	static FIXED m0x[4];
	static FIXED m1y[4];
	static FIXED m2z[4];
	
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

prntidx = 0;

	/**
	Rendering Planes
	With polygon subdivision based on the Z depth
	
	Idea:
	Load up a plane as a polygon
	Transform its vertices by the matrix, but don't explicitly screenspace transform it.
	These vertices will be "screen-centered", so now we can subvidide the polygon.

	The subdivison method is to start with the point or edge with the highest Z.
	
	**/
	
	//////////////////////////////////////////////////////////////
	// Normal-based discard of planes
	// If the plane's normal is facing away from where the player is,
	// it will not be put into the pile of planes to collision test.
	// PDATA vector space is inverted, so we negate them
	// **DO NOT rotate the mesh or else this will not work**
	//////////////////////////////////////////////////////////////
	for(unsigned int dst_poly = 0; dst_poly < mesh->nbPolygon; dst_poly++)
	{
				discard_vector[X] = -(mesh->pntbl[mesh->pltbl[dst_poly].Vertices[0]][X])
									- you.pos[X] - mesh_position[X];
				discard_vector[Y] = -(mesh->pntbl[mesh->pltbl[dst_poly].Vertices[0]][Y])
									- you.pos[Y] - mesh_position[Y];
				discard_vector[Z] = -(mesh->pntbl[mesh->pltbl[dst_poly].Vertices[0]][Z])
									- you.pos[Z] - mesh_position[Z];

				int normal_discard = fxdot(discard_vector, mesh->pltbl[dst_poly].norm);
				
	// slPrint("Discard vector:", slLocate(1, 9));
	// slPrintFX(discard_vector[X], slLocate(2, 10));
	// slPrintFX(discard_vector[Y], slLocate(2, 11));
	// slPrintFX(discard_vector[Z], slLocate(2, 12));
	// slPrint("Dot product:", slLocate(1, 13));
	// slPrintFX(normal_discard, slLocate(2, 14));
				
				if(normal_discard >= -(5<<16) && total_planes < 128){
					testing_planes[total_planes] = dst_poly;
					total_planes++;
				}
	}
	
	//jo_printf(1, 15, "Total planes: (%i)", total_planes);

	int min_z = 32767<<16;
	int max_z = -(32767<<16);

	int manhattan_01;
	int manhattan_03;
	int number_of_subdivisions;
	
	int max_subdivisions;

for(int i = 0; i < total_planes; i++)
{
		sub_vert_cnt = 0;
		sub_poly_cnt = 0;
	for(int u = 0; u < 4; u++)
	{
	//////////////////////////////////////////////////////////////
	// Load the points
	//////////////////////////////////////////////////////////////
		pl_pts[u][X] = (mesh->pntbl[mesh->pltbl[testing_planes[i]].Vertices[u]][X]);
		pl_pts[u][Y] = (mesh->pntbl[mesh->pltbl[testing_planes[i]].Vertices[u]][Y]);
		pl_pts[u][Z] = (mesh->pntbl[mesh->pltbl[testing_planes[i]].Vertices[u]][Z]);
	//////////////////////////////////////////////////////////////
	// Matrix transformation of the plane's points
	// Note: Does not yet transform to screenspace, clip by screen or portal, or push out to near plane.
	//////////////////////////////////////////////////////////////
        subdivided_points[u][Z] = trans_pt_by_component(pl_pts[u], m2z);
        subdivided_points[u][Y] = trans_pt_by_component(pl_pts[u], m1y);
        subdivided_points[u][X] = trans_pt_by_component(pl_pts[u], m0x);

		subdivided_polygons[0][u] = u;
	}
		sub_vert_cnt += 4;
		sub_poly_cnt += 1;
	
	//////////////////////////////////////////////////////////////
	// Check: Do we need to subdivide this polygon at all?
	// If the polygon's span from 0 -> 1 is large, we want to use a subdivide rule between the edges 0->1 and 2->3.
	// If the polygon's span from 0 -> 3 is large, we want to use a subdivide rule between the edges 0->3 and 1->2.
	// If the polygon's span in both accounts is large, we want to use both subdivision rules.
	// Right-shift five to get a subdivision count in fixed-point numbers, later converted to integers.
	//////////////////////////////////////////////////////////////
		manhattan_01 = (JO_ABS(pl_pts[0][X] - pl_pts[1][X])
					+ JO_ABS(pl_pts[0][Y] - pl_pts[1][Y])
					+ JO_ABS(pl_pts[0][Z] - pl_pts[1][Z]))>>5;
					
		manhattan_03 = (JO_ABS(pl_pts[0][X] - pl_pts[3][X])
					+ JO_ABS(pl_pts[0][Y] - pl_pts[3][Y])
					+ JO_ABS(pl_pts[0][Z] - pl_pts[3][Z]))>>5;
		
	///////////////////////////////////////////
	// Find the minimum Z value of the plane. This is used to arbitrate how many times we are allowed to subdivide it.
	///////////////////////////////////////////
	min_z = JO_MIN(JO_MIN(subdivided_points[subdivided_polygons[0][0]][Z], subdivided_points[subdivided_polygons[0][1]][Z]),
			JO_MIN(subdivided_points[subdivided_polygons[0][2]][Z], subdivided_points[subdivided_polygons[0][3]][Z]));
	///////////////////////////////////////////
	//	Check the maximum Z of every new polygon.
	// 	This is the first polygon. So, if its maximum Z is too low, just discard it.
	///////////////////////////////////////////
	max_z = JO_MAX(JO_MAX(subdivided_points[subdivided_polygons[0][0]][Z], subdivided_points[subdivided_polygons[0][1]][Z]),
			JO_MAX(subdivided_points[subdivided_polygons[0][2]][Z], subdivided_points[subdivided_polygons[0][3]][Z]));
	if(max_z <= 0) continue;
	
	///////////////////////////////////////////
	// Resolve the maximum # of subdivisions.
	// This is an arbitrary scale, just based on how far away the polygon is (its Z).
	///////////////////////////////////////////
	if(min_z < (100<<16))
	{
		max_subdivisions = 3;
	} else if(min_z < 200<<16)
	{
		max_subdivisions = 2;
	} else if(min_z < 400<<16)
	{
		max_subdivisions = 1;
	} else {
		max_subdivisions = 0;
	}

	if(manhattan_01 > 65535 && manhattan_03 > 65535)
	{
	///////////////////////////////////////////
	//Average the manhattans to an integer # of divisions
	///////////////////////////////////////////
	number_of_subdivisions = (manhattan_01 + manhattan_03)>>18;
	number_of_subdivisions = (number_of_subdivisions >= max_subdivisions) ? max_subdivisions : number_of_subdivisions;
	///////////////////////////////////////////
	//Subdivide, recursively by the # of desired subdivisions (derived from the polygon's span)
	///////////////////////////////////////////
	subdivide_plane(sub_vert_cnt, 0, '+', number_of_subdivisions);
	
	} else if(manhattan_01 > 65535)
	{
	///////////////////////////////////////////
	// Convert the manhattan distance 0->1 to integer
	///////////////////////////////////////////
	number_of_subdivisions = (manhattan_01)>>17;
	number_of_subdivisions = (number_of_subdivisions >= max_subdivisions) ? max_subdivisions : number_of_subdivisions;
	///////////////////////////////////////////
	//Subdivide, recursively by the # of desired subdivisions (derived from the polygon's span)
	///////////////////////////////////////////
	subdivide_plane(sub_vert_cnt, 0, '|', number_of_subdivisions);
	} else if(manhattan_03 > 65535)
	{
	///////////////////////////////////////////
	// Convert the manhattan distance 0->3 to integer
	///////////////////////////////////////////
	number_of_subdivisions = (manhattan_03)>>17;
	number_of_subdivisions = (number_of_subdivisions >= max_subdivisions) ? max_subdivisions : number_of_subdivisions;
	///////////////////////////////////////////
	//Subdivide, recursively by the # of desired subdivisions (derived from the polygon's span)
	///////////////////////////////////////////
	subdivide_plane(sub_vert_cnt, 0, '-', number_of_subdivisions);
	}

	// slPrintFX(min_z, slLocate(1, 7 + prntidx));
	// prntidx++;
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
		ssh2VertArea[v].pnt[Z] = (subdivided_points[v][Z] > 20<<16) ? subdivided_points[v][Z] : 20<<16;
		//Get 1/z
		inverseZ = fxdiv(MsScreenDist, ssh2VertArea[v].pnt[Z]);
        //Transform to screen-space
        ssh2VertArea[v].pnt[X] = fxm(subdivided_points[v][X], inverseZ)>>SCR_SCALE_X;
        ssh2VertArea[v].pnt[Y] = fxm(subdivided_points[v][Y], inverseZ)>>SCR_SCALE_Y;
        //Screen Clip Flags for on-off screen decimation
		ssh2VertArea[v].clipFlag = ((ssh2VertArea[v].pnt[X]) > JO_TV_WIDTH_2) ? SCRN_CLIP_X : 0; 
		ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[X]) < -JO_TV_WIDTH_2) ? SCRN_CLIP_NX : ssh2VertArea[v].clipFlag; 
		ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) > JO_TV_HEIGHT_2) ? SCRN_CLIP_Y : ssh2VertArea[v].clipFlag;
		ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) < -JO_TV_HEIGHT_2) ? SCRN_CLIP_NY : ssh2VertArea[v].clipFlag;
		ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Z]) <= 20<<16) ? CLIP_Z : ssh2VertArea[v].clipFlag;
	}
	///////////////////////////////////////////
	//
	// Z-sort Insertion & Command Arrangement of Polygons
	// j = subdivided polygon index
	// testing_planes[i] = plane data index
	//
	///////////////////////////////////////////
	vertex_t * ptv[4] = {0, 0, 0, 0};
	for(int j = 0; j < sub_poly_cnt; j++)
	{

		ptv[0] = &ssh2VertArea[subdivided_polygons[j][0]];
		ptv[1] = &ssh2VertArea[subdivided_polygons[j][1]];
		ptv[2] = &ssh2VertArea[subdivided_polygons[j][2]];
		ptv[3] = &ssh2VertArea[subdivided_polygons[j][3]];
		
		 int zDepthTgt = JO_MAX(
		JO_MAX(ptv[0]->pnt[Z], ptv[2]->pnt[Z]),
		JO_MAX(ptv[1]->pnt[Z], ptv[3]->pnt[Z]));
		 int offScrn = (ptv[0]->clipFlag & ptv[1]->clipFlag & ptv[2]->clipFlag & ptv[3]->clipFlag);
			if(offScrn ) continue;
      ssh2SetCommand(ptv[0]->pnt,
		ptv[1]->pnt,
		ptv[2]->pnt,
		ptv[3]->pnt,
		2, (5264 | (mesh->attbl[testing_planes[i]].atrb & 33024)), //Reads flip value, mesh enable, and msb bit
		pcoTexDefs[mesh->attbl[testing_planes[i]].texno].SRCA, 0, pcoTexDefs[mesh->attbl[testing_planes[i]].texno].SIZE, 0, zDepthTgt);
	}
    transVerts[0] += sub_vert_cnt;
	transPolys[0] += sub_poly_cnt;
}
	//////////////////////////////////////////////////////////////
	// Planar polygon rendering with Z-based subdivision end stub
	//////////////////////////////////////////////////////////////

}





