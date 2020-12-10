//object_col.c
//contains per polygon collision work
#include "object_col.h"

#define MATH_TOLERANCE (16384)

int prntidx = 0;


bool	project_segment_to_plane_and_test(FIXED p0[XYZ], FIXED p1[XYZ], FIXED point_on_plane[XYZ], FIXED unitNormal[XYZ], FIXED output[XYZ])
{

	FIXED line_scalar = 0;
	FIXED vseg[XYZ] = {0, 0, 0};
	FIXED w[XYZ] = {0, 0, 0};
	
	vseg[X] = p0[X] - p1[X];
	vseg[Y] = p0[Y] - p1[Y];
	vseg[Z] = p0[Z] - p1[Z];

	w[X] = (point_on_plane[X] - p0[X]);
	w[Y] = (point_on_plane[Y] - p0[Y]);
	w[Z] = (point_on_plane[Z] - p0[Z]);
	

	line_scalar = fxdiv(fxdot(w, unitNormal), fxdot(vseg, unitNormal));
	if(line_scalar > (1000<<16) || line_scalar < -(1000<<16)){
		return false;
	}

	output[X] = (p0[X] + fxm(vseg[X], line_scalar));
	output[Y] = (p0[Y] + fxm(vseg[Y], line_scalar));
	output[Z] = (p0[Z] + fxm(vseg[Z], line_scalar));

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
	if(plane_p0[X] == plane_p1[X] &&
	plane_p0[Y] == plane_p1[Y] &&
	plane_p0[Z] == plane_p1[Z])
	{
		return true;
	}
	
	if(discard == N_Xp)
	{
		left = fxm((test_pt[Y] - plane_p0[Y]), (plane_p1[Z] - plane_p0[Z]));
		right = fxm((test_pt[Z] - plane_p0[Z]), (plane_p1[Y] - plane_p0[Y]));
		//slPrint("Discard X+", slLocate(2, 5));
	} else if(discard == N_Zp)
	{
		left = fxm((test_pt[X] - plane_p0[X]), (plane_p1[Y] - plane_p0[Y]));
		right = fxm((test_pt[Y] - plane_p0[Y]), (plane_p1[X] - plane_p0[X]));
		//slPrint("Discard Z+", slLocate(2, 5));
	} else if(discard == N_Yn)
	{
		left = fxm((test_pt[X] - plane_p0[X]), (plane_p1[Z] - plane_p0[Z]));
		right = fxm((test_pt[Z] - plane_p0[Z]), (plane_p1[X] - plane_p0[X]));
		//slPrint("Discard Y+", slLocate(2, 5));
	} else if(discard == N_Xn)
	{
		right = fxm((test_pt[Y] - plane_p0[Y]), (plane_p1[Z] - plane_p0[Z]));
		left = fxm((test_pt[Z] - plane_p0[Z]), (plane_p1[Y] - plane_p0[Y]));
		//slPrint("Discard X-", slLocate(2, 5));
	} else if(discard == N_Zn)
	{
		right = fxm((test_pt[X] - plane_p0[X]), (plane_p1[Y] - plane_p0[Y]));
		left = fxm((test_pt[Y] - plane_p0[Y]), (plane_p1[X] - plane_p0[X]));
		//slPrint("Discard Z-", slLocate(2, 5));
	} else if(discard == N_Yp)
	{
		right = fxm((test_pt[X] - plane_p0[X]), (plane_p1[Z] - plane_p0[Z]));
		left = fxm((test_pt[Z] - plane_p0[Z]), (plane_p1[X] - plane_p0[X]));
		//slPrint("Discard Y-", slLocate(2, 5));
	}
	// slPrint("Left:", slLocate(2, 7 + (prntidx * 2)));
	// slPrintFX(left, slLocate(2, 8 + (prntidx * 2)));
	
	// slPrint("Right:", slLocate(18, 7 + (prntidx * 2)));
	// slPrintFX(right, slLocate(18, 8 + (prntidx * 2)));
	prntidx++;
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
	
	jo_printf(1, 15, "Total planes: (%i)", total_planes);
	
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





