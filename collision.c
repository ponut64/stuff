/*
This file is compiled separately.
*/
///collision.c

#include <jo/jo.h>


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

//Separates the angles of a normal according to an input reference vector.
void	separateAngles(FIXED unitA[XYZ], FIXED plUN[XYZ], int degreeOut[XYZ])
{	
	static FIXED crossXZ[XYZ];
	static FIXED crossYZ[XYZ];
	static FIXED crossYX[XYZ];
	static FIXED mcXZ;
	static FIXED mcYZ;
	static FIXED mcYX;
	static FIXED dotXZ;
	static FIXED dotYZ;
	static FIXED dotYX;
	static FIXED smuXZ[XYZ]; 
	static FIXED smuYZ[XYZ]; 
	static FIXED smuYX[XYZ]; 
	static FIXED spnXZ[XYZ]; 
	static FIXED spnYZ[XYZ]; 
	static FIXED spnYX[XYZ];
	smuXZ[X] = unitA[X]; smuXZ[Y] = 0; smuXZ[Z] = unitA[Z];

	smuYZ[X] = 0; smuYZ[Y] = unitA[Y]; smuYZ[Z] = unitA[Z];

	smuYX[X] = unitA[X]; smuYX[Y] = unitA[Y]; smuYX[Z] = 0;

	spnXZ[X] = plUN[X]; spnXZ[Y] = 0; spnXZ[Z] = plUN[Z];
	
	spnYZ[X] = 0; spnYZ[Y] = plUN[Y]; spnYZ[Z] = plUN[Z];
	
	spnYX[X] = plUN[X]; spnYX[Y] = plUN[Y]; spnYX[Z] = 0;

	cross_fixed(smuXZ, spnXZ, crossXZ);
	cross_fixed(smuYZ, spnYZ, crossYZ);
	cross_fixed(smuYX, spnYX, crossYX);
	mcXZ = slSquartFX(fxm(crossXZ[X], crossXZ[X]) + fxm(crossXZ[Y], crossXZ[Y]) + fxm(crossXZ[Z], crossXZ[Z]));
	mcYZ = slSquartFX(fxm(crossYZ[X], crossYZ[X]) + fxm(crossYZ[Y], crossYZ[Y]) + fxm(crossYZ[Z], crossYZ[Z]));
	mcYX = slSquartFX(fxm(crossYX[X], crossYX[X]) + fxm(crossYX[Y], crossYX[Y]) + fxm(crossYX[Z], crossYX[Z]));
	dotXZ = fxm(unitA[X],plUN[X]) + fxm(unitA[Z],plUN[Z]);
	dotYZ = fxm(unitA[Y],plUN[Y]) + fxm(unitA[Z],plUN[Z]);
	dotYX = fxm(unitA[Y],plUN[Y]) + fxm(unitA[X],plUN[X]);
	if(mcXZ != 0 || dotXZ != 0) degreeOut[X] = (slAtan(mcXZ, dotXZ));
	if(mcYZ != 0 || dotYZ != 0) degreeOut[Y] = (slAtan(mcYZ, dotYZ));
	if(mcYX != 0 || dotYX != 0) degreeOut[Z] = (slAtan(mcYX, dotYX));
	//This is currently producing angle numbers that appear like this:
	//Degrees are relative to the plane normal as if a plane. 0 degrees is parallel to the plane, but perpendicular to the planar normal direction.
	//Degrees 360 to 270 appear to be for angles facing away from the plane.
	//Degrees 0 to 90 appear to be for angles facing into the plane.
	//The sign (+/-) of the angle does not appear to be spoken of.
	
	//Three components: Y rot [X-Z]. Xrot [Y-Z]. Zrot [Y-X]. The exact definition of these rotations depends on the axis.
}

//What is this doing?
//It is re-processing the X and Z values of the output as if it were rotated X and Z _after_ it is rotated by output's Y.
//This is very strange, when I think about it.
void	sort_angle_to_domain(FIXED unitNormal[XYZ], FIXED unitOrient[XYZ], int output[XYZ])
{
static int angleComponents[XYZ];
separateAngles(unitOrient, unitNormal, angleComponents);
Uint8 domain = solve_domain(unitNormal);
//jo_printf(0, 20, "(%i)", domain);
// deg * 182 = angle
if(domain == 1){ //++
output[X] = (fxm(fxm(slSin(angleComponents[Z]), (angleComponents[Z] - 49140) ), slSin((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), (angleComponents[Y] - 49140) ), slSin((output[Y] + (90 * 182))) )); 
output[Z] = (fxm(fxm(slSin(angleComponents[Z]), angleComponents[Z] - 49140), slCos((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), angleComponents[Y] - 49140), slCos((output[Y] + (90 * 182))) )); 
						//return;
} else if(domain == 2){ //-+
output[X] = (fxm(fxm(slSin(angleComponents[Z]), (angleComponents[Z] - 49140) ), slSin((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), (angleComponents[Y] - 49140) ), slSin((output[Y] - (90 * 182))) )); 
output[Z] = (fxm(fxm(slSin(angleComponents[Z]), angleComponents[Z] - 49140), slCos((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), angleComponents[Y] - 49140), slCos((output[Y] - (90 * 182))) )); 
						//return;
} else if(domain == 3){ //+-
output[X] = -(fxm(fxm(slSin(angleComponents[Z]), (angleComponents[Z] - 49140) ), slSin((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), (angleComponents[Y] - 49140) ), slSin((output[Y] - (90 * 182))) )); 
output[Z] = -(fxm(fxm(slSin(angleComponents[Z]), angleComponents[Z] - 49140), slCos((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), angleComponents[Y] - 49140), slCos((output[Y] - (90 * 182))) )); 
						//return;
} else if(domain == 4){ //--
output[X] = -(fxm(fxm(slSin(angleComponents[Z]), (angleComponents[Z] - 49140) ), slSin((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), (angleComponents[Y] - 49140) ), slSin((output[Y] + (90 * 182))) )); 
output[Z] = -(fxm(fxm(slSin(angleComponents[Z]), angleComponents[Z] - 49140), slCos((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), angleComponents[Y] - 49140), slCos((output[Y] + (90 * 182))) )); 
						//return;
}	
}


void	set_from_this_normal(Uint8 normID, _boundBox stator, VECTOR setNormal)
{
	
	//jo_printf(12, 9, "(%i)", normID);
	
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
	
Floor collisions pass the boolean "hitSurface" that is processed in player_phy.c
Wall collisions pass the boolean "hitWall" that is processed in player_phy.c
	
	*/

	if(hitFace == N_Yn){
			if(stator->UVNY[Y] < -32768){
		
		you.floorNorm[X] = stator->UVY[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = stator->UVY[Y];
		you.floorNorm[Z] = stator->UVY[Z];
		
	sort_angle_to_domain(stator->UVY, alwaysLow, you.rot);
		
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
		
		you.floorNorm[X] = stator->UVNY[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = stator->UVNY[Y];
		you.floorNorm[Z] = stator->UVNY[Z];
		
	sort_angle_to_domain(stator->UVY, alwaysLow, you.rot);
		
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
		
		you.floorNorm[X] = stator->UVNZ[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = stator->UVNZ[Y];
		you.floorNorm[Z] = stator->UVNZ[Z];
		
	sort_angle_to_domain(stator->UVZ, alwaysLow, you.rot);
		
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
		
		you.floorNorm[X] = stator->UVZ[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = stator->UVZ[Y];
		you.floorNorm[Z] = stator->UVZ[Z];
		
	sort_angle_to_domain(stator->UVZ, alwaysLow, you.rot);
		
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
		
		you.floorNorm[X] = stator->UVNX[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = stator->UVNX[Y];
		you.floorNorm[Z] = stator->UVNX[Z];
		
	sort_angle_to_domain(stator->UVX, alwaysLow, you.rot);
		
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
		
		you.floorNorm[X] = stator->UVX[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = stator->UVX[Y];
		you.floorNorm[Z] = stator->UVX[Z];
		
	sort_angle_to_domain(stator->UVX, alwaysLow, you.rot);
		
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
		line_hit_plane_here(negative_pos, below_player, stator->Xplus, stator->UVX, stator->pos, xHit);
	} else {
		//This means we draw to X-
		line_hit_plane_here(negative_pos, below_player, stator->Xneg, stator->UVX, stator->pos, xHit);
	}
	
	if( centerDif[Y] < 0){
		//This means we draw to Y+
		line_hit_plane_here(negative_pos, below_player, stator->Yplus, stator->UVY, stator->pos, yHit);
	} else {
		//This means we draw to Y-
		line_hit_plane_here(negative_pos, below_player, stator->Yneg, stator->UVY, stator->pos, yHit);
	}
	
	if( centerDif[Z] < 0){
		//This means we draw to Z+
		line_hit_plane_here(negative_pos, below_player, stator->Zplus, stator->UVZ, stator->pos, zHit);
	} else {
		//This means we draw to Z-
		line_hit_plane_here(negative_pos, below_player, stator->Zneg, stator->UVZ, stator->pos, zHit);
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

bool	player_collide_boxes(_boundBox * stator, _boundBox * mover)
{

static FIXED bigRadius = 0;

static POINT centerDif = {0, 0, 0};

static POINT lineEnds[9];

static bool lineChecks[9];

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
		lineChecks[0] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Xplus, stator->UVX, stator->pos, lineEnds[0]);
		lineChecks[1] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Xplus, stator->UVX, stator->pos, lineEnds[1]);
		lineChecks[2] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Xplus, stator->UVX, stator->pos, lineEnds[2]);
	} else {
		//This means we draw to X-
		lineChecks[0] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Xneg, stator->UVX, stator->pos, lineEnds[0]);
		lineChecks[1] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Xneg, stator->UVX, stator->pos, lineEnds[1]);
		lineChecks[2] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Xneg, stator->UVX, stator->pos, lineEnds[2]);
	}
	
	if( centerDif[Y] < 0){
		//This means we draw to Y+
		lineChecks[3] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Yplus, stator->UVY, stator->pos, lineEnds[3]);
		lineChecks[4] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Yplus, stator->UVY, stator->pos, lineEnds[4]);
		lineChecks[5] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Yplus, stator->UVY, stator->pos, lineEnds[5]);
	} else {
		//This means we draw to Y-
		lineChecks[3] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Yneg, stator->UVY, stator->pos, lineEnds[3]);
		lineChecks[4] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Yneg, stator->UVY, stator->pos, lineEnds[4]);
		lineChecks[5] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Yneg, stator->UVY, stator->pos, lineEnds[5]);
	}
	
	if( centerDif[Z] < 0){
		//This means we draw to Z+
		lineChecks[6] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Zplus, stator->UVZ, stator->pos, lineEnds[6]);
		lineChecks[7] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Zplus, stator->UVZ, stator->pos, lineEnds[7]);
		lineChecks[8] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Zplus, stator->UVZ, stator->pos, lineEnds[8]);
	} else {
		//This means we draw to Z-
		lineChecks[6] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Zneg, stator->UVZ, stator->pos, lineEnds[6]);
		lineChecks[7] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Zneg, stator->UVZ, stator->pos, lineEnds[7]);
		lineChecks[8] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Zneg, stator->UVZ, stator->pos, lineEnds[8]);
	}
	
//	jo_printf(13, 12, "(%i)", hitFace);
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
	for(Uint8 i = 0; i < MAX_PHYS_PROXY; i++)
	{
		//jo_printf(0, 0, "(PHYS)"); //Debug ONLY
		skipdat = dWorldObjects[activeObjects[i]].type.ext_dat & (0xF000);
		if( skipdat == OBJPOP ){ //Check if object # is a collision-approved type
		if(player_collide_boxes(&RBBs[i], &pl_RBB) == true) return;
			} else if(skipdat == (ITEM | OBJPOP)) {
		run_item_collision(i, &pl_RBB);
			} else if(skipdat == (GATE_R | OBJPOP)) {
		test_gate_ring(i, &pl_RBB);
			} else if(skipdat == (GATE_P | OBJPOP)) {
		test_gate_posts(activeObjects[i], &pl_RBB);
		if(player_collide_boxes(&RBBs[i], &pl_RBB) == true) return;
			} else if(skipdat == (BUILD | OBJPOP))
			{
				if(RBBs[i].status[1] == 'C')
				{
				per_poly_collide(&entities[dWorldObjects[activeObjects[i]].type.entity_ID], &pl_RBB);
				}
			}
	}
	
	gate_track_manager();
	
	// slPrintHex(dWorldObjects[1].srot[Y], slLocate(0, 10));
	// slPrintHex(dWorldObjects[2].srot[Y], slLocate(0, 11));
	// slPrintHex(dWorldObjects[3].srot[Y], slLocate(0, 12));
	// slPrintHex(dWorldObjects[4].srot[Y], slLocate(0, 13));
	
	// slPrintHex(dWorldObjects[5].type.ext_dat, slLocate(13, 12));
	// slPrintHex(dWorldObjects[6].type.ext_dat, slLocate(13, 13));
	
	// slPrintHex(dWorldObjects[5].pos[Y], slLocate(0, 15));
	// slPrintHex(dWorldObjects[6].pos[Y], slLocate(0, 16));
	
//	jo_printf(0, 14, "(%i)E", numBoxChecks);
	numBoxChecks = 0;
	
}


