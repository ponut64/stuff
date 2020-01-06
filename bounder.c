/*
This file is compiled separately.
*/

///HIGH LEVEL ALERT///
/**THIS FUNCTION HAS HIGH PROBABILITY OF CAUSING CRITICAL GLITCHES**/

#include "bounder.h"

_boundBox * RBBs; //In LWRAM // 
_boundBox pl_RBB;
Uint8 curBoxes = 0;

//Usage:
// param X, Y, Z: The location of the box (center)
// param xrot, yrot, zrot: the rotation of the box.
// param r1x, r2y, r3z: the X, Y, and Z radius of the box.
//note: export models as -Y forward, Z up.
// param bbox: the bounding box struct to be modified.
void	makeBoundBox(FIXED x, FIXED y, FIXED z, ANGLE xrot, ANGLE yrot, ANGLE zrot, FIXED r1x, FIXED r2y, FIXED r3z, _boundBox * bbox)
{
	FIXED prevXpos[XYZ]; 
	prevXpos[X] = bbox->Xplus[X];
	prevXpos[Y] = bbox->Xplus[Y];
	prevXpos[Z] = bbox->Xplus[Z];
	FIXED prevYpos[XYZ]; 
	prevYpos[X] = bbox->Yplus[X];
	prevYpos[Y] = bbox->Yplus[Y]; 
	prevYpos[Z] = bbox->Yplus[Z];
	FIXED prevZpos[XYZ];
	prevZpos[X] = bbox->Zplus[X];
	prevZpos[Y] = bbox->Zplus[Y];
	prevZpos[Z] = bbox->Zplus[Z];
	FIXED prevNXpos[XYZ];
	prevNXpos[X] = bbox->Xneg[X];
	prevNXpos[Y] = bbox->Xneg[Y];
	prevNXpos[Z] = bbox->Xneg[Z];
	FIXED prevNYpos[XYZ];
	prevNYpos[X] = bbox->Yneg[X]; 
	prevNYpos[Y] = bbox->Yneg[Y];
	prevNYpos[Z] = bbox->Yneg[Z];
	FIXED prevNZpos[XYZ];
	prevNZpos[X] = bbox->Zneg[X]; 
	prevNZpos[Y] = bbox->Zneg[Y];
	prevNZpos[Z] = bbox->Zneg[Z];
	//Give the box its location
	bbox->pos[X] = -x;
	bbox->pos[Y] = -y;
	bbox->pos[Z] = -z;
	//Give the box its rotation
	bbox->boxRot[X] = xrot;
	bbox->boxRot[Y] = yrot;
	bbox->boxRot[Z] = zrot;
	//Give the box its radius
	bbox->brad[X] = r1x;
	bbox->brad[Y] = r2y;
	bbox->brad[Z] = r3z;

	register FIXED sinX = slSin(bbox->boxRot[X]);
	register FIXED cosX = slCos(bbox->boxRot[Y]);
	register FIXED sinY = slSin(bbox->boxRot[Y]);
	register FIXED cosY = slCos(bbox->boxRot[Y]);
	register FIXED sinZ = slSin(bbox->boxRot[Z]);
	register FIXED cosZ = slCos(bbox->boxRot[Z]);
	//SETUP UNIT VECTOR X
	bbox->UVX[X] = fxm(cosY, cosZ);
	bbox->UVX[Y] = fxm(sinZ, cosX) + fxm(fxm(sinX, sinY), cosZ);
	bbox->UVX[Z] = -fxm(fxm(sinY, cosZ), cosX) + fxm(sinZ, sinX);
	
	(*bbox).Xplus[X] = fxm((bbox->brad[X]), bbox->UVX[X]);
	(*bbox).Xplus[Y] = fxm((bbox->brad[X]), bbox->UVX[Y]);
	(*bbox).Xplus[Z] = fxm((bbox->brad[X]), bbox->UVX[Z]);

	//SETUP UNIT VECTOR Y
	bbox->UVY[X] = -fxm(sinZ, cosY);
	bbox->UVY[Y] = fxm(cosZ, cosX) - fxm(fxm(sinX, sinY), sinZ);
	bbox->UVY[Z] = fxm(sinX, cosZ) + fxm(fxm(sinY, sinZ), cosX);

	(*bbox).Yplus[X] = fxm((bbox->brad[Y]), bbox->UVY[X]);
	(*bbox).Yplus[Y] = fxm((bbox->brad[Y]), bbox->UVY[Y]);
	(*bbox).Yplus[Z] = fxm((bbox->brad[Y]), bbox->UVY[Z]);
	
	//SETUP UNIT VECTOR Z
	bbox->UVZ[X] = (sinY);
	bbox->UVZ[Y] = -fxm(sinX, cosY);
	bbox->UVZ[Z] = fxm(cosX, cosY);
	
	(*bbox).Zplus[X] = fxm((bbox->brad[Z]), bbox->UVZ[X]);
	(*bbox).Zplus[Y] = fxm((bbox->brad[Z]), bbox->UVZ[Y]);
	(*bbox).Zplus[Z] = fxm((bbox->brad[Z]), bbox->UVZ[Z]);
	//---------------------------------------------------------------------------------------------------------------
	bbox->UVNX[X] = -bbox->UVX[X];
	bbox->UVNX[Y] = -bbox->UVX[Y];
	bbox->UVNX[Z] = -bbox->UVX[Z];
	bbox->UVNY[X] = -bbox->UVY[X];
	bbox->UVNY[Y] = -bbox->UVY[Y];
	bbox->UVNY[Z] = -bbox->UVY[Z];
	bbox->UVNZ[X] = -bbox->UVZ[X];
	bbox->UVNZ[Y] = -bbox->UVZ[Y];
	bbox->UVNZ[Z] = -bbox->UVZ[Z];
	//axis given: Y (on Z axis and does not change Y axis) circle going only right/left, forward/backward
	(*bbox).Xneg[X] = -fxm((bbox->brad[X]), bbox->UVX[X]);
	(*bbox).Xneg[Y] = -fxm((bbox->brad[X]), bbox->UVX[Y]);
	(*bbox).Xneg[Z] = -fxm((bbox->brad[X]), bbox->UVX[Z]);
	//axis given: X (on Y axis and does not change X axis) circle going only up/down, forward/backward
	(*bbox).Yneg[X] = -fxm((bbox->brad[Y]), bbox->UVY[X]);
	(*bbox).Yneg[Y] = -fxm((bbox->brad[Y]), bbox->UVY[Y]);
	(*bbox).Yneg[Z] = -fxm((bbox->brad[Y]), bbox->UVY[Z]);
	//axis given: Z (on X axis and does not change Z axis) Circle going only up/down, left/right
	(*bbox).Zneg[X] = -fxm((bbox->brad[Z]), bbox->UVZ[X]);
	(*bbox).Zneg[Y] = -fxm((bbox->brad[Z]), bbox->UVZ[Y]);
	(*bbox).Zneg[Z] = -fxm((bbox->brad[Z]), bbox->UVZ[Z]);
	//end of negative

	//Sort and assign X, Y, and Z maximum normals. (For macros)
	//Warning: Sorting is GONE. :(

	//Determine a velocity from the difference of current and last position
	segment_to_vector(bbox->prevPos, bbox->pos, bbox->velocity);
	segment_to_vector(prevXpos, bbox->Xplus, bbox->veloX);
	segment_to_vector(prevYpos, bbox->Yplus, bbox->veloY);
	segment_to_vector(prevZpos, bbox->Zplus, bbox->veloZ);
	
	segment_to_vector(prevNXpos, bbox->Xneg, bbox->veloNX);
	segment_to_vector(prevNYpos, bbox->Yneg, bbox->veloNY);
	segment_to_vector(prevNZpos, bbox->Zneg, bbox->veloNZ);
	
	//Fill this crap out.
	bbox->prevPos[X] = bbox->pos[X];
	bbox->prevPos[Y] = bbox->pos[Y];
	bbox->prevPos[Z] = bbox->pos[Z];
	
}

//Usage:
// param X, Y, Z: The location of the box (center)
// param xrot, yrot, zrot: the rotation of the box.
// param r1x, r2y, r3z: the X, Y, and Z radius of the box.
// param bbox: the bounding box struct to be modified.
//Modified version to suit a Y axis gimbal lock. In other words, Y rotation is around a fixed post.
///Also, this doesn't make a box. It's just matrix generation, but with a radius and some velocities sprinkled on top; you can _make_ a box from it.
void	make2AxisBox(FIXED x, FIXED y, FIXED z, ANGLE xrot, ANGLE yrot, ANGLE zrot, FIXED r1x, FIXED r2y, FIXED r3z, _boundBox * bbox)
{
	FIXED prevXpos[XYZ]; 
	prevXpos[X] = bbox->Xplus[X];
	prevXpos[Y] = bbox->Xplus[Y];
	prevXpos[Z] = bbox->Xplus[Z];
	FIXED prevYpos[XYZ]; 
	prevYpos[X] = bbox->Yplus[X];
	prevYpos[Y] = bbox->Yplus[Y]; 
	prevYpos[Z] = bbox->Yplus[Z];
	FIXED prevZpos[XYZ];
	prevZpos[X] = bbox->Zplus[X];
	prevZpos[Y] = bbox->Zplus[Y];
	prevZpos[Z] = bbox->Zplus[Z];
	FIXED prevNXpos[XYZ];
	prevNXpos[X] = bbox->Xneg[X];
	prevNXpos[Y] = bbox->Xneg[Y];
	prevNXpos[Z] = bbox->Xneg[Z];
	FIXED prevNYpos[XYZ];
	prevNYpos[X] = bbox->Yneg[X]; 
	prevNYpos[Y] = bbox->Yneg[Y];
	prevNYpos[Z] = bbox->Yneg[Z];
	FIXED prevNZpos[XYZ];
	prevNZpos[X] = bbox->Zneg[X]; 
	prevNZpos[Y] = bbox->Zneg[Y];
	prevNZpos[Z] = bbox->Zneg[Z];
	//Give the box its location
	bbox->pos[X] = x;
	bbox->pos[Y] = y;
	bbox->pos[Z] = z;
	//Give the box its rotation
	bbox->boxRot[X] = xrot;
	bbox->boxRot[Y] = yrot;
	bbox->boxRot[Z] = zrot;
	//Give the box its radius
	bbox->brad[X] = r1x;
	bbox->brad[Y] = r2y;
	bbox->brad[Z] = r3z;

	register FIXED sinX = slSin(bbox->boxRot[X]);
	register FIXED cosX = slCos(bbox->boxRot[X]);
	register FIXED sinY = slSin(bbox->boxRot[Y]);
	register FIXED cosY = slCos(bbox->boxRot[Y]);
	register FIXED sinZ = slSin(bbox->boxRot[Z]);
	register FIXED cosZ = slCos(bbox->boxRot[Z]);
	//SETUP UNIT VECTOR X
	///left - right points. Affected as: X rotation causes no movement. Y rotation causes movement on X-Z axis. Z rotation causes movement on Y-X axis.
	bbox->UVX[X] = fxm(cosY, cosZ) - fxm(fxm(sinY, -sinX), -sinZ);
	bbox->UVX[Y] = fxm(sinZ, cosX);
	bbox->UVX[Z] = -fxm(sinY, cosZ) - fxm(fxm(-sinX, -sinZ), cosY);
	
	(*bbox).Xplus[X] = fxm((r1x), bbox->UVX[X]);
	(*bbox).Xplus[Y] = fxm((r1x), bbox->UVX[Y]);
	(*bbox).Xplus[Z] = fxm((r1x), bbox->UVX[Z]);

	//SETUP UNIT VECTOR Y
	///up - down points. Affected as: X rotation causes movement on Y-Z axis. Y rotation causes no movement. Z rotation causes movement on Y-X axis.
	bbox->UVY[X] = -fxm(fxm(sinY, sinX), cosZ) - fxm(sinZ, cosY);
	bbox->UVY[Y] = fxm(cosX, cosZ);
	bbox->UVY[Z] = fxm(fxm(-sinX, cosY), cosZ) + fxm(sinZ, sinY);

	(*bbox).Yplus[X] = fxm((r2y), bbox->UVY[X]);
	(*bbox).Yplus[Y] = fxm((r2y), bbox->UVY[Y]);
	(*bbox).Yplus[Z] = fxm((r2y), bbox->UVY[Z]);

	///Fwd - back points. Affected as: Y rotation causes movement on X-Z axis. X rotation causes movement on Y-Z. Z rotation causes no movement.
	//SETUP UNIT VECTOR Z
	bbox->UVZ[X] = fxm(sinY, cosX);
	bbox->UVZ[Y] = (sinX);
	bbox->UVZ[Z] = fxm(cosX, cosY);
	
	(*bbox).Zplus[X] = fxm((r3z), bbox->UVZ[X]);
	(*bbox).Zplus[Y] = fxm((r3z), bbox->UVZ[Y]);
	(*bbox).Zplus[Z] = fxm((r3z), bbox->UVZ[Z]);
	//---------------------------------------------------------------------------------------------------------------
	bbox->UVNX[X] = -bbox->UVX[X];
	bbox->UVNX[Y] = -bbox->UVX[Y];
	bbox->UVNX[Z] = -bbox->UVX[Z];
	bbox->UVNY[X] = -bbox->UVY[X];
	bbox->UVNY[Y] = -bbox->UVY[Y];
	bbox->UVNY[Z] = -bbox->UVY[Z];
	bbox->UVNZ[X] = -bbox->UVZ[X];
	bbox->UVNZ[Y] = -bbox->UVZ[Y];
	bbox->UVNZ[Z] = -bbox->UVZ[Z];
	//axis given: Y (on Z axis and does not change Y axis) circle going only right/left, forward/backward
	(*bbox).Xneg[X] = -fxm((r1x), bbox->UVX[X]);
	(*bbox).Xneg[Y] = -fxm((r1x), bbox->UVX[Y]);
	(*bbox).Xneg[Z] = -fxm((r1x), bbox->UVX[Z]);
	//axis given: X (on Y axis and does not change X axis) circle going only up/down, forward/backward
	(*bbox).Yneg[X] = -fxm((r2y), bbox->UVY[X]);
	(*bbox).Yneg[Y] = -fxm((r2y), bbox->UVY[Y]);
	(*bbox).Yneg[Z] = -fxm((r2y), bbox->UVY[Z]);
	//axis given: Z (on X axis and does not change Z axis) Circle going only up/down, left/right
	(*bbox).Zneg[X] = -fxm((r3z), bbox->UVZ[X]);
	(*bbox).Zneg[Y] = -fxm((r3z), bbox->UVZ[Y]);
	(*bbox).Zneg[Z] = -fxm((r3z), bbox->UVZ[Z]);
	//end of negative
	
	//Sort and assign X, Y, and Z maximum normals. (For macros)
	//Warning: Sorting is GONE. :(

	//Determine a velocity from the difference of current and last position
	segment_to_vector(bbox->prevPos, bbox->pos, bbox->velocity);
	segment_to_vector(prevXpos, bbox->Xplus, bbox->veloX);
	segment_to_vector(prevYpos, bbox->Yplus, bbox->veloY);
	segment_to_vector(prevZpos, bbox->Zplus, bbox->veloZ);
	
	segment_to_vector(prevNXpos, bbox->Xneg, bbox->veloNX);
	segment_to_vector(prevNYpos, bbox->Yneg, bbox->veloNY);
	segment_to_vector(prevNZpos, bbox->Zneg, bbox->veloNZ);
	
	//Fill this crap out.
	bbox->prevPos[X] = bbox->pos[X];
	bbox->prevPos[Y] = bbox->pos[Y];
	bbox->prevPos[Z] = bbox->pos[Z];
}

void	initPhys(void){
	for(Uint8 x = 0; x<MAX_PHYS_PROXY; x++){
		RBBs[x].isBoxPop = false;
		RBBs[x].velocity[X] = 0;
		RBBs[x].velocity[Y] = 0;
		RBBs[x].velocity[Z] = 0;
		
		RBBs[x].veloX[X] = 0;
		RBBs[x].veloX[Y] = 0;
		RBBs[x].veloX[Z] = 0;
		RBBs[x].veloNX[X] = 0;
		RBBs[x].veloNX[Y] = 0;
		RBBs[x].veloNX[Z] = 0;
		
		RBBs[x].veloY[X] = 0;
		RBBs[x].veloY[Y] = 0;
		RBBs[x].veloY[Z] = 0;
		RBBs[x].veloNY[X] = 0;
		RBBs[x].veloNY[Y] = 0;
		RBBs[x].veloNY[Z] = 0;

		RBBs[x].veloZ[X] = 0;
		RBBs[x].veloZ[Y] = 0;
		RBBs[x].veloZ[Z] = 0;
		RBBs[x].veloNZ[X] = 0;
		RBBs[x].veloNZ[Y] = 0;
		RBBs[x].veloNZ[Z] = 0;
	}
}
