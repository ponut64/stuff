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

///Alternative point collision detection. Faster, though less strict. Works entirely with integers, mostly 16-bit.
//FIXED point operation is commented out as it has issues with domain, particularly of Y axis rotation.
Bool sort_collide(FIXED pos[XYZ], _boundBox * targetBox, int* nearNormalID, int tolerance)
{
	int dotX = realpt_to_plane(pos, targetBox->Xplus, targetBox->pos);
		if(dotX < tolerance){return false;}
	int dotNX = realpt_to_plane(pos, targetBox->Xneg, targetBox->pos);
		if(dotNX < tolerance){return false;}
	int dotY = realpt_to_plane(pos, targetBox->Yplus, targetBox->pos);
		if(dotY < tolerance){return false;}
	int dotNY = realpt_to_plane(pos, targetBox->Yneg, targetBox->pos);
		if(dotNY < tolerance){return false;}
	int dotZ = realpt_to_plane(pos, targetBox->Zplus, targetBox->pos);
		if(dotZ < tolerance){return false;}
	int dotNZ = realpt_to_plane(pos, targetBox->Zneg, targetBox->pos);
		if(dotNZ < tolerance){return false;}

	int leastDistance;
		leastDistance = JO_MIN(JO_MIN(JO_MIN(JO_MIN(JO_MIN(JO_ABS(dotZ), JO_ABS(dotNZ)), JO_ABS(dotX)), JO_ABS(dotNX)), JO_ABS(dotY)), JO_ABS(dotNY));
		if(leastDistance == JO_ABS(dotZ) ){
			*nearNormalID = N_Zp;
		} else if(leastDistance == JO_ABS(dotNZ) ){
			*nearNormalID = N_Zn;
		} else if(leastDistance == JO_ABS(dotX) ){
			*nearNormalID = N_Xp;
		} else if(leastDistance == JO_ABS(dotNX) ){
			*nearNormalID = N_Xn;
		} else if(leastDistance == JO_ABS(dotY) ){
			*nearNormalID = N_Yp;
		} else if(leastDistance == JO_ABS(dotNY) ){
			*nearNormalID = N_Yn;
			}
//End surface collision detection.

	return true;
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

/* 
if(alignF == 0)
{
mtxX[0] = rrX[X];
mtxX[1] = rrX[Y];
mtxX[2] = rrX[Z];

mtxY[0] = surface_normal[X];
mtxY[1] = surface_normal[Y];
mtxY[2] = surface_normal[Z];

mtxZ[0] = rrZ[X];
mtxZ[1] = rrZ[Y];
mtxZ[2] = rrZ[Z];

//Transform the unit vectors of the player by the floor's matrix
//Even though in this case the Y vector would be affected, we don't use it (yet?), so we don't calculate it.
rruX[X] = fxdot(mtxX, rrauX);
rruX[Y] = -fxdot(mtxY, rrauX);
rruX[Z] = fxdot(mtxZ, rrauX);

rruZ[X] = fxdot(mtxX, rrauZ);
rruZ[Y] = -fxdot(mtxY, rrauZ);
rruZ[Z] = fxdot(mtxZ, rrauZ);

pl_RBB.UVX[X] = rruX[X];
pl_RBB.UVX[Y] = -rruX[Y];
pl_RBB.UVX[Z] = -rruX[Z];
pl_RBB.UVY[X] = -surface_normal[X];
pl_RBB.UVY[Y] = -surface_normal[Y];
pl_RBB.UVY[Z] = -surface_normal[Z];
pl_RBB.UVZ[X] = rruZ[X];
pl_RBB.UVZ[Y] = -rruZ[Y];
pl_RBB.UVZ[Z] = -rruZ[Z];

pl_RBB.UVNX[X] = -pl_RBB.UVX[X];
pl_RBB.UVNX[Y] = -pl_RBB.UVX[Y];
pl_RBB.UVNX[Z] = -pl_RBB.UVX[Z];
pl_RBB.UVNY[X] = -pl_RBB.UVY[X];
pl_RBB.UVNY[Y] = -pl_RBB.UVY[Y];
pl_RBB.UVNY[Z] = -pl_RBB.UVY[Z];
pl_RBB.UVNZ[X] = -pl_RBB.UVZ[X];
pl_RBB.UVNZ[Y] = -pl_RBB.UVZ[Y];
pl_RBB.UVNZ[Z] = -pl_RBB.UVZ[Z];

pl_RBB.Xplus[X] = fxm(pl_RBB.UVX[X], pl_RBB.brad[X]);
pl_RBB.Xplus[Y] = fxm(pl_RBB.UVX[Y], pl_RBB.brad[Y]);
pl_RBB.Xplus[Z] = fxm(pl_RBB.UVX[Z], pl_RBB.brad[Z]);
pl_RBB.Yplus[X] = fxm(pl_RBB.UVY[X], pl_RBB.brad[X]);
pl_RBB.Yplus[Y] = fxm(pl_RBB.UVY[Y], pl_RBB.brad[Y]);
pl_RBB.Yplus[Z] = fxm(pl_RBB.UVY[Z], pl_RBB.brad[Z]);
pl_RBB.Zplus[X] = fxm(pl_RBB.UVZ[X], pl_RBB.brad[X]);
pl_RBB.Zplus[Y] = fxm(pl_RBB.UVZ[Y], pl_RBB.brad[Y]);
pl_RBB.Zplus[Z] = fxm(pl_RBB.UVZ[Z], pl_RBB.brad[Z]);

pl_RBB.Xneg[X] = -pl_RBB.Xplus[X];
pl_RBB.Xneg[Y] = -pl_RBB.Xplus[Y];
pl_RBB.Xneg[Z] = -pl_RBB.Xplus[Z];
pl_RBB.Yneg[X] = -pl_RBB.Yplus[X];
pl_RBB.Yneg[Y] = -pl_RBB.Yplus[Y];
pl_RBB.Yneg[Z] = -pl_RBB.Yplus[Z];
pl_RBB.Zneg[X] = -pl_RBB.Zplus[X];
pl_RBB.Zneg[Y] = -pl_RBB.Zplus[Y];
pl_RBB.Zneg[Z] = -pl_RBB.Zplus[Z];
} else {
	
mtxX[0] = rrX[X];
mtxX[1] = rrX[Y];
mtxX[2] = rrX[Z];

mtxY[0] = -surface_normal[X];
mtxY[1] = surface_normal[Y];
mtxY[2] = surface_normal[Z];

mtxZ[0] = rrZ[X];
mtxZ[1] = rrZ[Y];
mtxZ[2] = rrZ[Z];

//Transform the unit vectors of the player by the floor's matrix
//Even though in this case the Y vector would be affected, we don't use it (yet?), so we don't calculate it.
rruX[X] = fxdot(mtxX, rrauX);
rruX[Y] = -fxdot(mtxY, rrauX);
rruX[Z] = fxdot(mtxZ, rrauX);

rruZ[X] = fxdot(mtxX, rrauZ);
rruZ[Y] = -fxdot(mtxY, rrauZ);
rruZ[Z] = fxdot(mtxZ, rrauZ);

	
fxrotY(rruX, rrX, -you.rot[Y]);
fxrotY(rruZ, rrZ, -you.rot[Y]);
//Get the final X and Z rotation angles out of the calculation
output[X] = -(slAtan(rrZ[Y], rrZ[Z]) + 16383);
output[Z] = -(slAtan(rrX[Y], rrX[X]) - 16383);
}
 */

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

fmtx->Xplus[X] = fxm(fmtx->UVX[X], fmtx->brad[X]);
fmtx->Xplus[Y] = fxm(fmtx->UVX[Y], fmtx->brad[Y]);
fmtx->Xplus[Z] = fxm(fmtx->UVX[Z], fmtx->brad[Z]);
fmtx->Yplus[X] = fxm(fmtx->UVY[X], fmtx->brad[X]);
fmtx->Yplus[Y] = fxm(fmtx->UVY[Y], fmtx->brad[Y]);
fmtx->Yplus[Z] = fxm(fmtx->UVY[Z], fmtx->brad[Z]);
fmtx->Zplus[X] = fxm(fmtx->UVZ[X], fmtx->brad[X]);
fmtx->Zplus[Y] = fxm(fmtx->UVZ[Y], fmtx->brad[Y]);
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

void	standing_surface_alignment(FIXED * surface_normal, int * output)
{
	
/*

Mostly great function.
Process, follow comments.

*/

static int fwdYmtx[XYZ][XYZ];

static int plane_matrix[XYZ][XYZ];

//Zero-out plane matrix
zero_matrix(fwdYmtx[0]);
zero_matrix(plane_matrix[0]);

plane_matrix[Y][X] = surface_normal[X];
plane_matrix[Y][Y] = surface_normal[Y];
plane_matrix[Y][Z] = surface_normal[Z];

// Rotation unit vector X
int rruX[3] = {1<<16, 0, 0};

// Rotation unit vector Z
int rruZ[3] = {0, 0, 1<<16};

int rruY[3] = {0, 1<<16, 0};

//Initialize this part of the forward-Y matrix
fwdYmtx[Y][Y] = 1<<16;

/*

The following code chunk generates axis-aligned unit matrix data from the normal.
It does so with respect to the major axis of the normal and its sign.

*/
if(JO_ABS(surface_normal[Y]) > 32768)
{
	if(surface_normal[Y] < 0)
	{
		//Find the X axis of the floor's matrix (from rotating the Y axis by Z+90)
		fxrotZ(plane_matrix[Y], plane_matrix[X], 16834);
		//Find the Z axis of the floor's matrix (from rotating the Y axis by X+90)
		fxrotX(plane_matrix[Y], plane_matrix[Z], 16384);
		
		fxrotY(rruX, fwdYmtx[X], you.rot[Y]);
		fxrotY(rruZ, fwdYmtx[Z], you.rot[Y]);
	} else {
		//Find the X axis of the floor's matrix (from rotating the Y axis by Z+90)
		fxrotZ(plane_matrix[Y], plane_matrix[X], 16834);
		//Find the Z axis of the floor's matrix (from rotating the Y axis by X+90)
		fxrotX(plane_matrix[Y], plane_matrix[Z], -16384);
		
		fxrotY(rruX, fwdYmtx[X], -(you.rot[Y] + (180 * 182)));
		fxrotY(rruZ, fwdYmtx[Z], -(you.rot[Y] + (180 * 182)));
	}
	//These have to be zeroed-out since they must be axis-aligned.
	//Otherwise, they would inherit these portions of the surface normal!
	//Notice: this means these have to be re-normalized.
	plane_matrix[X][Z] = 0;
	plane_matrix[Z][X] = 0;
} else if(JO_ABS(surface_normal[X]) > 32768){

	if(surface_normal[X] > 0)
	{
		fxrotZ(plane_matrix[Y], plane_matrix[X], 16384);
		fxrotY(plane_matrix[Y], plane_matrix[Z], 16384);
	} else {
		//This may not be right.
		fxrotZ(plane_matrix[Y], plane_matrix[X], -16384);
		fxrotY(plane_matrix[Y], plane_matrix[Z], -16384);
	}
	
	//These have to be zeroed-out since they must be axis-aligned.
	//Otherwise, they would inherit these portions of the surface normal!
	//Notice: this means these have to be re-normalized.
	plane_matrix[X][Z] = 0;
	plane_matrix[Z][Y] = 0;
	
	fxrotY(rruX, fwdYmtx[X], you.rot[Y]);
	fxrotY(rruZ, fwdYmtx[Z], you.rot[Y]);
} else if(JO_ABS(surface_normal[Z]) > 32768)
{

	if(surface_normal[Z] > 0)
	{
		fxrotY(plane_matrix[Y], plane_matrix[X], 16384);
		fxrotX(plane_matrix[Y], plane_matrix[Z], 16384);
	} else {
		//This may not be right.
		fxrotY(plane_matrix[Y], plane_matrix[X], -16384);
		fxrotX(plane_matrix[Y], plane_matrix[Z], -16384);
	}
	//These have to be zeroed-out since they must be axis-aligned.
	//Otherwise, they would inherit these portions of the surface normal!
	//Notice: this means these have to be re-normalized.
	plane_matrix[X][Y] = 0;
	plane_matrix[Z][X] = 0;
	
	fxrotY(rruX, fwdYmtx[X], you.rot[Y]);
	fxrotY(rruZ, fwdYmtx[Z], you.rot[Y]);
}

// accurate_normalize(fwdYmtx[X], fwdYmtx[X]);
// accurate_normalize(fwdYmtx[Z], fwdYmtx[Z]);
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
fxRotLocalAxis(plane_matrix[0], fwdYmtx[Y], you.rot[Y]);

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

static short drawposA[3];
static short drawposB[3];
static int 	drawposC[3];
static short drawposD[3];
static short drawposE[3];
static short drawposF[3];

drawposC[X] = pl_RBB.Yplus[X] - you.pos[X];
drawposC[Y] = pl_RBB.Yplus[Y] - you.pos[Y];
drawposC[Z] = pl_RBB.Yplus[Z] - you.pos[Z];

drawposA[X] = (plane_matrix[Y][X]>>2); 
drawposA[Y] = (plane_matrix[Y][Y]>>2); 
drawposA[Z] = (plane_matrix[Y][Z]>>2); 

drawposB[X] = (rruY[X]>>2); 
drawposB[Y] = (rruY[Y]>>2); 
drawposB[Z] = (rruY[Z]>>2); 
				  
drawposD[X] = (rruX[X]>>2); 
drawposD[Y] = (rruX[Y]>>2); 
drawposD[Z] = (rruX[Z]>>2); 

drawposE[X] = (plane_matrix[X][X]>>2); 
drawposE[Y] = (plane_matrix[X][Y]>>2); 
drawposE[Z] = (plane_matrix[X][Z]>>2); 
				  
drawposF[X] = (plane_matrix[Z][X]>>2); 
drawposF[Y] = (plane_matrix[Z][Y]>>2); 
drawposF[Z] = (plane_matrix[Z][Z]>>2); 

// add_to_sprite_list(drawposC, drawposB, 7, 0, 'L', 0, 2184);

// add_to_sprite_list(drawposC, drawposD, 15, 0, 'L', 0, 2184);

add_to_sprite_list(drawposC, drawposE, 23, 0, 'L', 0, 2184);

add_to_sprite_list(drawposC, drawposF, 31, 0, 'L', 0, 2184);

add_to_sprite_list(drawposC, drawposA, 1, 0, 'L', 0, 2184);



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

void	pl_physics_handler(_boundBox * stator, _boundBox * mover, POINT hitPt, Uint8 hitFace)
{

	/*
	
Floor collisions pass the Boolean "hitSurface" that is processed in player_phy.c
Wall collisions pass the Boolean "hitWall" that is processed in player_phy.c
	
	*/

	if(hitFace == N_Yn){
			if(stator->UVNY[Y] < -32768){
		
		you.floorNorm[X] = stator->UVNY[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = stator->UVNY[Y];
		you.floorNorm[Z] = stator->UVNY[Z];
		
	standing_surface_alignment(you.floorNorm, you.renderRot);
		
	you.floorPos[X] = (-(hitPt[X]) - (mover->Yneg[X]));
	you.floorPos[Y] = (-(hitPt[Y]) - (mover->Yneg[Y]));
	you.floorPos[Z] = (-(hitPt[Z]) - (mover->Yneg[Z]));
	you.shadowPos[X] = -hitPt[X];
	you.shadowPos[Y] = -hitPt[Y];
	you.shadowPos[Z] = -hitPt[Z];
		
	you.hitSurface = true;
			} else {
	you.wallNorm[X] = stator->UVNY[X];
	you.wallNorm[Y] = stator->UVNY[Y];
	you.wallNorm[Z] = stator->UVNY[Z];
	you.wallPos[X] = hitPt[X];
	you.wallPos[Y] = hitPt[Y];
	you.wallPos[Z] = hitPt[Z];
		
	you.hitWall = true;
			}
	} else if(hitFace == N_Yp){
		
			if(stator->UVY[Y] < -32768){
		
		you.floorNorm[X] = stator->UVY[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = stator->UVY[Y];
		you.floorNorm[Z] = stator->UVY[Z];
		
	standing_surface_alignment(you.floorNorm, you.renderRot);
		
	you.floorPos[X] = (-(hitPt[X]) - (mover->Yneg[X]));
	you.floorPos[Y] = (-(hitPt[Y]) - (mover->Yneg[Y]));
	you.floorPos[Z] = (-(hitPt[Z]) - (mover->Yneg[Z]));
	you.shadowPos[X] = -hitPt[X];
	you.shadowPos[Y] = -hitPt[Y];
	you.shadowPos[Z] = -hitPt[Z];
		
	you.hitSurface = true;
			} else {
	you.wallNorm[X] = stator->UVY[X];
	you.wallNorm[Y] = stator->UVY[Y];
	you.wallNorm[Z] = stator->UVY[Z];
	you.wallPos[X] = hitPt[X];
	you.wallPos[Y] = hitPt[Y];
	you.wallPos[Z] = hitPt[Z];
		
	you.hitWall = true;
			}
	} else if(hitFace == N_Zp){
		
			if(stator->UVZ[Y] < -32768){
		
		you.floorNorm[X] = stator->UVZ[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = stator->UVZ[Y];
		you.floorNorm[Z] = stator->UVZ[Z];
		
	standing_surface_alignment(you.floorNorm, you.renderRot);
		
	you.floorPos[X] = (-(hitPt[X]) - (mover->Yneg[X]));
	you.floorPos[Y] = (-(hitPt[Y]) - (mover->Yneg[Y]));
	you.floorPos[Z] = (-(hitPt[Z]) - (mover->Yneg[Z]));
	you.shadowPos[X] = -hitPt[X];
	you.shadowPos[Y] = -hitPt[Y];
	you.shadowPos[Z] = -hitPt[Z];
		
	you.hitSurface = true;
			} else {
	you.wallNorm[X] = stator->UVZ[X];
	you.wallNorm[Y] = stator->UVZ[Y];
	you.wallNorm[Z] = stator->UVZ[Z];
	you.wallPos[X] = hitPt[X];
	you.wallPos[Y] = hitPt[Y];
	you.wallPos[Z] = hitPt[Z];
		
	you.hitWall = true;
			}
	} else if(hitFace == N_Zn){
		
			if(stator->UVNZ[Y] < -32768){
		
		you.floorNorm[X] = stator->UVNZ[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = stator->UVNZ[Y];
		you.floorNorm[Z] = stator->UVNZ[Z];
		
	standing_surface_alignment(you.floorNorm, you.renderRot);
		
	you.floorPos[X] = (-(hitPt[X]) - (mover->Yneg[X]));
	you.floorPos[Y] = (-(hitPt[Y]) - (mover->Yneg[Y]));
	you.floorPos[Z] = (-(hitPt[Z]) - (mover->Yneg[Z]));
	you.shadowPos[X] = -hitPt[X];
	you.shadowPos[Y] = -hitPt[Y];
	you.shadowPos[Z] = -hitPt[Z];
		
	you.hitSurface = true;
			} else {
	you.wallNorm[X] = stator->UVNZ[X];
	you.wallNorm[Y] = stator->UVNZ[Y];
	you.wallNorm[Z] = stator->UVNZ[Z];
	you.wallPos[X] = hitPt[X];
	you.wallPos[Y] = hitPt[Y];
	you.wallPos[Z] = hitPt[Z];
		
	you.hitWall = true;
			}
	} else if(hitFace == N_Xp){

			if(stator->UVX[Y] < -32768){
		
		you.floorNorm[X] = stator->UVX[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = stator->UVX[Y];
		you.floorNorm[Z] = stator->UVX[Z];
		
	standing_surface_alignment(you.floorNorm, you.renderRot);
		
	you.floorPos[X] = (-(hitPt[X]) - (mover->Yneg[X]));
	you.floorPos[Y] = (-(hitPt[Y]) - (mover->Yneg[Y]));
	you.floorPos[Z] = (-(hitPt[Z]) - (mover->Yneg[Z]));
	you.shadowPos[X] = -hitPt[X];
	you.shadowPos[Y] = -hitPt[Y];
	you.shadowPos[Z] = -hitPt[Z];
	
	you.hitSurface = true;
			} else {
	you.wallNorm[X] = stator->UVX[X];
	you.wallNorm[Y] = stator->UVX[Y];
	you.wallNorm[Z] = stator->UVX[Z];
	you.wallPos[X] = hitPt[X];
	you.wallPos[Y] = hitPt[Y];
	you.wallPos[Z] = hitPt[Z];
		
	you.hitWall = true;
			}
	} else if(hitFace == N_Xn){

			if(stator->UVNX[Y] < -32768){
		
		you.floorNorm[X] = stator->UVNX[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = stator->UVNX[Y];
		you.floorNorm[Z] = stator->UVNX[Z];
		
	standing_surface_alignment(you.floorNorm, you.renderRot);
		
	you.floorPos[X] = (-(hitPt[X]) - (mover->Yneg[X]));
	you.floorPos[Y] = (-(hitPt[Y]) - (mover->Yneg[Y]));
	you.floorPos[Z] = (-(hitPt[Z]) - (mover->Yneg[Z]));
	you.shadowPos[X] = -hitPt[X];
	you.shadowPos[Y] = -hitPt[Y];
	you.shadowPos[Z] = -hitPt[Z];
		
	you.hitSurface = true;
			} else {
	you.wallNorm[X] = stator->UVNX[X];
	you.wallNorm[Y] = stator->UVNX[Y];
	you.wallNorm[Z] = stator->UVNX[Z];
	you.wallPos[X] = hitPt[X];
	you.wallPos[Y] = hitPt[Y];
	you.wallPos[Z] = hitPt[Z];
		
	you.hitWall = true;
			}
	}

}

void	player_shadow_object(_boundBox * stator, POINT centerDif)
{
	
	//Again, I have no idea how my coordinate systems get so reliably inverted...
	POINT below_player = {-you.pos[X], -(you.pos[Y] - (1<<16)), -you.pos[Z]};
	POINT negative_pos = {-you.pos[X], -(you.pos[Y]), -you.pos[Z]};
	POINT xHit;
	POINT yHit;
	POINT zHit;
	int hitBools[XYZ];
	POINT highHit = {0, 0, 0};

	if( centerDif[X] < 0){
		//This means we draw to X+
		line_hit_plane_here(negative_pos, below_player, stator->Xplus, stator->UVX, stator->pos, 16384, xHit);
	} else {
		//This means we draw to X-
		line_hit_plane_here(negative_pos, below_player, stator->Xneg, stator->UVX, stator->pos, 16384, xHit);
	}
	
	if( centerDif[Y] < 0){
		//This means we draw to Y+
		line_hit_plane_here(negative_pos, below_player, stator->Yplus, stator->UVY, stator->pos, 1<<16, yHit);
	} else {
		//This means we draw to Y-
		line_hit_plane_here(negative_pos, below_player, stator->Yneg, stator->UVY, stator->pos, 1<<16, yHit);
	}
	
	if( centerDif[Z] < 0){
		//This means we draw to Z+
		line_hit_plane_here(negative_pos, below_player, stator->Zplus, stator->UVZ, stator->pos, 16384, zHit);
	} else {
		//This means we draw to Z-
		line_hit_plane_here(negative_pos, below_player, stator->Zneg, stator->UVZ, stator->pos, 16384, zHit);
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
	
	if((hitBools[X] == true || hitBools[Y] == true || hitBools[Z] == true) && you.pos[Y] > -highHit[Y])
	{
		//Inverted coordinates...
		you.shadowPos[X] = -highHit[X];
		you.shadowPos[Y] = -highHit[Y];
		you.shadowPos[Z] = -highHit[Z];
		you.aboveObject = true;
	}

}

Bool	player_collide_boxes(_boundBox * stator, _boundBox * mover)
{

static FIXED bigRadius = 0;

static POINT centerDif = {0, 0, 0};

static POINT lineEnds[9];

static Bool lineChecks[9];

static int hitFace;
		
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

if(bigDif > (bigRadius + (20<<16))) return false;

numBoxChecks++;

//Box Collision Check
_lineTable moverCFs = {
	.xp0[X] = mover->Xplus[X] 	- mover->pos[X] - mover->velocity[X],
	.xp0[Y] = mover->Xplus[Y] 	- mover->pos[Y] - mover->velocity[Y],
	.xp0[Z] = mover->Xplus[Z] 	- mover->pos[Z] - mover->velocity[Z],
	.xp1[X] = mover->Xneg[X] 	- mover->pos[X] + mover->velocity[X],
	.xp1[Y] = mover->Xneg[Y] 	- mover->pos[Y] + mover->velocity[Y],
	.xp1[Z] = mover->Xneg[Z] 	- mover->pos[Z] + mover->velocity[Z],
	.yp0[X] = mover->Yplus[X] 	- mover->pos[X] - mover->velocity[X],
	.yp0[Y] = mover->Yplus[Y] 	- mover->pos[Y] - mover->velocity[Y],
	.yp0[Z] = mover->Yplus[Z] 	- mover->pos[Z] - mover->velocity[Z],
	.yp1[X] = mover->Yneg[X] 	- mover->pos[X] + mover->velocity[X],
	.yp1[Y] = mover->Yneg[Y] 	- mover->pos[Y] + mover->velocity[Y],
	.yp1[Z] = mover->Yneg[Z] 	- mover->pos[Z] + mover->velocity[Z],
	.zp0[X] = mover->Zplus[X] 	- mover->pos[X] - mover->velocity[X],
	.zp0[Y] = mover->Zplus[Y] 	- mover->pos[Y] - mover->velocity[Y],
	.zp0[Z] = mover->Zplus[Z] 	- mover->pos[Z] - mover->velocity[Z],
	.zp1[X] = mover->Zneg[X] 	- mover->pos[X] + mover->velocity[X],
	.zp1[Y] = mover->Zneg[Y] 	- mover->pos[Y] + mover->velocity[Y],
	.zp1[Z] = mover->Zneg[Z] 	- mover->pos[Z] + mover->velocity[Z]
}; 

		/*
	ABSOLUTE PRIORITY: Once again, the normal hit during collision must be found ABSOLUTELY. ACCURATELY.
	How we did this best before:
	Step 0: Determine which faces face you.
	Step 1: Draw lines to a point from every pair of CFs on the mover to every facing face of the stator->
	Step 2: Test the points and find which one collides and which face it collided with.
	Step 3: Use the normal of that face for collision.

		*/
		

//	Step 0: Determine which faces face you.

	POINT negCenter = {-mover->pos[X], -mover->pos[Y], -mover->pos[Z]};
	//This math figures out what side of the box we're on, with respect to its rotation, too.
	centerDif[X] = realpt_to_plane(negCenter, stator->UVX, stator->pos);
	centerDif[Y] = realpt_to_plane(negCenter, stator->UVY, stator->pos);
	centerDif[Z] = realpt_to_plane(negCenter, stator->UVZ, stator->pos);
//	Step 1: Draw lines to a point from every pair of CFs on the mover to every face of the stator->
	if( centerDif[X] < 0){
		//This means we draw to X+
		lineChecks[0] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Xplus, stator->UVX, stator->pos, 16384, lineEnds[0]);
		lineChecks[1] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Xplus, stator->UVX, stator->pos, 1<<16, lineEnds[1]);
		lineChecks[2] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Xplus, stator->UVX, stator->pos, 16384, lineEnds[2]);
	} else {
		//This means we draw to X-
		lineChecks[0] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Xneg, stator->UVX, stator->pos, 16384, lineEnds[0]);
		lineChecks[1] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Xneg, stator->UVX, stator->pos, 1<<16, lineEnds[1]);
		lineChecks[2] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Xneg, stator->UVX, stator->pos, 16384, lineEnds[2]);
	}
	
	if( centerDif[Y] < 0){
		//This means we draw to Y+
		lineChecks[3] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Yplus, stator->UVY, stator->pos, 16384, lineEnds[3]);
		lineChecks[4] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Yplus, stator->UVY, stator->pos, 1<<16, lineEnds[4]);
		lineChecks[5] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Yplus, stator->UVY, stator->pos, 16384, lineEnds[5]);
	} else {
		//This means we draw to Y-
		lineChecks[3] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Yneg, stator->UVY, stator->pos, 16384, lineEnds[3]);
		lineChecks[4] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Yneg, stator->UVY, stator->pos, 1<<16, lineEnds[4]);
		lineChecks[5] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Yneg, stator->UVY, stator->pos, 16384, lineEnds[5]);
	}
	
	if( centerDif[Z] < 0){
		//This means we draw to Z+
		lineChecks[6] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Zplus, stator->UVZ, stator->pos, 16384, lineEnds[6]);
		lineChecks[7] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Zplus, stator->UVZ, stator->pos, 1<<16, lineEnds[7]);
		lineChecks[8] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Zplus, stator->UVZ, stator->pos, 16384, lineEnds[8]);
	} else {
		//This means we draw to Z-
		lineChecks[6] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Zneg, stator->UVZ, stator->pos, 16384, lineEnds[6]);
		lineChecks[7] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Zneg, stator->UVZ, stator->pos, 1<<16, lineEnds[7]);
		lineChecks[8] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Zneg, stator->UVZ, stator->pos, 16384, lineEnds[8]);
	}
	
//	nbg_sprintf(13, 12, "(%i)", hitFace);
				player_shadow_object(stator, centerDif);
		//Step 2: Test the points and find which one collides and which face it collided with.
	for(int i = 0; i < 9; i++){
		if(lineChecks[i] == true){
			if(sort_collide(lineEnds[i], stator, &hitFace, -HIT_TOLERANCE) == true){
				//Step 3: Use the normal of that face for collision.
				pl_physics_handler(stator, mover, lineEnds[i], hitFace);
				you.hitBox = true;
				return true;
			}
		}
		
	}
		you.hitBox = false;
		return false;
}

void	player_collision_test_loop(void)
{
	//This runs the physics workhorse stuff, but a lot of this logic is in physobjet.c -
	//much of it has as much to do with game state as it does physics.
	//
	//How could this be structured to be more easily expandable?
	// 1 - Make a compiler-defined # of "behaviour types"
	// 2 - Run the loop for every behaviour type
	// 3 - A behaviour array is a list of pointers to functions that match the type of stuff you want to track
	// They can be container functions for multiple things, or a direct-to-conclusion sort of thing
	//Then again, maybe not needed -
	//Consider, this is just for physics proxies.
	//What you want to add most is a projectile tracker.
	//Instead of running checks from the physics proxies to the projectiles, 
	//run a check from the projectile list on the physics proxy list. (and the ground)
	//Which involves new control logic.
	//Also makes me afraid of projectile logic, since I am using RBB (not AABB). But it is just one point to test, so maybe not so bad.
	//In any case, you end up with projectiles operating on a different control and data method than normal physics proxies.
	
	you.hitObject = false;
	if(ldata_ready != true) return; //Just in case.
	int skipdat;
	int edata;
	for(Uint8 i = 0; i < MAX_PHYS_PROXY; i++)
	{
		//nbg_sprintf(0, 0, "(PHYS)"); //Debug ONLY
		edata = dWorldObjects[activeObjects[i]].type.ext_dat;
		skipdat = edata & (0xF000);
		if( skipdat == OBJPOP ){ //Check if object # is a collision-approved type
				if(player_collide_boxes(&RBBs[i], &pl_RBB) == true)
				{
					return;
				}
			} else if(skipdat == (ITEM | OBJPOP)) {
				run_item_collision(i, &pl_RBB);
			} else if(skipdat == (GATE_R | OBJPOP)) {
				test_gate_ring(i, &pl_RBB);
			} else if(skipdat == (GATE_P | OBJPOP)) {
				test_gate_posts(activeObjects[i], &pl_RBB);
				if(player_collide_boxes(&RBBs[i], &pl_RBB) == true)
				{
					return;
				}
			} else if(skipdat == (BUILD | OBJPOP))
			{
				if(RBBs[i].status[1] == 'C')
				{
					per_poly_collide(&entities[dWorldObjects[activeObjects[i]].type.entity_ID], &pl_RBB, RBBs[i].pos);
				}
			}
	}
	
	gate_track_manager();
	
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


