/*
This file is compiled separately.
*/
///collision.c

#include <jo/jo.h>


#include "collision.h"

///Alternative point collision detection. Faster, though less strict. Works entirely with integers, mostly 16-bit.
//FIXED point operation is commented out as it has issues with domain, particularly of Y axis rotation.
Bool sort_collide(FIXED pos[XYZ], _boundBox * targetBox, Uint8* nearNormalID, int tolerance)
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
			*nearNormalID = (Uint8)5;
		} else if(leastDistance == JO_ABS(dotNZ) ){
			*nearNormalID = (Uint8)6;
		} else if(leastDistance == JO_ABS(dotX) ){
			*nearNormalID = (Uint8)1;
		} else if(leastDistance == JO_ABS(dotNX) ){
			*nearNormalID = (Uint8)2;
		} else if(leastDistance == JO_ABS(dotY) ){
			*nearNormalID = (Uint8)3;
		} else if(leastDistance == JO_ABS(dotNY) ){
			*nearNormalID = (Uint8)4;
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
	// degreeOut[X] = slDivFX(65536.0, fxm(slAtan(mcXZ, dotXZ), 360.0));
	// degreeOut[Y] = slDivFX(65536.0, fxm(slAtan(mcYZ, dotYZ), 360.0));
	// degreeOut[Z] = slDivFX(65536.0, fxm(slAtan(mcYX, dotYX), 360.0));
	//This is currently producing angle numbers that appear like this:
	//Degrees are relative to the plane normal as if a plane. 0 degrees is parallel to the plane, but perpendicular to the planar normal direction.
	//Degrees 360 to 270 appear to be for angles facing away from the plane.
	//Degrees 0 to 90 appear to be for angles facing into the plane.
	//The sign (+/-) of the angle does not appear to be spoken of.
	
	//Three components: Y rot [X-Z]. Xrot [Y-Z]. Zrot [Y-X]. The exact definition of these rotations depends on the axis.
}

void	sort_face_to_line(FIXED p0[XYZ], FIXED p1[XYZ], _boundBox * targetBox, Uint8 nearNormalID, FIXED unmoving[XYZ], FIXED output[XYZ])
{
	if(nearNormalID == (Uint8)3){
		cntrl_line_hit_plane(p0, p1, targetBox->Yplus, targetBox->UVY, targetBox->pos, unmoving, output);
	} else if(nearNormalID == (Uint8)1){
		cntrl_line_hit_plane(p0, p1, targetBox->Xplus, targetBox->UVX, targetBox->pos, unmoving, output);
	} else if(nearNormalID == (Uint8)2){
		cntrl_line_hit_plane(p0, p1, targetBox->Xneg, targetBox->UVX, targetBox->pos, unmoving, output);
	} else if(nearNormalID == (Uint8)5){
		cntrl_line_hit_plane(p0, p1, targetBox->Zplus, targetBox->UVZ, targetBox->pos, unmoving, output);
	} else if(nearNormalID == (Uint8)6){
		cntrl_line_hit_plane(p0, p1, targetBox->Zneg, targetBox->UVZ, targetBox->pos, unmoving, output);
	} else if(nearNormalID == (Uint8)4){
		cntrl_line_hit_plane(p0, p1, targetBox->Yneg, targetBox->UVY, targetBox->pos, unmoving, output);
	}
}

void	sort_angle_to_domain(FIXED unitNormal[XYZ], FIXED unitOrient[XYZ], int output[XYZ])
{
static int angleComponents[XYZ];
separateAngles(unitOrient, unitNormal, angleComponents);
Uint8 domain = solve_domain(unitNormal);
//jo_printf(0, 20, "(%i)", domain);
if(domain == 1){ //++
output[X] = (fxm(fxm(slSin(angleComponents[Z]), (angleComponents[Z] - 49140) ), slSin((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), (angleComponents[Y] - 49140) ), slSin((output[Y] + DEGtoANG(90))) )); 
output[Z] = (fxm(fxm(slSin(angleComponents[Z]), angleComponents[Z] - 49140), slCos((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), angleComponents[Y] - 49140), slCos((output[Y] + DEGtoANG(90))) )); 
						return;
} else if(domain == 2){ //-+
output[X] = (fxm(fxm(slSin(angleComponents[Z]), (angleComponents[Z] - 49140) ), slSin((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), (angleComponents[Y] - 49140) ), slSin((output[Y] - DEGtoANG(90))) )); 
output[Z] = (fxm(fxm(slSin(angleComponents[Z]), angleComponents[Z] - 49140), slCos((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), angleComponents[Y] - 49140), slCos((output[Y] - DEGtoANG(90))) )); 
						return;
} else if(domain == 3){ //+-
output[X] = -(fxm(fxm(slSin(angleComponents[Z]), (angleComponents[Z] - 49140) ), slSin((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), (angleComponents[Y] - 49140) ), slSin((output[Y] - DEGtoANG(90))) )); 
output[Z] = -(fxm(fxm(slSin(angleComponents[Z]), angleComponents[Z] - 49140), slCos((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), angleComponents[Y] - 49140), slCos((output[Y] - DEGtoANG(90))) )); 
						return;
} else if(domain == 4){ //--
output[X] = -(fxm(fxm(slSin(angleComponents[Z]), (angleComponents[Z] - 49140) ), slSin((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), (angleComponents[Y] - 49140) ), slSin((output[Y] + DEGtoANG(90))) )); 
output[Z] = -(fxm(fxm(slSin(angleComponents[Z]), angleComponents[Z] - 49140), slCos((output[Y]) - angleComponents[X])) +
						fxm(fxm(slSin(angleComponents[Y]), angleComponents[Y] - 49140), slCos((output[Y] + DEGtoANG(90))) )); 
						return;
}	
}

