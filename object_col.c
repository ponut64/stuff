//object_col.c
//contains per polygon collision work
#include "object_col.h"

#define MATH_TOLERANCE (16384)
#define SUBDIVISION_SCALE (50)

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


int		edge_wind_test(POINT plane_p0, POINT plane_p1, POINT test_pt, int discard)
{
	
	int left = 0;
	int right = 0;
	
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
	return left - right;
}

// Special notice for code observers:
// In rendering functions, prematrix data (which includes the translation data) is used.
// We could have simplified the function arguments here if we used the prematrix data.
// However, this cannot be done, because the prematrix data is entity-by-entity.
// Entities can be rendered multiple times, this means the prematrix data changes on which instance of the entity is rendered.
// This function is running on Master SH2. The rendering functions are running on Slave SH2.
// In this condition, the setting of the prematrix is controlled by the Slave SH2.
// Hitherto, the correct prematrix may not be set for testing collision. The position must be pointed to instead of changing the prematrix.
void	per_poly_collide(entity_t * ent, _boundBox * mover, FIXED * mesh_position)
{
		//If the entity is not loaded, cease the test.
		if(ent->file_done != true) return;
		//If the player has already hit a *different* object, cease the test.
		if(you.hitObject == true) return;

GVPLY * mesh = ent->pol;
static short testing_planes[128];
static unsigned char backfaced[128];
short total_planes = 0;
POINT discard_vector = {0, 0, 0};
bool hitY = false;
bool hitXZ = false;
bool shadowStruck = false;

	discard_vector[X] = (mesh_position[X] - mover->pos[X]);
	discard_vector[Y] = (mesh_position[Y] - mover->pos[Y]);
	discard_vector[Z] = (mesh_position[Z] - mover->pos[Z]);
	//If the player is farther away from the object than twice its radius, cease the test.
	if(discard_vector[X] > (ent->radius[X]<<1) &&
	discard_vector[Y] > (ent->radius[Y]<<1) &&
	discard_vector[Z] > (ent->radius[Z]<<1)) return;

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
			discard_vector[X] = -(mesh->pntbl[mesh->pltbl[dst_poly].Vertices[0]][X])
			- mover->prevPos[X] - mesh_position[X];
			discard_vector[Y] = -(mesh->pntbl[mesh->pltbl[dst_poly].Vertices[0]][Y])
			- mover->prevPos[Y] - mesh_position[Y];
			discard_vector[Z] = -(mesh->pntbl[mesh->pltbl[dst_poly].Vertices[0]][Z])
			- mover->prevPos[Z] - mesh_position[Z];

			int normal_discard = fxdot(discard_vector, mesh->pltbl[dst_poly].norm);
				
	// slPrint("Discard vector:", slLocate(1, 9));
	// slPrintFX(discard_vector[X], slLocate(2, 10));
	// slPrintFX(discard_vector[Y], slLocate(2, 11));
	// slPrintFX(discard_vector[Z], slLocate(2, 12));
	// slPrint("Dot product:", slLocate(1, 13));
	// slPrintFX(normal_discard, slLocate(2, 14));
			
			/////////
			// Dual-plane handling
			/////////
			if(!(mesh->attbl[dst_poly].render_data_flags & GV_FLAG_SINGLE) && total_planes < 128)
			{
				testing_planes[total_planes] = dst_poly;
				backfaced[total_planes] = (normal_discard >= 0) ? 0 : 1;
				total_planes++;
				continue;
			}
			/////////
			// Single-plane handling
			/////////
			if(normal_discard >= -(5<<16) && total_planes < 128)
			{
				testing_planes[total_planes] = dst_poly;
				backfaced[total_planes] = 0;
				total_planes++;
			}
	}
	
	//jo_printf(1, 15, "Total planes: (%i)", total_planes);
	
	//////////////////////////////////////////////////////////////
	// Add the position of the mover's box centre-faces to the mover's world position
	// "Get world-space point position"
	//////////////////////////////////////////////////////////////
_lineTable moverCFs = {
	.xp0[X] = mover->Xplus[X] + mover->pos[X] - mover->velocity[X],
	.xp0[Y] = mover->Xplus[Y] + mover->pos[Y] - mover->velocity[Y],
	.xp0[Z] = mover->Xplus[Z] + mover->pos[Z] - mover->velocity[Z],
	.xp1[X] = mover->Xneg[X] + mover->pos[X] + mover->velocity[X],
	.xp1[Y] = mover->Xneg[Y] + mover->pos[Y] + mover->velocity[Y],
	.xp1[Z] = mover->Xneg[Z] + mover->pos[Z] + mover->velocity[Z],
	.yp0[X] = mover->Yplus[X] + mover->pos[X] - mover->velocity[X],
	.yp0[Y] = mover->Yplus[Y] + mover->pos[Y] - mover->velocity[Y],
	.yp0[Z] = mover->Yplus[Z] + mover->pos[Z] - mover->velocity[Z],
	.yp1[X] = mover->Yneg[X] + mover->pos[X] + mover->velocity[X],
	.yp1[Y] = mover->Yneg[Y] + mover->pos[Y] + mover->velocity[Y],
	.yp1[Z] = mover->Yneg[Z] + mover->pos[Z] + mover->velocity[Z],
	.zp0[X] = mover->Zplus[X] + mover->pos[X] - mover->velocity[X],
	.zp0[Y] = mover->Zplus[Y] + mover->pos[Y] - mover->velocity[Y],
	.zp0[Z] = mover->Zplus[Z] + mover->pos[Z] - mover->velocity[Z],
	.zp1[X] = mover->Zneg[X] + mover->pos[X] + mover->velocity[X],
	.zp1[Y] = mover->Zneg[Y] + mover->pos[Y] + mover->velocity[Y],
	.zp1[Z] = mover->Zneg[Z] + mover->pos[Z] + mover->velocity[Z]
}; 

POINT plane_points[4];
VECTOR used_normal;
int dominant_axis = N_Yp;
bool lineChecks[3];
POINT lineEnds[3];

for(int i = 0; i < total_planes; i++)
{
	POINT plane_center = {0, 0, 0};
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
	//Add to the plane's center
	plane_center[X] += plane_points[u][X];
	plane_center[Y] += plane_points[u][Y];
	plane_center[Z] += plane_points[u][Z];
	}
	//Divide sum of plane points by 4 to average all the points
	plane_center[X] >>=2;
	plane_center[Y] >>=2;
	plane_center[Z] >>=2;

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
	lineChecks[Y] = project_segment_to_plane_and_test(moverCFs.yp0, moverCFs.yp1, plane_center, used_normal, lineEnds[Y]);
	} else {
	lineChecks[Y] = false;
	}
	if(!hitXZ)
	{
	lineChecks[Z] = project_segment_to_plane_and_test(moverCFs.zp0, moverCFs.zp1, plane_center, used_normal, lineEnds[Z]);
	lineChecks[X] = project_segment_to_plane_and_test(moverCFs.xp0, moverCFs.xp1, plane_center, used_normal, lineEnds[X]);	
	} else {
	lineChecks[Z] = false;
	lineChecks[X] = false;
	}
	//////////////////////////////////////////////////////////////
	// Shadow Posititon
	//////////////////////////////////////////////////////////////
	if((!shadowStruck || !hitY) && (lineEnds[Y][Y] < you.pos[Y]))
	{	
		if(edge_wind_test(plane_points[0], plane_points[1], lineEnds[Y], dominant_axis))
		{
			if(edge_wind_test(plane_points[1], plane_points[2], lineEnds[Y], dominant_axis))
			{
				if(edge_wind_test(plane_points[2], plane_points[3], lineEnds[Y], dominant_axis))
				{
					if(edge_wind_test(plane_points[3], plane_points[0], lineEnds[Y], dominant_axis))
					{
						shadowStruck = true;
						you.aboveObject = true;
						you.shadowPos[X] = lineEnds[Y][X];
						you.shadowPos[Y] = lineEnds[Y][Y];
						you.shadowPos[Z] = lineEnds[Y][Z];
					}
				}
			}
		}
	}
	if(!lineChecks[0] && !lineChecks[1] && !lineChecks[2]) { continue; }
	//////////////////////////////////////////////////////////////
	// If we reach this point, at least one of the lines project to a point that is inside the player's bounding box.
	// Since we know that, we can start with a winding test comparing the point to the plane.
	// When we do this, we will discard the major axis of the normal to make it 2D.
	//////////////////////////////////////////////////////////////
	// If it is a dual-plane which was determined to be otherwise back-facing, negate the normal.
	if(backfaced[i])
	{
		used_normal[X] = -used_normal[X];
		used_normal[Y] = -used_normal[Y];
		used_normal[Z] = -used_normal[Z];
	}
	//////////////////////////////////////////////////////////////
	// Line Checks Y
	//////////////////////////////////////////////////////////////
	if(!hitY)
	{
		if(lineChecks[Y]){
			//slPrint("Testing Y", slLocate(2, 6));
			if(edge_wind_test(plane_points[0], plane_points[1], lineEnds[Y], dominant_axis) >= 0)
			{
				if(edge_wind_test(plane_points[1], plane_points[2], lineEnds[Y], dominant_axis) >= 0)
				{
					if(edge_wind_test(plane_points[2], plane_points[3], lineEnds[Y], dominant_axis) >= 0)
					{
						if(edge_wind_test(plane_points[3], plane_points[0], lineEnds[Y], dominant_axis) >= 0)
						{
							if(dominant_axis == N_Yn && !backfaced[i])
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
							you.aboveObject = true;
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
			if(edge_wind_test(plane_points[0], plane_points[1], lineEnds[Z], dominant_axis) >= 0)
			{
				if(edge_wind_test(plane_points[1], plane_points[2], lineEnds[Z], dominant_axis) >= 0)
				{
					if(edge_wind_test(plane_points[2], plane_points[3], lineEnds[Z], dominant_axis) >= 0)
					{
						if(edge_wind_test(plane_points[3], plane_points[0], lineEnds[Z], dominant_axis) >= 0)
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
			if(edge_wind_test(plane_points[0], plane_points[1], lineEnds[X], dominant_axis) >= 0)
			{
				if(edge_wind_test(plane_points[1], plane_points[2], lineEnds[X], dominant_axis) >= 0)
				{
					if(edge_wind_test(plane_points[2], plane_points[3], lineEnds[X], dominant_axis) >= 0)
					{
						if(edge_wind_test(plane_points[3], plane_points[0], lineEnds[X], dominant_axis) >= 0)
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
	//////////////////////////////////////////////////////////////
	// Per polygon collision function end stub
	//////////////////////////////////////////////////////////////
}

/**

**/


		POINT	subdivided_points[512];
		short	subdivided_polygons[256][4]; //4 Vertex IDs of the subdivided_points
		short	used_textures[256];
		short	sub_poly_cnt = 0;
		short	sub_vert_cnt = 0;


void	subdivide_plane(short start_point, short overwritten_polygon, FIXED * norm)
{

	const int overflow_protection = 4;

	short tgt_pnt = start_point;
	
	short poly_a = overwritten_polygon; //Polygon A is a polygon ID we will overwrite (replace the original polygon)
	
	//"Load" the original points (code shortening operation)
	FIXED upit[4][3];
	FIXED * ptv[5];
	for(int u = 0; u < 4; u++)
	{
		// ptv[u] = &subdivided_points[subdivided_polygons[overwritten_polygon][u]][X];
		upit[u][X] = subdivided_points[subdivided_polygons[overwritten_polygon][u]][X];
		upit[u][Y] = subdivided_points[subdivided_polygons[overwritten_polygon][u]][Y];
		upit[u][Z] = subdivided_points[subdivided_polygons[overwritten_polygon][u]][Z];
		ptv[u] = upit[u];
	}
	
 	VECTOR used_normal;
	int dominant_axis;
	//////////////////////////////////////////////////////////////
	// Grab the absolute normal used for finding the dominant axis
	//////////////////////////////////////////////////////////////
	used_normal[X] = JO_ABS(norm[X]);
	used_normal[Y] = JO_ABS(norm[Y]);
	used_normal[Z] = JO_ABS(norm[Z]);
	FIXED max_axis = JO_MAX(JO_MAX((used_normal[X]), (used_normal[Y])), (used_normal[Z]));
	dominant_axis = ((used_normal[X]) == max_axis) ? N_Xp : dominant_axis;
	dominant_axis = ((used_normal[Y]) == max_axis) ? N_Yp : dominant_axis;
	dominant_axis = ((used_normal[Z]) == max_axis) ? N_Zp : dominant_axis;
	//////////////////////////////////////////////////////////////
	// Grab the normal used for point-to-plane projection
	// This also retains the sign, which we use to check if the axis is + or -.
	//////////////////////////////////////////////////////////////
	used_normal[X] = (norm[X]);
	used_normal[Y] = (norm[Y]);
	used_normal[Z] = (norm[Z]);

	if(dominant_axis == N_Xp && used_normal[X] < 0) dominant_axis = N_Xn;
	if(dominant_axis == N_Yp && used_normal[Y] < 0) dominant_axis = N_Yn;
	if(dominant_axis == N_Zp && used_normal[Z] < 0) dominant_axis = N_Zn;
	//////////////////////////////////////////////////////////////
	 
	// Initial Conditions
	//////////////////////
	//
	//////////////////////
	int grid_scale = -(25<<16);
	POINT center;
	center[X] = (ptv[0][X] + ptv[1][X] + ptv[2][X] + ptv[3][X])>>2;
	center[Y] = (ptv[0][Y] + ptv[1][Y] + ptv[2][Y] + ptv[3][Y])>>2;
	center[Z] = (ptv[0][Z] + ptv[1][Z] + ptv[2][Z] + ptv[3][Z])>>2;
	FIXED cptv[4][3];
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 3; j++)
		{
		cptv[i][j] = ptv[i][j] - center[j];
		}
	}
	//////////////////////
	// Step 2: Produce unit vectors
	//////////////////////////////////////////
	// 0 -> 1
	static VECTOR tVector;
	static VECTOR unit01;
	tVector[X] = (ptv[0][X] - ptv[1][X])>>overflow_protection;
	tVector[Y] = (ptv[0][Y] - ptv[1][Y])>>overflow_protection;
	tVector[Z] = (ptv[0][Z] - ptv[1][Z])>>overflow_protection;
	double_normalize(tVector, unit01);
	//////////////////////////////////////////
	// Local Y axis (cross-product of 0->1 and the polygon's normal)
	static VECTOR gUnit;
	static VECTOR gridAngle;
	cross_fixed(tVector, norm, gridAngle);
	double_normalize(gridAngle, gUnit);
	//////////////////////////////////////////
	// 0 -> 3
	static VECTOR vector30;
	vector30[X] = (ptv[3][X] - ptv[0][X]);//>>overflow_protection;
	vector30[Y] = (ptv[3][Y] - ptv[0][Y]);//>>overflow_protection;
	vector30[Z] = (ptv[3][Z] - ptv[0][Z]);//>>overflow_protection;
	//////////////////////////////////////////
	// 2 -> 1
	static VECTOR vector21;
	vector21[X] = (ptv[1][X] - ptv[2][X]);//>>overflow_protection;
	vector21[Y] = (ptv[1][Y] - ptv[2][Y]);//>>overflow_protection;
	vector21[Z] = (ptv[1][Z] - ptv[2][Z]);//>>overflow_protection;
	//////////////////////////////////////////
	// 2 -> 3
	static VECTOR vector23;
	vector23[X] = (ptv[2][X] - ptv[3][X]);//>>overflow_protection;
	vector23[Y] = (ptv[2][Y] - ptv[3][Y]);//>>overflow_protection;
	vector23[Z] = (ptv[2][Z] - ptv[3][Z]);//>>overflow_protection;

	int scale_to_21 = 0;
	int scale_to_23 = 0;
	int scale_to_30 = 0;
	/*
	//Okay! So I have solved some things:
	// 1. Can determine accurately if line intersection is inside an edge, or outside of it 
	//	(may need to do overflow filtering or divide by zero filtering)
	// 2. Should then be able to cleanly decide # of intersections
	// if it's three, we should just stop the process and don't change the polygon (dunno how that'd happen, but...)
	// 3. Should be able to step along, cleanly, until all intersections are outside of the polygon
	//	Should also be able to find out which verts are above the intersection, and include them in the shape.
	*/

	POINT first_step;
	int lineHits[3][3];
		for(int i = 0; i < 3; i++)
		{
	first_step[i] = fxm(gUnit[i], grid_scale) + ptv[0][i];
		}
		
	/*
	IIRC, been awhile:
	The idea is to step along the grid angle from point 0, and then draw a line in the same direction as the edge 0->1
	and see where that line intersects with all other edges of the polygon.
	If there are only two intersections, it should be then valid to use those two intersections to form a new polygon.
	The projection along the grid angle direction is stored, and used again when we project the next line down.
	There are important exception cases to consider when the new line intersections "overtake" a vertex of the polygon.
	In this case, that new vertex will need to be incorporated into a new triangle, and that triangle thusly excluded from subdivision.
	Rhombus shapes might cause that.
	
	To be accurate, most of the time the lines will only intersect 1->2 and 3->0.
	If ever their intersection does not lie within these segments, we should check 2->3.
	If it doesn't intersect with 2->3 either, stop!
	The 2->3 intersection should be simply solved when 1->2 is missing, it is vert 2. When 3->0 is missing, it's vert 3.
	In the weird case where a line intersects three edges, this polygon is odd and shouldn't be subdivided at all.
	*/
	
	//(if it's a triangle, don't do this part)
	if(!(ptv[2][X] == ptv[3][X] && ptv[2][Y] == ptv[3][Y] && ptv[2][Z] == ptv[3][Z]))
	{
	scale_to_23 = JO_ABS(line_intersection_function(ptv[2], vector23, first_step, unit01, lineHits[1]));
	}
	
	scale_to_21 = JO_ABS(line_intersection_function(ptv[1], vector21, first_step, unit01, lineHits[0]));
	if((scale_to_21) < (1<<16) && scale_to_21 < (1000<<16))
	{
		cpy3(lineHits[0], subdivided_points[2]);
	} else if((scale_to_23) < (1<<16) && scale_to_23 < (1000<<16))
	{
		cpy3(lineHits[1], subdivided_points[2]);
	} else {
		//Stop condition (after repeating the stepping of polygons down the shape). 
	}
	
	scale_to_30 = JO_ABS(line_intersection_function(ptv[3], vector30, first_step, unit01, lineHits[2]));
	if((scale_to_30) < (1<<16) && scale_to_30 < (1000<<16))
	{
		cpy3(lineHits[2], subdivided_points[3]);
	} else if((scale_to_23) < (1<<16) && scale_to_23 < (1000<<16))
	{
		cpy3(lineHits[1], subdivided_points[3]);
	} else {
		//Stop condition.
	}
		
		
	//	if(!(ptv[2][X] == ptv[3][X] && ptv[2][Y] == ptv[3][Y] && ptv[2][Z] == ptv[3][Z]))
		//if(scale_to_30 < 255 && scale_to_21 < 255 && scale_to_23 < 255)
		//{
			
		slPrint("scales:", slLocate(0, 6));
		slPrintFX(scale_to_30, slLocate(2, 7));
		slPrintFX(scale_to_23, slLocate(2, 8));
		slPrintFX(scale_to_21, slLocate(2, 9));
			
		// slPrint("u01 / v12:", slLocate(0, 6));
		// slPrintFX(unit01[X], slLocate(2, 7));
		// slPrintFX(unit01[Y], slLocate(2, 8));
		// slPrintFX(unit01[Z], slLocate(2, 9));

		// slPrintFX(vector21[X], slLocate(18, 7));
		// slPrintFX(vector21[Y], slLocate(18, 8));
		// slPrintFX(vector21[Z], slLocate(18, 9));
		// slPrint("v23 / v30:", slLocate(0, 10));
		// slPrintFX(vector23[X], slLocate(2, 7+4));
		// slPrintFX(vector23[Y], slLocate(2, 8+4));
		// slPrintFX(vector23[Z], slLocate(2, 9+4));

		// slPrintFX(vector30[X], slLocate(18, 7+4));
		// slPrintFX(vector30[Y], slLocate(18, 8+4));
		// slPrintFX(vector30[Z], slLocate(18, 9+4));
	//	}
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

	int min_z = 32767<<16;
	int max_z = -(32767<<16);

	int number_of_subdivisions;
	
	int max_subdivisions;
	int specific_texture;
	
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
	//////////////////////////////////////////////////////////////
	// Associate vertices with polygon 0
	//////////////////////////////////////////////////////////////
		subdivided_polygons[0][u] = u;
	//////////////////////////////////////////////////////////////
	// Early screenspace transform to throw out off-screen planes
	//////////////////////////////////////////////////////////////
		//Push to near-plane
		ssh2VertArea[u].pnt[Z] = (subdivided_points[u][Z] > 20<<16) ? subdivided_points[u][Z] : 20<<16;
		//Get 1/z
		SetFixDiv(MsScreenDist, ssh2VertArea[u].pnt[Z]);
		
		ssh2VertArea[u].clipFlag = ((ssh2VertArea[u].pnt[Z]) <= 20<<16) ? CLIP_Z : 0;
		
		inverseZ = *DVDNTL;
        //Transform to screen-space
        ssh2VertArea[u].pnt[X] = fxm(subdivided_points[u][X], inverseZ)>>SCR_SCALE_X;
        ssh2VertArea[u].pnt[Y] = fxm(subdivided_points[u][Y], inverseZ)>>SCR_SCALE_Y;
        //Screen Clip Flags for on-off screen decimation
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[X]) > JO_TV_WIDTH_2) ? SCRN_CLIP_X : ssh2VertArea[u].clipFlag; 
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[X]) < -JO_TV_WIDTH_2) ? SCRN_CLIP_NX : ssh2VertArea[u].clipFlag; 
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[Y]) > JO_TV_HEIGHT_2) ? SCRN_CLIP_Y : ssh2VertArea[u].clipFlag;
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[Y]) < -JO_TV_HEIGHT_2) ? SCRN_CLIP_NY : ssh2VertArea[u].clipFlag;
		
		subdivided_points[u][X] = pl_pts[u][X];
		subdivided_points[u][Y] = pl_pts[u][Y];
		subdivided_points[u][Z] = pl_pts[u][Z];
		
	}
		 if(ssh2VertArea[0].clipFlag
		 & ssh2VertArea[1].clipFlag
		 & ssh2VertArea[2].clipFlag
		 & ssh2VertArea[3].clipFlag) continue;
	flags = mesh->attbl[i].render_data_flags;
	//flip = GET_FLIP_DATA(flags);
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
	flags = (((flags & GV_FLAG_MESH)>>1) | ((flags & GV_FLAG_DARK)<<4))<<8;
	//////////////////////////////////////////////////////////////
	// We have at least four vertices, and at least one polygon (the plane's data itself).
	//////////////////////////////////////////////////////////////
		sub_vert_cnt += 4;
		sub_poly_cnt += 1;
	/////////////////////////////////////////////
	// Subdivisions
	/////////////////////////////////////////////
		subdivide_plane(sub_vert_cnt, 0, mesh->pltbl[i].norm);
		used_textures[0] = 4;
			if(i == 10)
			{

			// slPrintFX(subdivided_points[4][X], slLocate(2, 7));
			// slPrintFX(subdivided_points[4][Y], slLocate(2, 8));
			// slPrintFX(subdivided_points[4][Z], slLocate(2, 9));
	
			// slPrintFX(subdivided_points[5][X], slLocate(2, 7+3));
			// slPrintFX(subdivided_points[5][Y], slLocate(2, 8+3));
			// slPrintFX(subdivided_points[5][Z], slLocate(2, 9+3));
			
			// slPrintFX(subdivided_points[6][X], slLocate(18, 7));
			// slPrintFX(subdivided_points[6][Y], slLocate(18, 8));
			// slPrintFX(subdivided_points[6][Z], slLocate(18, 9));
				
			// slPrintFX(subdivided_points[5][X], slLocate(18, 7+3));
			// slPrintFX(subdivided_points[5][Y], slLocate(18, 8+3));
			// slPrintFX(subdivided_points[5][Z], slLocate(18, 9+3));
				
			// jo_printf(2, 15, "sbcnt(%i)", sub_vert_cnt);
			}
		
		
	///////////////////////////////////////////
	//
	// Screenspace Transform of Vertices
	// v = subdivided point index
	//
	///////////////////////////////////////////
	for(int v = 0; v < sub_vert_cnt; v++)
	{
        /**calculate z**/
        ssh2VertArea[v].pnt[Z] = trans_pt_by_component(subdivided_points[v], m2z);
		ssh2VertArea[v].pnt[Z] = (ssh2VertArea[v].pnt[Z] > nearP) ? ssh2VertArea[v].pnt[Z] : nearP;

         /**Starts the division**/
        SetFixDiv(MsScreenDist, ssh2VertArea[v].pnt[Z]);

        /**Calculates X and Y while waiting for screenDist/z **/
        ssh2VertArea[v].pnt[Y] = trans_pt_by_component(subdivided_points[v], m1y);
        ssh2VertArea[v].pnt[X] = trans_pt_by_component(subdivided_points[v], m0x);
		
        /** Retrieves the result of the division **/
		inverseZ = *DVDNTL;

        /**Transform X and Y to screen space**/
        ssh2VertArea[v].pnt[X] = fxm(ssh2VertArea[v].pnt[X], inverseZ)>>SCR_SCALE_X;
        ssh2VertArea[v].pnt[Y] = fxm(ssh2VertArea[v].pnt[Y], inverseZ)>>SCR_SCALE_Y;
 
        //Screen Clip Flags for on-off screen decimation
		//Simplified to increase CPU performance
		ssh2VertArea[v].clipFlag = ((ssh2VertArea[v].pnt[X]) > JO_TV_WIDTH_2) ? SCRN_CLIP_X : 0; 
		ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[X]) < -JO_TV_WIDTH_2) ? SCRN_CLIP_NX : ssh2VertArea[v].clipFlag; 
		ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) > JO_TV_HEIGHT_2) ? SCRN_CLIP_Y : ssh2VertArea[v].clipFlag;
		ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) < -JO_TV_HEIGHT_2) ? SCRN_CLIP_NY : ssh2VertArea[v].clipFlag;
		ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Z]) <= nearP) ? CLIP_Z : ssh2VertArea[v].clipFlag;
	}
	///////////////////////////////////////////
	//
	// Z-sort Insertion & Command Arrangement of Polygons
	// j = subdivided polygon index
	// testing_planes[i] = plane data index
	//
	///////////////////////////////////////////
	if(ssh2SentPolys[0] + sub_poly_cnt > MAX_SSH2_SENT_POLYS) return;
	
	
	vertex_t * ptv[5] = {0, 0, 0, 0, 0};
	for(int j = 0; j < sub_poly_cnt; j++)
	{
		
		ptv[0] = &ssh2VertArea[subdivided_polygons[j][0]];
		ptv[1] = &ssh2VertArea[subdivided_polygons[j][1]];
		ptv[2] = &ssh2VertArea[subdivided_polygons[j][2]];
		ptv[3] = &ssh2VertArea[subdivided_polygons[j][3]];
		
		 int offScrn = (ptv[0]->clipFlag & ptv[1]->clipFlag & ptv[2]->clipFlag & ptv[3]->clipFlag);
			if(offScrn) continue;
		///////////////////////////////////////////
		// Use a combined texture, if the subdivision system stated one should be used.
		// Otherwise, use the base texture.
		///////////////////////////////////////////
			if(used_textures[j] != 0)
			{
				specific_texture = ((mesh->attbl[i].texno - ent->base_texture)<<2)
					 + (ent->numTexture + ent->base_texture) + used_textures[j];
			} else {
				specific_texture = mesh->attbl[i].texno;
			}
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
		///////////////////////////////////////////
		// Flipping polygon such that vertice 0 is on-screen, or disable pre-clipping
		///////////////////////////////////////////
//		flip = 0;
//		pclp = 0; 
//		if( (ptv[0]->clipFlag & 12) ){ //Vertical flip
//			//Incoming Arrangement:
//			// 0 - 1		^
//			//-------- Edge | Y-
//			// 3 - 2		|
//			//				
//           ptv[4] = ptv[3]; ptv[3] = ptv[0]; ptv[0] = ptv[4];
//           ptv[4] = ptv[1]; ptv[1] = ptv[2]; ptv[2] = ptv[4];
//           flip ^= 1<<5; //sprite flip value [v flip]
//			//Outgoing Arrangement:
//			// 3 - 2		^
//			//-------- Edge | Y-
//			// 0 - 1		|
//		} else if( (ptv[0]->clipFlag & 3) ){//H flip 
//			//Incoming Arrangement:
//			//	0 | 1
//			//	3 | 2
//			//	 Edge  ---> X+
//           ptv[4] = ptv[1];  ptv[1]=ptv[0];  ptv[0] = ptv[4];
//           ptv[4] = ptv[2];  ptv[2]=ptv[3];  ptv[3] = ptv[4];
//           flip ^= 1<<4; //sprite flip value [h flip]
//			//Outgoing Arrangement:
//			// 1 | 0
//			// 2 | 3
//			//	Edge	---> X+
//		} else if( !ptv[0]->clipFlag && !ptv[1]->clipFlag && !ptv[2]->clipFlag && !ptv[3]->clipFlag)
//		{
//			pclp = 2048; //Preclipping Disable
//		} 
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
		luma += fxdot(mesh->pltbl[i].norm, active_lights[0].ambient_light); 
		//Use transformed normal as shade determinant
		colorBank = (luma >= 98294) ? 0 : 1;
		colorBank = (luma < 49152) ? 2 : colorBank;
		colorBank = (luma < 32768) ? 3 : colorBank; 
		colorBank = (luma < 16384) ? 515 : colorBank; //Make really dark? use MSB shadow
			
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





