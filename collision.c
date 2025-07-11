/*
This file is compiled separately.
*/
///collision.c

#include <sl_def.h>
#include "def.h"
#include "mymath.h"

#include "mloader.h"
#include "render.h"
#include "physobjet.h"
#include "particle.h"
#include "ldata.h"

#include "collision.h"

#define MARGIN	(0)

int numBoxChecks = 0;

int boxDisField[6];

void	init_box_handling(void)
{
	
boxDisField[0] = N_Yn;
boxDisField[1] = N_Zn;
boxDisField[2] = N_Xn;
boxDisField[3] = N_Yp;
boxDisField[4] = N_Zp;
boxDisField[5] = N_Xp;

}


int edge_wind_test(int * pp0, int * pp1, int * pp2, int * pp3, int * tpt, int discard, short shift)
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
	int * plane_p0;
	int * plane_p1;
	
	/////////////////////////////////////////////////////////////////////////////////////////////
	//Using integer math. The precision of fixed point is not required, and this prevents overflows.
	/////////////////////////////////////////////////////////////////////////////////////////////
	// Don't touch this: it works. It's reliable. It's basic. There is no need to edit it.
	/////////////////////////////////////////////////////////////////////////////////////////////
	switch(discard)
	{
	case (N_Xp):
		plane_p0 = pp0;
		plane_p1 = pp1;
		left = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		right = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp1;
		plane_p1 = pp2;
		left = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		right = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp2;
		plane_p1 = pp3;
		left = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		right = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp3;
		plane_p1 = pp0;
		left = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		right = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		if((left-right) < MARGIN) break;
		return 1;
		break;
	case (N_Zp):
		plane_p0 = pp0;
		plane_p1 = pp1;
		left = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		right = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp1;
		plane_p1 = pp2;
		left = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		right = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp2;
		plane_p1 = pp3;
		left = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		right = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp3;
		plane_p1 = pp0;
		left = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		right = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		return 1;
		break;
	case (N_Yn):
		plane_p0 = pp0;
		plane_p1 = pp1;
		left = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		right = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp1;
		plane_p1 = pp2;
		left = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		right = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp2;
		plane_p1 = pp3;
		left = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		right = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp3;
		plane_p1 = pp0;
		left = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		right = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		return 1;
		break;
	case (N_Xn):
		plane_p0 = pp0;
		plane_p1 = pp1;
		right = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		left = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp1;
		plane_p1 = pp2;
		right = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		left = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp2;
		plane_p1 = pp3;
		right = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		left = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp3;
		plane_p1 = pp0;
		right = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		left = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		if((left-right) < MARGIN) break;
		return 1;
		break;
	case (N_Zn):
		plane_p0 = pp0;
		plane_p1 = pp1;
		right = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		left = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp1;
		plane_p1 = pp2;
		right = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		left = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp2;
		plane_p1 = pp3;
		right = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		left = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp3;
		plane_p1 = pp0;
		right = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Y] - plane_p0[Y])>>shift);
		left = ((tpt[Y] - plane_p0[Y])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		return 1;
		break;
	case (N_Yp):
		//I'm serious. Don't touch this. Please.
		plane_p0 = pp0;
		plane_p1 = pp1;
		right = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		left = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp1;
		plane_p1 = pp2;
		right = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		left = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp2;
		plane_p1 = pp3;
		right = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		left = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		plane_p0 = pp3;
		plane_p1 = pp0;
		right = ((tpt[X] - plane_p0[X])>>shift) * ((plane_p1[Z] - plane_p0[Z])>>shift);
		left = ((tpt[Z] - plane_p0[Z])>>shift) * ((plane_p1[X] - plane_p0[X])>>shift);
		if((left-right) < MARGIN) break;
		return 1;
		break;
	}
	// slPrint("Left:", slLocate(2, 7 + (prntidx * 2)));
	// slPrintFX(left, slLocate(2, 8 + (prntidx * 2)));
	return 0;
	// slPrint("Right:", slLocate(18, 7 + (prntidx * 2)));
	// slPrintFX(right, slLocate(18, 8 + (prntidx * 2)));
	//prntidx++;
}

int edge_projection_test(int * pp0, int * pp1, int * pp2, int * pp3, _lineTable * boxAxis, _boundBox * box, int discard)
{
	
	/*
	Edge Projection Test
	
	Hitherto shall be constructed a vector from each edge of the polygon supplied as:
	(pp0 -> pp1)
	(pp1 -> pp2)
	(pp2 -> pp3)
	(pp3 -> pp0)

	What does the code do now?
	It takes the (discard) and applies the (discard) as:
	vect_point, vect_norm, and polygon points.
	So if N_Xp, all of those will point back to X+ face/normal/center.
	Furthermore, the discard, to check if within the face, will proceed with (discard).
	
	So if we want to actually check a polygon that might face X+ but be a shape that won't collide with its center,
	we need to do this test with Z+/- and Y+/-.
	
	We could do a simple test to check the sign of the difference between the box and the polygon, center to center.
	This way, we could confine the test to only two walls (eight projections), instead of four walls (sixteen projections);
	this is still a very intense solution and there is absolutely a simpler way.

	*/
	
	if(box->status[3] != 'B') return false;
	
	static int intersections[4][3];

	int * vect_point;
	int * vect_norm;
	int * plpnt[4];

	switch(discard)
	{
	case (N_Xp):
		//Polygon #5
		vect_point = boxAxis->xp0;
		vect_norm = box->UVX;
		plpnt[0] = box->pltbl[5][0];
		plpnt[1] = box->pltbl[5][1];
		plpnt[2] = box->pltbl[5][2];
		plpnt[3] = box->pltbl[5][3];
		break;
	case (N_Zp):
		//Polygon #4
		vect_point = boxAxis->zp0;
		vect_norm = box->UVZ;
		plpnt[0] = box->pltbl[4][0];
		plpnt[1] = box->pltbl[4][1];
		plpnt[2] = box->pltbl[4][2];
		plpnt[3] = box->pltbl[4][3];
		break;
	case (N_Yn):
	default:
		//Polygon #0
		vect_point = boxAxis->yp1;
		vect_norm = box->UVNY;
		plpnt[0] = box->pltbl[0][0];
		plpnt[1] = box->pltbl[0][1];
		plpnt[2] = box->pltbl[0][2];
		plpnt[3] = box->pltbl[0][3];
		break;
	case (N_Xn):
		//Polygon #2
		vect_point = boxAxis->xp1;
		vect_norm = box->UVNX;
		plpnt[0] = box->pltbl[2][0];
		plpnt[1] = box->pltbl[2][1];
		plpnt[2] = box->pltbl[2][2];
		plpnt[3] = box->pltbl[2][3];
		break;
	case (N_Zn):
		//Polygon #1
		vect_point = boxAxis->zp1;
		vect_norm = box->UVNZ;
		plpnt[0] = box->pltbl[1][0];
		plpnt[1] = box->pltbl[1][1];
		plpnt[2] = box->pltbl[1][2];
		plpnt[3] = box->pltbl[1][3];
		break;
	case (N_Yp):
		//Polygon #3
		vect_point = boxAxis->yp0;
		vect_norm = box->UVY;
		plpnt[0] = box->pltbl[3][0];
		plpnt[1] = box->pltbl[3][1];
		plpnt[2] = box->pltbl[3][2];
		plpnt[3] = box->pltbl[3][3];
		break;
	}
	
		if(line_hit_plane_here(pp0, pp1, vect_point, vect_norm, zPt, 16384, intersections[0]))
		{
			if(edge_wind_test(plpnt[0], plpnt[1], plpnt[2], plpnt[3], intersections[0], discard, 12))
			{
				return true;
			}
		}
		if(line_hit_plane_here(pp1, pp2, vect_point, vect_norm, zPt, 16384, intersections[1]))
		{
			if(edge_wind_test(plpnt[0], plpnt[1], plpnt[2], plpnt[3], intersections[1], discard, 12))
			{
				return true;
			}
		}
		if(line_hit_plane_here(pp2, pp3, vect_point, vect_norm, zPt, 16384, intersections[2]))
		{
			if(edge_wind_test(plpnt[0], plpnt[1], plpnt[2], plpnt[3], intersections[2], discard, 12))
			{
				return true;
			}
		}
		if(line_hit_plane_here(pp3, pp0, vect_point, vect_norm, zPt, 16384, intersections[3]))
		{
			if(edge_wind_test(plpnt[0], plpnt[1], plpnt[2], plpnt[3], intersections[3], discard, 12))
			{
				return true;
			}
		}
		
	return false;
	
}


Bool simple_collide(FIXED pos[XYZ], _boundBox * targetBox)
{
	if(realpt_to_plane(pos, targetBox->Zplus, targetBox->pos) < (-HIT_TOLERANCE)){return false;}
	if(realpt_to_plane(pos, targetBox->Zneg, targetBox->pos) < (-HIT_TOLERANCE)){return false;}
	if(realpt_to_plane(pos, targetBox->Xplus, targetBox->pos) < (-HIT_TOLERANCE)){return false;}
	if(realpt_to_plane(pos, targetBox->Xneg, targetBox->pos) < (-HIT_TOLERANCE)){return false;}
	if(realpt_to_plane(pos, targetBox->Yplus, targetBox->pos) < (-HIT_TOLERANCE)){return false;}
	if(realpt_to_plane(pos, targetBox->Yneg, targetBox->pos) < (-HIT_TOLERANCE)){return false;}
	return true;
}

int aMtx[XYZ][XYZ];

void	finalize_alignment(_boundBox * fmtx)
{
	
fmtx->UVX[X] = aMtx[X][X];
fmtx->UVX[Y] = aMtx[X][Y];
fmtx->UVX[Z] = aMtx[X][Z];
fmtx->UVY[X] = aMtx[Y][X];
fmtx->UVY[Y] = aMtx[Y][Y];
fmtx->UVY[Z] = aMtx[Y][Z];
fmtx->UVZ[X] = aMtx[Z][X];
fmtx->UVZ[Y] = aMtx[Z][Y];
fmtx->UVZ[Z] = aMtx[Z][Z];
	
fmtx->UVNX[X] = -fmtx->UVX[X];
fmtx->UVNX[Y] = -fmtx->UVX[Y];
fmtx->UVNX[Z] = -fmtx->UVX[Z];
fmtx->UVNY[X] = -fmtx->UVY[X];
fmtx->UVNY[Y] = -fmtx->UVY[Y];
fmtx->UVNY[Z] = -fmtx->UVY[Z];
fmtx->UVNZ[X] = -fmtx->UVZ[X];
fmtx->UVNZ[Y] = -fmtx->UVZ[Y];
fmtx->UVNZ[Z] = -fmtx->UVZ[Z];

//You know a really funny bug?
//Transforming the radius instead of using it as the per-axis scalar....
fmtx->Xplus[X] = fxm(fmtx->UVX[X], fmtx->radius[X]);
fmtx->Xplus[Y] = fxm(fmtx->UVX[Y], fmtx->radius[X]);
fmtx->Xplus[Z] = fxm(fmtx->UVX[Z], fmtx->radius[X]);
fmtx->Yplus[X] = fxm(fmtx->UVY[X], fmtx->radius[Y]);
fmtx->Yplus[Y] = fxm(fmtx->UVY[Y], fmtx->radius[Y]);
fmtx->Yplus[Z] = fxm(fmtx->UVY[Z], fmtx->radius[Y]);
fmtx->Zplus[X] = fxm(fmtx->UVZ[X], fmtx->radius[Z]);
fmtx->Zplus[Y] = fxm(fmtx->UVZ[Y], fmtx->radius[Z]);
fmtx->Zplus[Z] = fxm(fmtx->UVZ[Z], fmtx->radius[Z]);

fmtx->Xneg[X] = -fmtx->Xplus[X];
fmtx->Xneg[Y] = -fmtx->Xplus[Y];
fmtx->Xneg[Z] = -fmtx->Xplus[Z];
fmtx->Yneg[X] = -fmtx->Yplus[X];
fmtx->Yneg[Y] = -fmtx->Yplus[Y];
fmtx->Yneg[Z] = -fmtx->Yplus[Z];
fmtx->Zneg[X] = -fmtx->Zplus[X];
fmtx->Zneg[Y] = -fmtx->Zplus[Y];
fmtx->Zneg[Z] = -fmtx->Zplus[Z];
	
}

void	standing_surface_alignment(FIXED * surface_normal)
{
	
	pl_RBB.status[0] = 'A'; //"A" for "Needs Adjustment"
/*

Fuck this shit...

*/

static int plane_matrix[XYZ][XYZ];

//Zero-out plane matrix
zero_matrix(plane_matrix[0]);

plane_matrix[Y][X] = surface_normal[X];
plane_matrix[Y][Y] = surface_normal[Y];
plane_matrix[Y][Z] = surface_normal[Z];
int used_angle = you.rot2[Y];

// short colr1;
// short colr2;
// short colr3;

int rruY[3] = {0, 1<<16, 0};

/*

The following code chunk generates axis-aligned unit matrix data from the normal.
It does so with respect to the major axis of the normal and its sign.

*/
// Okay, listen.
// I want it to first follow/make the Z axis, not the X axis, since the Z axis is the one we actually walk towards.
// It can help you always go up, instead of at some angle left or right.
// But I don't have the time to 'fix' that right now. Of course, it is a non-issue. 
if(JO_ABS(surface_normal[Y]) > 32768)
{
	if(surface_normal[Y] < 0)
	{
		//Find the X axis of the floor's matrix (from rotating the Y axis by Z+90)
		fxrotZ(plane_matrix[Y], plane_matrix[X], 16834);
		//To find the Z axis, first we must axis-align the X axis. We do this by ensuring it has no Z.
		plane_matrix[X][Z] = 0;
		//Now the Z axis should be the cross product of Y axis and X axis
		fxcross(plane_matrix[X], plane_matrix[Y], plane_matrix[Z]);
		used_angle = (you.ladder) ? (you.rot2[Y]) : (you.rot[Y]);
		//colr3 = 1;
	} else {
		//Find the X axis of the floor's matrix (from rotating the Y axis by Z+270)
		fxrotZ(plane_matrix[Y], plane_matrix[X], 49152);
		//To find the Z axis, first we must axis-align the X axis. We do this by ensuring it has no Z.
		plane_matrix[X][Z] = 0;
		//Now the Z axis should be the cross product of Y axis and X axis
		fxcross(plane_matrix[X], plane_matrix[Y], plane_matrix[Z]);
		used_angle = (you.ladder) ? (you.rot2[Y]) : (you.rot[Y]);
		//colr3 = 10;
	}

	//colr1 = 7;
	//colr2 = 15;
} else if(JO_ABS(surface_normal[X]) > 32768)
{
	//X branch
	//The X axis can be found with the same rule.

	if(surface_normal[X] > 0)
	{
		fxrotZ(plane_matrix[Y], plane_matrix[X], 16384);
		plane_matrix[X][Z] = 0; //(Axis-alignment)
		fxcross(plane_matrix[X], plane_matrix[Y], plane_matrix[Z]);
		used_angle = (you.ladder) ? (you.rot2[Y] + 16384) : you.rot[Y];
		//colr3 = 1;
	} else {
		fxrotZ(plane_matrix[Y], plane_matrix[X], -16384);
		plane_matrix[X][Z] = 0; //(Axis-alignment)
		fxcross(plane_matrix[X], plane_matrix[Y], plane_matrix[Z]);
		used_angle = (you.ladder) ? (you.rot2[Y] + 16384) : (you.rot[Y] + 32768);
		//colr3 = 10;
	}
	
	//colr1 = 23;
	//colr2 = 31;
} else {
	//(Z branch)
	//The X axis is instead found with a Y rotation, instead of a Z rotation.

	if(surface_normal[Z] > 0)
	{
		fxrotY(plane_matrix[Y], plane_matrix[X], 16384);
		plane_matrix[X][Y] = 0; //(Axis-alignment)
		fxcross(plane_matrix[X], plane_matrix[Y], plane_matrix[Z]);
		//colr3 = 1;
		used_angle = (you.ladder) ? (you.rot2[Y]) : you.rot[Y];
	} else {
		fxrotY(plane_matrix[Y], plane_matrix[X], 16384);
		plane_matrix[X][Y] = 0; //(Axis-alignment)
		fxcross(plane_matrix[X], plane_matrix[Y], plane_matrix[Z]);
		used_angle = (you.ladder) ? (you.rot2[Y]) : (you.rot[Y] + 32768);
		//colr3 = 10;
	}

	//colr1 = 39;
	//colr2 = 47;
}

accurate_normalize(plane_matrix[X], plane_matrix[X], 5);
accurate_normalize(plane_matrix[Z], plane_matrix[Z], 5);

//Use an axis-relative rotation.
/**
Special Note:
NO ONE WILL TELL YOU THIS, BUT...
Rotation-about-axis (fxRotLocalAxis), and the math associated with it, uses a **MATRIX-SPACE** axis.
That means if you put (1, 0, 0) as the axis, it does NOT rotate about GLOBAL (1, 0, 0).
It rotates about (1, 0, 0) COMMUTED THROUGH THE MATRIX, e.g. the matrix' local X axis.
It may seem obvious to some, but if you want to rotate about the global X axis, you just rotate by X.
Same for Y and Z, et cetera.
The axis input into this function is relative to the axis itself.
In this case, fwdMtx[Y] is {0, 1, 0}, meaning the matrix' Y axis is what I want to rotate around.
**/
fxRotLocalAxis(plane_matrix[0], rruY, used_angle);

aMtx[X][X] = plane_matrix[X][X];
aMtx[X][Y] = plane_matrix[X][Y];
aMtx[X][Z] = plane_matrix[X][Z];
aMtx[Y][X] = -plane_matrix[Y][X];
aMtx[Y][Y] = -plane_matrix[Y][Y];
aMtx[Y][Z] = -plane_matrix[Y][Z];
aMtx[Z][X] = -plane_matrix[Z][X];
aMtx[Z][Y] = -plane_matrix[Z][Y];
aMtx[Z][Z] = -plane_matrix[Z][Z];

//finalize_alignment(&pl_RBB);

// nbg_sprintf(1, 6, "x(%i)", surface_normal[X]);
// nbg_sprintf(1, 7, "y(%i)", surface_normal[Y]);
// nbg_sprintf(1, 8, "z(%i)", surface_normal[Z]);

// nbg_sprintf(13, 6, "x(%i)", rrX[X]);
// nbg_sprintf(13, 7, "y(%i)", rrX[Y]);
// nbg_sprintf(13, 8, "z(%i)", rrX[Z]);

// static short drawposA[3];
// static int 	drawposC[3];
// static short drawposE[3];
// static short drawposF[3];

// drawposC[X] = pl_RBB.Yplus[X] - you.pos[X];
// drawposC[Y] = pl_RBB.Yplus[Y] - you.pos[Y];
// drawposC[Z] = pl_RBB.Yplus[Z] - you.pos[Z];

// drawposA[X] = (plane_matrix[Y][X]>>2); 
// drawposA[Y] = (plane_matrix[Y][Y]>>2); 
// drawposA[Z] = (plane_matrix[Y][Z]>>2); 

// drawposE[X] = (plane_matrix[X][X]>>2); 
// drawposE[Y] = (plane_matrix[X][Y]>>2); 
// drawposE[Z] = (plane_matrix[X][Z]>>2); 
				  
// drawposF[X] = (plane_matrix[Z][X]>>2); 
// drawposF[Y] = (plane_matrix[Z][Y]>>2); 
// drawposF[Z] = (plane_matrix[Z][Z]>>2); 

}


void	set_from_this_normal(Uint8 normID, _boundBox stator, VECTOR setNormal)
{
	
	//nbg_sprintf(12, 9, "(%i)", normID);
	
	if(normID == N_Xp){
		setNormal[X] = stator.UVX[X];
		setNormal[Y] = stator.UVX[Y];
		setNormal[Z] = stator.UVX[Z];
	} else if(normID == N_Xn){
		setNormal[X] = stator.UVNX[X];
		setNormal[Y] = stator.UVNX[Y];
		setNormal[Z] = stator.UVNX[Z];
	} else if(normID == N_Yp){
		setNormal[X] = stator.UVY[X];
		setNormal[Y] = stator.UVY[Y];
		setNormal[Z] = stator.UVY[Z];
	} else if(normID == N_Yn){
		setNormal[X] = stator.UVNY[X];
		setNormal[Y] = stator.UVNY[Y];
		setNormal[Z] = stator.UVNY[Z];
	} else if(normID == N_Zp){
		setNormal[X] = stator.UVZ[X];
		setNormal[Y] = stator.UVZ[Y];
		setNormal[Z] = stator.UVZ[Z];
	} else if(normID == N_Zn){
		setNormal[X] = stator.UVNZ[X];
		setNormal[Y] = stator.UVNZ[Y];
		setNormal[Z] = stator.UVNZ[Z];
	}

}

int		hitscan_vector_from_position_box(int * ray_normal, int * ray_pos, int * hit, int * hitNormal, _boundBox * box)
{

	static POINT trash = {0, 0, 0};
	static POINT possible_hit = {0,0,0};
	int hasHit = 0;
	//Box Populated Check
	if(box->status[1] != 'C')
	{
		return 0;
	}
	
	//If the collision proxy is not ready for this frame, make it.
	if(box->status[3] != 'B')
	{
		finalize_collision_proxy(box);
	}

	for(int i = 0; i < 6; i++)
	{
   		//Backfacing Faces
		if(fxdot(box->nmtbl[i], ray_normal) > 0) continue;
		//Drawing lines to face
		
		ray_to_plane(ray_normal, ray_pos, box->nmtbl[i], box->pltbl[i][0], possible_hit);
		
		//It can almost work without this. Might be an investigation point to find another shortcut.
		trash[X] = ray_pos[X] - possible_hit[X];
		trash[Y] = ray_pos[Y] - possible_hit[Y];
		trash[Z] = ray_pos[Z] - possible_hit[Z];
		int scale_to_phit = fxdot(trash, ray_normal);
		if(scale_to_phit > 0) continue;
		
		if(edge_wind_test(box->pltbl[i][0], box->pltbl[i][1], box->pltbl[i][2], box->pltbl[i][3], possible_hit, box->maxtbl[i], 12))
		{
				//Distance test "possible hit" to "ray_pos"
				//Can be very far, must use integers.
				trash[X] = (possible_hit[X] - ray_pos[X])>>16;
				trash[Y] = (possible_hit[Y] - ray_pos[Y])>>16;
				trash[Z] = (possible_hit[Z] - ray_pos[Z])>>16;
				unsigned int possible_hit_scale = (trash[X] * trash[X]) + (trash[Y] * trash[Y]) + (trash[Z] * trash[Z]);
				trash[X] = (hit[X] - ray_pos[X])>>16;
				trash[Y] = (hit[Y] - ray_pos[Y])>>16;
				trash[Z] = (hit[Z] - ray_pos[Z])>>16;
				unsigned int hit_scale = (trash[X] * trash[X]) + (trash[Y] * trash[Y]) + (trash[Z] * trash[Z]);
				if(possible_hit_scale < hit_scale)
				{
					hit[X] = possible_hit[X];
					hit[Y] = possible_hit[Y];
					hit[Z] = possible_hit[Z];
					hitNormal[X] = box->nmtbl[i][X];
					hitNormal[Y] = box->nmtbl[i][Y];
					hitNormal[Z] = box->nmtbl[i][Z];
					hasHit = 1;
				}
		} 
	}
	return hasHit;
}

void	pl_physics_handler(_boundBox * mover, _boundBox * stator, _lineTable * moverTimeAxis, POINT hitPt, short faceIndex, short obj_type_data)
{
	/*
	
Floor collisions pass the Boolean "hitSurface" that is processed in player_phy.c
Wall collisions pass the Boolean "hitWall" that is processed in player_phy.c
	
	*/
	if((obj_type_data & ETYPE) == OBJECT && (obj_type_data & OBJECT_TYPE) == LADDER_OBJECT)
	{
		//
		// This is strictly in case of ladder.
		// If climbable, we just don't set ladder as true, and don't restrict the Y rotation.
		//
		you.climbing = true;
		you.ladder = true;
		if(slCos(you.rot2[Y]) >= 0)
		{
			you.rot2[Y] = 0;
		} else {
			you.rot2[Y] = 32768;
		}
	} else if((obj_type_data & ETYPE) == OBJECT && (obj_type_data & OBJECT_TYPE) == CLIMB_OBJECT)
	{
		you.climbing = true;
	}


	if(stator->nmtbl[faceIndex][Y] < -32768 || you.climbing)
	{
		//If we were going to stand on this surface anyway, un-flag climbing; we can just stand.
		if(stator->nmtbl[faceIndex][Y] < -49152) you.climbing = false;
		you.floorNorm[X] = stator->nmtbl[faceIndex][X];
		you.floorNorm[Y] = stator->nmtbl[faceIndex][Y];
		you.floorNorm[Z] = stator->nmtbl[faceIndex][Z];
		
		standing_surface_alignment(you.floorNorm);
			
		you.floorPos[X] = -(hitPt[X]) - moverTimeAxis->yp1[X];
		you.floorPos[Y] = -(hitPt[Y]) - moverTimeAxis->yp1[Y];
		you.floorPos[Z] = -(hitPt[Z]) - moverTimeAxis->yp1[Z];
		you.shadowPos[X] = -hitPt[X];
		you.shadowPos[Y] = -hitPt[Y];
		you.shadowPos[Z] = -hitPt[Z];
			
		you.aboveObject = true;
		you.hitSurface = true;
	} else {
		you.wallNorm[X] = stator->nmtbl[faceIndex][X];
		you.wallNorm[Y] = stator->nmtbl[faceIndex][Y];
		you.wallNorm[Z] = stator->nmtbl[faceIndex][Z];
		you.wallPos[X] = hitPt[X];
		you.wallPos[Y] = hitPt[Y];
		you.wallPos[Z] = hitPt[Z];
			
		you.hitWall = true;
	}
}

void	player_shadow_object(_boundBox * stator, POINT centerDif)
{
	
	//Again, I have no idea how my coordinate systems get so reliably inverted...
	POINT below_player = {you.wpos[X], (you.wpos[Y] - (1<<16)), you.wpos[Z]};
	POINT xHit;
	POINT yHit;
	POINT zHit;
	int hitBools[XYZ];
	POINT highHit = {0, 0, 0};

	if(fxdot(centerDif, stator->UVX) < 0){
		//This means we draw to X+
		line_hit_plane_here(you.wpos, below_player, stator->Xplus, stator->UVX, stator->pos, 16384, xHit);
	} else {
		//This means we draw to X-
		line_hit_plane_here(you.wpos, below_player, stator->Xneg, stator->UVX, stator->pos, 16384, xHit);
	}
	
	if(fxdot(centerDif, stator->UVY) < 0){
		//This means we draw to Y+
		line_hit_plane_here(you.wpos, below_player, stator->Yplus, stator->UVY, stator->pos, 16384, yHit);
	} else {
		//This means we draw to Y-
		line_hit_plane_here(you.wpos, below_player, stator->Yneg, stator->UVY, stator->pos, 16384, yHit);
	}
	
	if(fxdot(centerDif, stator->UVZ) < 0){
		//This means we draw to Z+
		line_hit_plane_here(you.wpos, below_player, stator->Zplus, stator->UVZ, stator->pos, 16384, zHit);
	} else {
		//This means we draw to Z-
		line_hit_plane_here(you.wpos, below_player, stator->Zneg, stator->UVZ, stator->pos, 16384, zHit);
	}
	
	if(simple_collide(xHit, stator) == true){
		hitBools[X] = true;
		highHit[X] = xHit[X];
		highHit[Y] = xHit[Y];
		highHit[Z] = xHit[Z];
	} else {
		hitBools[X] = false;
	}
	
	if(simple_collide(yHit, stator) == true){
		hitBools[Y] = true;
		highHit[X] = (JO_ABS(yHit[Y]) > highHit[Y]) ? yHit[X] : highHit[X];
		highHit[Y] = (JO_ABS(yHit[Y]) > highHit[Y]) ? yHit[Y] : highHit[Y];
		highHit[Z] = (JO_ABS(yHit[Y]) > highHit[Y]) ? yHit[Z] : highHit[Z];
	} else {
		hitBools[Y] = false;
	}
	
	if(simple_collide(zHit, stator) == true){
		hitBools[Z] = true;
		highHit[X] = (JO_ABS(zHit[Y]) > highHit[Y]) ? zHit[X] : highHit[X];
		highHit[Y] = (JO_ABS(zHit[Y]) > highHit[Y]) ? zHit[Y] : highHit[Y];
		highHit[Z] = (JO_ABS(zHit[Y]) > highHit[Y]) ? zHit[Z] : highHit[Z];
	} else {
		hitBools[Z] = false;
	}
	
			// slPrintFX(highHit[X], slLocate(9, 7));
			// slPrintFX(highHit[Y], slLocate(19, 7));
			// slPrintFX(highHit[Z], slLocate(29, 7));
	
	if((hitBools[X] == true || hitBools[Y] == true || hitBools[Z] == true) && you.pos[Y] > highHit[Y])
	{
		//Inverted coordinates...
		you.shadowPos[X] = -highHit[X];
		you.shadowPos[Y] = -highHit[Y];
		you.shadowPos[Z] = -highHit[Z];
		you.aboveObject = true;
	}

}

Bool	player_collide_boxes(_boundBox * stator, _boundBox * mover, _lineTable * moverCFs, _lineTable * moverTimeAxis, short obj_type_data)
{

static FIXED bigRadius = 0;

static POINT centerDif = {0, 0, 0};

static POINT lineEnds[9];

static Bool lineChecks[9];
		
static FIXED bigDif = 0;


//Box Populated Check
if(stator->status[1] != 'C')
{
	return false;
}


//Box Distance Culling Check
bigRadius = JO_MAX(JO_MAX(JO_MAX(JO_MAX(JO_ABS(stator->Xplus[X]), JO_ABS(stator->Xplus[Y])), JO_ABS(stator->Xplus[Z])),
		JO_MAX(JO_MAX(JO_ABS(stator->Yplus[X]), JO_ABS(stator->Yplus[Y])), JO_ABS(stator->Yplus[Z]))),
		JO_MAX(JO_MAX(JO_ABS(stator->Zplus[X]), JO_ABS(stator->Zplus[Y])), JO_ABS(stator->Zplus[Z])));
		

centerDif[X] = stator->pos[X] + mover->pos[X];
centerDif[Y] = stator->pos[Y] + mover->pos[Y];
centerDif[Z] = stator->pos[Z] + mover->pos[Z];


bigDif = JO_MAX(JO_MAX(JO_ABS(centerDif[X]), JO_ABS(centerDif[Y])),JO_ABS(centerDif[Z]));
	//Shadow projector has to happen before radius cut.
	if(centerDif[X] < bigRadius && centerDif[Z] < bigRadius)
	{
	player_shadow_object(stator, centerDif);
	}
if(bigDif > (bigRadius<<1)) return false;

//If the collision proxy is not ready for this frame, make it.
if(stator->status[3] != 'B')
{
	finalize_collision_proxy(stator);
}

numBoxChecks++;

	for(int i = 0; i < 6; i++)
	{
   		//Backfacing Faces
		if(fxdot(centerDif, stator->nmtbl[i]) > 0) continue;
		//Drawing lines to face
		
		lineChecks[X] = line_hit_plane_here(moverCFs->xp0, moverCFs->xp1, stator->cftbl[i],
										stator->nmtbl[i], stator->pos, 16384, lineEnds[X]);
		lineChecks[Y] = line_hit_plane_here(moverCFs->yp0, moverCFs->yp1, stator->cftbl[i],
										stator->nmtbl[i], stator->pos, 65536, lineEnds[Y]);
		lineChecks[Z] = line_hit_plane_here(moverCFs->zp0, moverCFs->zp1, stator->cftbl[i],
										stator->nmtbl[i], stator->pos, 16384, lineEnds[Z]);
		for(int u = 0; u < 3; u++)
		{
			if(lineChecks[u])
			{
				if(edge_wind_test(stator->pltbl[i][0], stator->pltbl[i][1], stator->pltbl[i][2], stator->pltbl[i][3], lineEnds[u], stator->maxtbl[i], 12))
				{
					pl_physics_handler(mover, stator, moverTimeAxis, lineEnds[u], i, obj_type_data);
					mover->collisionID = stator->boxID;
					stator->collisionID = mover->boxID;
					you.hitBox = true;
					return true;
				} 
			}
		}  
	}

	return false;
}


int		broad_phase_sector_finder(int * pos, int * mesh_position, _sector * test_sector)
{
	entity_t * ent = test_sector->ent;
	//If the entity is not loaded, cease the test.
	if(ent->file_done != true) return INVALID_SECTOR;
	if(ent->type != MODEL_TYPE_SECTORED) return INVALID_SECTOR;
	//This is the initial phase of determining which sector something is in.
	//Once we have passed this broad phase, we can find out which sector something is in by less complex means;
	//mostly by using data from collision processing an actor or the player.
	//This will be very slow on large levels. So do not use this blindly.
	//(... proceeds to use blindly...)
	
	//Method:
	//Do a line of sight test from the test position, with the sight vector being down.
	//Generally, the idea works that all sectors will have floors, but not all sectors will have ceilings.
	//Bonus: We could do this more efficiently if we did the chirality test first, before the raycast.
	//Meh.
	GVPLY * mesh = ent->pol;
	static int testDirection[3] = {0, 65535, 0};
	static int pHit[3] = {0, 0, 0};
	int wdist[3] = {0,0,0};
	static int hitPolyID = 0;

	//Must re-set this variable in order for the hitscan function to be able to properly filter the nearest hit
	pHit[X] = 32766<<16;
	pHit[Y] = 32766<<16;
	pHit[Z] = 32766<<16;
	int abovePolygon = hitscan_vector_from_position_building(testDirection, pos, pHit, &hitPolyID, ent, mesh_position, test_sector);
		
	//Sectors do not usually overlap each other on the vertical axis, and when they do,
	//it is normally not through a vertical, traversable gap (i.e. stairs that have same-sector polygons underneath them).
	//Yet, it is a pretty huge pain to design around a strict constraint like that.
	//So this represented error state has been resolved by instead always checking sectors near the sector being tested if within radius.
		
	//if(!abovePolygon)
	//{
		//If we were not in that sector, we need to instead check all sectors visible from that sector.
		for(int i = 0; i < test_sector->nbVisible; i++)
		{
			_sector * sct = &sectors[test_sector->pvs[i]];
			
			//Broad-phase: Check radius
			wdist[X] = JO_ABS(pos[X] - (mesh_position[X] - sct->center_pos[X]));
			wdist[Y] = JO_ABS(pos[Y] - (mesh_position[Y] - sct->center_pos[Y]));
			wdist[Z] = JO_ABS(pos[Z] - (mesh_position[Z] - sct->center_pos[Z]));
			
			if(wdist[X] < sct->radius[X] && wdist[Y] < sct->radius[Y] && wdist[Z] < sct->radius[Z])
			{

				abovePolygon += hitscan_vector_from_position_building(testDirection, pos, pHit, &hitPolyID, ent, mesh_position, sct);
				//if(abovePolygon) break;
			}
		}
	//}
	
	//If we are STILL not above a polygon...
	if(!abovePolygon) return INVALID_SECTOR;
	
	return mesh->attbl[hitPolyID].first_sector;
		
	
}


void	player_collision_test_loop(void)
{
	//This runs the physics workhorse stuff, but a lot of this logic is in physobjet.c -
	//much of it has as much to do with game state as it does physics.
	
	you.hitObject = false;
	if(ldata_ready != true) return; //Just in case.
	int boxType;
	int edata;
	static int hitscanPly = 0;
	
	//First, find the player's sector and build sector lists.
	//Special note: The collision system is using next-frame position, so the sector system must also use next-frame position.
	you.curSector = broad_phase_sector_finder(you.realTimeAxis.yp1, levelPos, &sectors[you.curSector]);
	//This is also the moment where we should build the adjacent / visible sector list.
	//Use the current sector's adjacent list as the draw list.
	_sector * sct = &sectors[you.curSector];
	nearSectorCt = sct->nbVisible;
	
	collide_in_sector_of_entity(sct->ent, sct, &you.box, &you.realTimeAxis);

	//for(unsigned int s = 0; s < (sct->nbAdjacent+1); s++)
	//{
		// you.hasValidAim += hitscan_vector_from_position_building(you.uview, you.viewPos, you.hitscanPt, &hitscanPly, sct->ent, levelPos, &sectors[sct->pvs[s]]);
		// you.hitscanNm[X] = sct->ent->pol->nmtbl[hitscanPly][X];
		// you.hitscanNm[Y] = sct->ent->pol->nmtbl[hitscanPly][Y];
		// you.hitscanNm[Z] = sct->ent->pol->nmtbl[hitscanPly][Z];
	//}
	
	
	for(int i = 0; i < MAX_PHYS_PROXY; i++)
	{
		//nbg_sprintf(0, 0, "(PHYS)"); //Debug ONLY
		if(RBBs[i].status[1] != 'C') continue;
		edata = dWorldObjects[activeObjects[i]].type.ext_dat;
		boxType = edata & (0xF000);
		//Check if object # is a collision-approved type
		
		switch(boxType)
		{
			case(OBJPOP):
			case(SPAWNER):
			//you.hasValidAim += hitscan_vector_from_position_box(you.uview, you.viewPos, you.hitscanPt, you.hitscanNm, &RBBs[i]);
			player_collide_boxes(&RBBs[i], &pl_RBB, &you.bwd_world_faces, &you.time_axis, edata);
			subtype_collision_logic(&dWorldObjects[activeObjects[i]], &RBBs[i], &pl_RBB);
			break;
			case(ITEM | OBJPOP):
			item_collision(i, &pl_RBB);
			break;
			case(BUILD | OBJPOP):
			//you.hasValidAim += hitscan_vector_from_position_building(you.uview, you.viewPos, you.hitscanPt, &hitscanPly, &entities[dWorldObjects[activeObjects[i]].type.entity_ID], RBBs[i].pos, NULL);
			//you.hitscanNm[X] = entities[dWorldObjects[activeObjects[i]].type.entity_ID].pol->nmtbl[hitscanPly][X];
			//you.hitscanNm[Y] = entities[dWorldObjects[activeObjects[i]].type.entity_ID].pol->nmtbl[hitscanPly][Y];
			//you.hitscanNm[Z] = entities[dWorldObjects[activeObjects[i]].type.entity_ID].pol->nmtbl[hitscanPly][Z];

			collide_per_polygon_of_mesh(&entities[dWorldObjects[activeObjects[i]].type.entity_ID], &you.box, &you.realTimeAxis);
			if(you.hitObject  == true)
			{
				RBBs[i].collisionID = pl_RBB.boxID;
				pl_RBB.collisionID = RBBs[i].boxID;
			}
			break;
			default:
			break;
		}
	
	}
	
	ldata_manager();

//	nbg_sprintf(0, 14, "(%i)E", numBoxChecks);
	numBoxChecks = 0;
	
}


