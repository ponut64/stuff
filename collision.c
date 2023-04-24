/*
This file is compiled separately.
*/
///collision.c

#include <sl_def.h>
#include "def.h"
#include "mymath.h"
#include "bounder.h"
#include "mloader.h"
#include "render.h"
#include "physobjet.h"
#include "object_col.h"
#include "ldata.h"

#include "collision.h"

int numBoxChecks = 0;

int boxPointField[8][3];
int * boxNormField[6];
int * boxPolyField[6][4];
int * boxCenterField[6];
int boxDisField[6];

void	init_box_handling(void)
{
	
/*
			Y-		  Z+
		3-----------2
	  /	|		   /|
	0---+--------1  |
X-	|	|	     |	|	X+
	|	7--------+--6
	| / 		 |/
	4------------5 
	Z-		Y+
	Verts
	0 = X- Y- Z-
	1 = X+ Y- Z-
	2 = X+ Y- Z+
	3 = X- Y- Z+
	4 = X- Y+ Z-
	5 = X+ Y+ Z-
	6 = X+ Y+ Z+
	7 = X- Y+ Z+
	Polygons:
	0: 3 - 2 - 1 - 0, normal Y-
	1: 0 - 1 - 5 - 4, normal Z-
	2: 0 - 4 - 7 - 3, normal X-
	3: 4 - 5 - 6 - 7, normal Y+
	4: 3 - 7 - 6 - 2, normal Z+
	5: 1 - 2 - 6 - 5, normal X+

*/
	
boxDisField[0] = N_Yn;
boxDisField[1] = N_Zn;
boxDisField[2] = N_Xn;
boxDisField[3] = N_Yp;
boxDisField[4] = N_Zp;
boxDisField[5] = N_Xp;


boxPolyField[0][0] = boxPointField[3];	
boxPolyField[0][1] = boxPointField[2];
boxPolyField[0][2] = boxPointField[1];
boxPolyField[0][3] = boxPointField[0];

boxPolyField[1][0] = boxPointField[0];	
boxPolyField[1][1] = boxPointField[1];
boxPolyField[1][2] = boxPointField[5];
boxPolyField[1][3] = boxPointField[4];

boxPolyField[2][0] = boxPointField[0];	
boxPolyField[2][1] = boxPointField[4];
boxPolyField[2][2] = boxPointField[7];
boxPolyField[2][3] = boxPointField[3];

boxPolyField[3][0] = boxPointField[4];	
boxPolyField[3][1] = boxPointField[5];
boxPolyField[3][2] = boxPointField[6];
boxPolyField[3][3] = boxPointField[7];

boxPolyField[4][0] = boxPointField[3];	
boxPolyField[4][1] = boxPointField[7];
boxPolyField[4][2] = boxPointField[6];
boxPolyField[4][3] = boxPointField[2];

boxPolyField[5][0] = boxPointField[1];	
boxPolyField[5][1] = boxPointField[2];
boxPolyField[5][2] = boxPointField[6];
boxPolyField[5][3] = boxPointField[5];
	
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
fmtx->Xplus[X] = fxm(fmtx->UVX[X], fmtx->brad[X]);
fmtx->Xplus[Y] = fxm(fmtx->UVX[Y], fmtx->brad[X]);
fmtx->Xplus[Z] = fxm(fmtx->UVX[Z], fmtx->brad[X]);
fmtx->Yplus[X] = fxm(fmtx->UVY[X], fmtx->brad[Y]);
fmtx->Yplus[Y] = fxm(fmtx->UVY[Y], fmtx->brad[Y]);
fmtx->Yplus[Z] = fxm(fmtx->UVY[Z], fmtx->brad[Y]);
fmtx->Zplus[X] = fxm(fmtx->UVZ[X], fmtx->brad[Z]);
fmtx->Zplus[Y] = fxm(fmtx->UVZ[Y], fmtx->brad[Z]);
fmtx->Zplus[Z] = fxm(fmtx->UVZ[Z], fmtx->brad[Z]);

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
		used_angle = (you.rot[Y]);
		//colr3 = 1;
	} else {
		//Find the X axis of the floor's matrix (from rotating the Y axis by Z+270)
		fxrotZ(plane_matrix[Y], plane_matrix[X], 49152);
		//To find the Z axis, first we must axis-align the X axis. We do this by ensuring it has no Z.
		plane_matrix[X][Z] = 0;
		//Now the Z axis should be the cross product of Y axis and X axis
		fxcross(plane_matrix[X], plane_matrix[Y], plane_matrix[Z]);
		used_angle = (you.rot2[Y]);
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
		used_angle = (you.rot2[Y] + 16384);
		//colr3 = 1;
	} else {
		fxrotZ(plane_matrix[Y], plane_matrix[X], -16384);
		plane_matrix[X][Z] = 0; //(Axis-alignment)
		fxcross(plane_matrix[X], plane_matrix[Y], plane_matrix[Z]);
		used_angle = (you.rot2[Y] + 16384);
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
		used_angle = (you.rot2[Y]);
	} else {
		fxrotY(plane_matrix[Y], plane_matrix[X], 16384);
		plane_matrix[X][Y] = 0; //(Axis-alignment)
		fxcross(plane_matrix[X], plane_matrix[Y], plane_matrix[Z]);
		used_angle = (you.rot2[Y]);
		//colr3 = 10;
	}

	//colr1 = 39;
	//colr2 = 47;
}

accurate_normalize(plane_matrix[X], plane_matrix[X]);
accurate_normalize(plane_matrix[Z], plane_matrix[Z]);

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

// add_to_sprite_list(drawposC, drawposE, 0, colr1, 0, 'L', 0, 2184);
// add_to_sprite_list(drawposC, drawposF, 0, colr2, 0, 'L', 0, 2184);
// add_to_sprite_list(drawposC, drawposA, 0, colr3, 0, 'L', 0, 2184);


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

void	pl_physics_handler(_boundBox * mover, POINT hitPt, short faceIndex, short obj_type_data)
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


	if(boxNormField[faceIndex][Y] < -32768 || you.climbing)
	{
		//If we were going to stand on this surface anyway, un-flag climbing; we can just stand.
		if(boxNormField[faceIndex][Y] < -49152) you.climbing = false;
		you.floorNorm[X] = boxNormField[faceIndex][X];
		you.floorNorm[Y] = boxNormField[faceIndex][Y];
		you.floorNorm[Z] = boxNormField[faceIndex][Z];
		
		standing_surface_alignment(you.floorNorm);
			
		you.floorPos[X] = -(hitPt[X]) - (mover->Yneg[X]);
		you.floorPos[Y] = -(hitPt[Y]) - (mover->Yneg[Y]);
		you.floorPos[Z] = -(hitPt[Z]) - (mover->Yneg[Z]);
		you.shadowPos[X] = -hitPt[X];
		you.shadowPos[Y] = -hitPt[Y];
		you.shadowPos[Z] = -hitPt[Z];
			
		you.aboveObject = true;
		you.hitSurface = true;
	} else {
		you.wallNorm[X] = boxNormField[faceIndex][X];
		you.wallNorm[Y] = boxNormField[faceIndex][Y];
		you.wallNorm[Z] = boxNormField[faceIndex][Z];
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

Bool	player_collide_boxes(_boundBox * stator, _boundBox * mover, short obj_type_data)
{

static FIXED bigRadius = 0;

static POINT centerDif = {0, 0, 0};

static POINT lineEnds[9];

static Bool lineChecks[9];
		
static FIXED bigDif = 0;


//Box Populated Check
if(stator->status[1] != 'C'){
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
if(bigDif > (bigRadius + (20<<16))) return false;



numBoxChecks++;

//Box Collision Check
_lineTable moverCFs = {
	.xp0[X] = mover->Xplus[X] 	- mover->pos[X],// - mover->velocity[X],
	.xp0[Y] = mover->Xplus[Y] 	- mover->pos[Y],// - mover->velocity[Y],
	.xp0[Z] = mover->Xplus[Z] 	- mover->pos[Z],// - mover->velocity[Z],
	.xp1[X] = mover->Xneg[X] 	- mover->pos[X],// + mover->velocity[X],
	.xp1[Y] = mover->Xneg[Y] 	- mover->pos[Y],// + mover->velocity[Y],
	.xp1[Z] = mover->Xneg[Z] 	- mover->pos[Z],// + mover->velocity[Z],
	.yp0[X] = mover->Yplus[X] 	- mover->pos[X],// - mover->velocity[X],
	.yp0[Y] = mover->Yplus[Y] 	- mover->pos[Y],// - mover->velocity[Y],
	.yp0[Z] = mover->Yplus[Z] 	- mover->pos[Z],// - mover->velocity[Z],
	.yp1[X] = mover->Yneg[X] 	- mover->pos[X],// + mover->velocity[X],
	.yp1[Y] = mover->Yneg[Y] 	- mover->pos[Y],// + mover->velocity[Y],
	.yp1[Z] = mover->Yneg[Z] 	- mover->pos[Z],// + mover->velocity[Z],
	.zp0[X] = mover->Zplus[X] 	- mover->pos[X],// - mover->velocity[X],
	.zp0[Y] = mover->Zplus[Y] 	- mover->pos[Y],// - mover->velocity[Y],
	.zp0[Z] = mover->Zplus[Z] 	- mover->pos[Z],// - mover->velocity[Z],
	.zp1[X] = mover->Zneg[X] 	- mover->pos[X],// + mover->velocity[X],
	.zp1[Y] = mover->Zneg[Y] 	- mover->pos[Y],// + mover->velocity[Y],
	.zp1[Z] = mover->Zneg[Z] 	- mover->pos[Z]// + mover->velocity[Z]
}; 


boxNormField[0] = stator->UVNY;
boxNormField[1] = stator->UVNZ;
boxNormField[2] = stator->UVNX;
boxNormField[3] = stator->UVY;
boxNormField[4] = stator->UVZ;
boxNormField[5] = stator->UVX;
boxCenterField[0] = stator->Yneg;
boxCenterField[1] = stator->Zneg;
boxCenterField[2] = stator->Xneg;
boxCenterField[3] = stator->Yplus;
boxCenterField[4] = stator->Zplus;
boxCenterField[5] = stator->Xplus;

boxPointField[0][X] = (stator->Xneg[X] +  stator->Yneg[X] + stator->Zneg[X]		+ stator->pos[X]);
boxPointField[0][Y] = (stator->Xneg[Y] +  stator->Yneg[Y] + stator->Zneg[Y]		+ stator->pos[Y]);
boxPointField[0][Z] = (stator->Xneg[Z] +  stator->Yneg[Z] + stator->Zneg[Z]		+ stator->pos[Z]);
boxPointField[1][X] = (stator->Xplus[X] + stator->Yneg[X] + stator->Zneg[X]		+ stator->pos[X]);
boxPointField[1][Y] = (stator->Xplus[Y] + stator->Yneg[Y] + stator->Zneg[Y]		+ stator->pos[Y]);
boxPointField[1][Z] = (stator->Xplus[Z] + stator->Yneg[Z] + stator->Zneg[Z]		+ stator->pos[Z]);
boxPointField[2][X] = (stator->Xplus[X] + stator->Yneg[X] + stator->Zplus[X]	+ stator->pos[X]);
boxPointField[2][Y] = (stator->Xplus[Y] + stator->Yneg[Y] + stator->Zplus[Y]	+ stator->pos[Y]);
boxPointField[2][Z] = (stator->Xplus[Z] + stator->Yneg[Z] + stator->Zplus[Z]	+ stator->pos[Z]);
boxPointField[3][X] = (stator->Xneg[X] +  stator->Yneg[X] + stator->Zplus[X]	+ stator->pos[X]);
boxPointField[3][Y] = (stator->Xneg[Y] +  stator->Yneg[Y] + stator->Zplus[Y]	+ stator->pos[Y]);
boxPointField[3][Z] = (stator->Xneg[Z] +  stator->Yneg[Z] + stator->Zplus[Z]	+ stator->pos[Z]);
boxPointField[4][X] = (stator->Xneg[X] +  stator->Yplus[X] + stator->Zneg[X]	+ stator->pos[X]);
boxPointField[4][Y] = (stator->Xneg[Y] +  stator->Yplus[Y] + stator->Zneg[Y]	+ stator->pos[Y]);
boxPointField[4][Z] = (stator->Xneg[Z] +  stator->Yplus[Z] + stator->Zneg[Z]	+ stator->pos[Z]);
boxPointField[5][X] = (stator->Xplus[X] + stator->Yplus[X] + stator->Zneg[X]	+ stator->pos[X]);
boxPointField[5][Y] = (stator->Xplus[Y] + stator->Yplus[Y] + stator->Zneg[Y]	+ stator->pos[Y]);
boxPointField[5][Z] = (stator->Xplus[Z] + stator->Yplus[Z] + stator->Zneg[Z]	+ stator->pos[Z]);
boxPointField[6][X] = (stator->Xplus[X] + stator->Yplus[X] + stator->Zplus[X]	+ stator->pos[X]);
boxPointField[6][Y] = (stator->Xplus[Y] + stator->Yplus[Y] + stator->Zplus[Y]	+ stator->pos[Y]);
boxPointField[6][Z] = (stator->Xplus[Z] + stator->Yplus[Z] + stator->Zplus[Z]	+ stator->pos[Z]);
boxPointField[7][X] = (stator->Xneg[X] +  stator->Yplus[X] + stator->Zplus[X]	+ stator->pos[X]);
boxPointField[7][Y] = (stator->Xneg[Y] +  stator->Yplus[Y] + stator->Zplus[Y]	+ stator->pos[Y]);
boxPointField[7][Z] = (stator->Xneg[Z] +  stator->Yplus[Z] + stator->Zplus[Z]	+ stator->pos[Z]);

	for(int i = 0; i < 6; i++)
	{
   		//Backfacing Faces
		if(fxdot(centerDif, boxNormField[i]) > 0) continue;
		//Drawing lines to face
		
		lineChecks[X] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, boxCenterField[i],
										boxNormField[i], stator->pos, 16384, lineEnds[X]);
		lineChecks[Y] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, boxCenterField[i],
										boxNormField[i], stator->pos, 65536, lineEnds[Y]);
		lineChecks[Z] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, boxCenterField[i],
										boxNormField[i], stator->pos, 16384, lineEnds[Z]);
		for(int u = 0; u < 3; u++)
		{
			if(lineChecks[u])
			{
				if(edge_wind_test(boxPolyField[i][0], boxPolyField[i][1], lineEnds[u], boxDisField[i]) > 0)
				{
					if(edge_wind_test(boxPolyField[i][1], boxPolyField[i][2], lineEnds[u], boxDisField[i]) > 0)
					{
						if(edge_wind_test(boxPolyField[i][2], boxPolyField[i][3], lineEnds[u], boxDisField[i]) > 0)
						{
							if(edge_wind_test(boxPolyField[i][3], boxPolyField[i][0], lineEnds[u], boxDisField[i]) > 0)
							{
								pl_physics_handler(mover, lineEnds[u], i, obj_type_data);
								mover->collisionID = stator->boxID;
								stator->collisionID = mover->boxID;
								you.hitBox = true;
								return true;
							}
						}
					}
				} 
			}
		}  
	}

	return false;
}

void	player_collision_test_loop(void)
{
	//This runs the physics workhorse stuff, but a lot of this logic is in physobjet.c -
	//much of it has as much to do with game state as it does physics.
	
	you.hitObject = false;
	if(ldata_ready != true) return; //Just in case.
	int skipdat;
	int edata;
	for(int i = 0; i < MAX_PHYS_PROXY; i++)
	{
		//nbg_sprintf(0, 0, "(PHYS)"); //Debug ONLY
		if(RBBs[i].status[1] != 'C') continue;
		edata = dWorldObjects[activeObjects[i]].type.ext_dat;
		skipdat = edata & (0xF000);
		if( skipdat == OBJPOP ){ //Check if object # is a collision-approved type
				player_collide_boxes(&RBBs[i], &pl_RBB, edata);
				subtype_collision_logic(&dWorldObjects[activeObjects[i]], &RBBs[i], &pl_RBB);
			} else if(skipdat == (ITEM | OBJPOP)) {
				item_collision(i, &pl_RBB);
			} else if(skipdat == (GATE_R | OBJPOP)) {
				test_gate_ring(i, &pl_RBB);
			} else if(skipdat == (GATE_P | OBJPOP)) {
				test_gate_posts(activeObjects[i], &pl_RBB);
				if(entities[dWorldObjects[activeObjects[i]].type.entity_ID].type == MODEL_TYPE_BUILDING)
				{
					//If it was loaded as a building-type object, collision test it as such.
					per_poly_collide(&entities[dWorldObjects[activeObjects[i]].type.entity_ID], &pl_RBB, RBBs[i].pos);
					if(you.hitObject == true){
						RBBs[i].collisionID = pl_RBB.boxID;
						pl_RBB.collisionID = RBBs[i].boxID;
					}
				} else {
					player_collide_boxes(&RBBs[i], &pl_RBB, edata);
				}
			} else if(skipdat == (BUILD | OBJPOP))
			{
				per_poly_collide(&entities[dWorldObjects[activeObjects[i]].type.entity_ID], &pl_RBB, RBBs[i].pos);
				if(you.hitObject  == true){
					RBBs[i].collisionID = pl_RBB.boxID;
					pl_RBB.collisionID = RBBs[i].boxID;
				}
			}
	}
	
	ldata_manager();
	
	// slPrintHex(dWorldObjects[1].rot[Y], slLocate(0, 10));
	// slPrintHex(dWorldObjects[2].rot[Y], slLocate(0, 11));
	// slPrintHex(dWorldObjects[3].rot[Y], slLocate(0, 12));
	// slPrintHex(dWorldObjects[4].rot[Y], slLocate(0, 13));
	
	// slPrintHex(dWorldObjects[5].type.ext_dat, slLocate(13, 12));
	// slPrintHex(dWorldObjects[6].type.ext_dat, slLocate(13, 13));
	
	// slPrintHex(dWorldObjects[5].pos[Y], slLocate(0, 15));
	// slPrintHex(dWorldObjects[6].pos[Y], slLocate(0, 16));
	
//	nbg_sprintf(0, 14, "(%i)E", numBoxChecks);
	numBoxChecks = 0;
	
}


