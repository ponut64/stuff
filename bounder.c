/*
This file is compiled separately.
*/

///HIGH LEVEL ALERT///
/**THIS FUNCTION HAS HIGH PROBABILITY OF CAUSING CRITICAL GLITCHES**/
#include <sl_def.h>
#include "def.h"
#include "mymath.h"


#include "bounder.h"

_boundBox BoundBoxHost[MAX_PHYS_PROXY];
_boundBox BoundBoxDraw[MAX_PHYS_PROXY];
_boundBox * RBBs;
_boundBox * DBBs;
_boundBox pl_RBB;
_boundBox sl_RBB;
_object_arguments bound_box_starter;
Uint8 curBoxes = 0;

//Usage:
// param X, Y, Z: The location of the box (center)
// param xrot, yrot, zrot: the rotation of the box.
// param source_data->x_radius, source_data->y_radius, source_data->z_radius: the X, Y, and Z radius of the box.
//note: export models as -Y forward, Z up.
// param bbox: the bounding box struct to be modified.
// Something's wrong with this, because X+ is the same as X-. Do not use it.
void	makeBoundBox(_object_arguments * source_data, int euler)
{
	FIXED prevXpos[XYZ]; 
	prevXpos[X] = source_data->modified_box->Xplus[X];
	prevXpos[Y] = source_data->modified_box->Xplus[Y];
	prevXpos[Z] = source_data->modified_box->Xplus[Z];
	FIXED prevYpos[XYZ]; 
	prevYpos[X] = source_data->modified_box->Yplus[X];
	prevYpos[Y] = source_data->modified_box->Yplus[Y]; 
	prevYpos[Z] = source_data->modified_box->Yplus[Z];
	FIXED prevZpos[XYZ];
	prevZpos[X] = source_data->modified_box->Zplus[X];
	prevZpos[Y] = source_data->modified_box->Zplus[Y];
	prevZpos[Z] = source_data->modified_box->Zplus[Z];
	FIXED prevNXpos[XYZ];
	prevNXpos[X] = source_data->modified_box->Xneg[X];
	prevNXpos[Y] = source_data->modified_box->Xneg[Y];
	prevNXpos[Z] = source_data->modified_box->Xneg[Z];
	FIXED prevNYpos[XYZ];
	prevNYpos[X] = source_data->modified_box->Yneg[X]; 
	prevNYpos[Y] = source_data->modified_box->Yneg[Y];
	prevNYpos[Z] = source_data->modified_box->Yneg[Z];
	FIXED prevNZpos[XYZ];
	prevNZpos[X] = source_data->modified_box->Zneg[X]; 
	prevNZpos[Y] = source_data->modified_box->Zneg[Y];
	prevNZpos[Z] = source_data->modified_box->Zneg[Z];
	//Give the box its previous location
	source_data->modified_box->prevPos[X] = source_data->modified_box->pos[X];
	source_data->modified_box->prevPos[Y] = source_data->modified_box->pos[Y];
	source_data->modified_box->prevPos[Z] = source_data->modified_box->pos[Z];
	//Give the box its location
	source_data->modified_box->pos[X] = source_data->x_location;
	source_data->modified_box->pos[Y] = source_data->y_location;
	source_data->modified_box->pos[Z] = source_data->z_location;
	//Give the box its rotation
	source_data->modified_box->boxRot[X] = source_data->x_rotation;
	source_data->modified_box->boxRot[Y] = source_data->y_rotation;
	source_data->modified_box->boxRot[Z] = source_data->z_rotation;
	//Give the box its radius
	source_data->modified_box->brad[X] = source_data->x_radius;
	source_data->modified_box->brad[Y] = source_data->y_radius;
	source_data->modified_box->brad[Z] = source_data->z_radius;

	//SETUP UNIT VECTOR X
	int unitX[3] = {65536, 0, 0};
	int unitY[3] = {0, 65536, 0};
	int unitZ[3] = {0, 0, 65536};
	//Passing vector used as some RAM to throw the vector around safely.
	int unitP[3] = {0, 0, 0};
	if(euler == 0)
	{
	///////////////////////////////////////////////////////////////////////////
	// Matches XYZ Euler in Blender, but is actually XZY Euler.
	// Because Y and Z are swapped.
	///////////////////////////////////////////////////////////////////////////
	//Calculate UVX
	fxrotX(unitX, unitP, source_data->modified_box->boxRot[X]);
	fxrotZ(unitP, unitX, source_data->modified_box->boxRot[Z]);
	fxrotY(unitX, source_data->modified_box->UVX, source_data->modified_box->boxRot[Y]);
	//Calculate UVY
	fxrotX(unitY, unitP, source_data->modified_box->boxRot[X]);
	fxrotZ(unitP, unitY, source_data->modified_box->boxRot[Z]);
	fxrotY(unitY, source_data->modified_box->UVY, source_data->modified_box->boxRot[Y]);
	//Calculate UVZ
	fxrotX(unitZ, unitP, source_data->modified_box->boxRot[X]);
	fxrotZ(unitP, unitZ, source_data->modified_box->boxRot[Z]);
	fxrotY(unitZ, source_data->modified_box->UVZ, source_data->modified_box->boxRot[Y]);
	} else if(euler == 1)
	{
	///////////////////////////////////////////////////////////////////////////
	// YZX Euler
	///////////////////////////////////////////////////////////////////////////
	//Calculate UVX
	fxrotY(unitX, unitP, source_data->modified_box->boxRot[Y]);
	fxrotZ(unitP, unitX, source_data->modified_box->boxRot[Z]);
	fxrotX(unitX, source_data->modified_box->UVX, source_data->modified_box->boxRot[X]);
	//Calculate UVY
	fxrotY(unitY, unitP, source_data->modified_box->boxRot[Y]);
	fxrotZ(unitP, unitY, source_data->modified_box->boxRot[Z]);
	fxrotX(unitY, source_data->modified_box->UVY, source_data->modified_box->boxRot[X]);
	//Calculate UVZ
	fxrotY(unitZ, unitP, source_data->modified_box->boxRot[Y]);
	fxrotZ(unitP, unitZ, source_data->modified_box->boxRot[Z]);
	fxrotX(unitZ, source_data->modified_box->UVZ, source_data->modified_box->boxRot[X]);
	}
	
	
	source_data->modified_box->Xplus[X] = fxm((source_data->modified_box->brad[X]), source_data->modified_box->UVX[X]);
	source_data->modified_box->Xplus[Y] = fxm((source_data->modified_box->brad[X]), source_data->modified_box->UVX[Y]);
	source_data->modified_box->Xplus[Z] = fxm((source_data->modified_box->brad[X]), source_data->modified_box->UVX[Z]);

	source_data->modified_box->Yplus[X] = fxm((source_data->modified_box->brad[Y]), source_data->modified_box->UVY[X]);
	source_data->modified_box->Yplus[Y] = fxm((source_data->modified_box->brad[Y]), source_data->modified_box->UVY[Y]);
	source_data->modified_box->Yplus[Z] = fxm((source_data->modified_box->brad[Y]), source_data->modified_box->UVY[Z]);
	
	source_data->modified_box->Zplus[X] = fxm((source_data->modified_box->brad[Z]), source_data->modified_box->UVZ[X]);
	source_data->modified_box->Zplus[Y] = fxm((source_data->modified_box->brad[Z]), source_data->modified_box->UVZ[Y]);
	source_data->modified_box->Zplus[Z] = fxm((source_data->modified_box->brad[Z]), source_data->modified_box->UVZ[Z]);
	//---------------------------------------------------------------------------------------------------------------
	source_data->modified_box->UVNX[X] = -source_data->modified_box->UVX[X];
	source_data->modified_box->UVNX[Y] = -source_data->modified_box->UVX[Y];
	source_data->modified_box->UVNX[Z] = -source_data->modified_box->UVX[Z];
	source_data->modified_box->UVNY[X] = -source_data->modified_box->UVY[X];
	source_data->modified_box->UVNY[Y] = -source_data->modified_box->UVY[Y];
	source_data->modified_box->UVNY[Z] = -source_data->modified_box->UVY[Z];
	source_data->modified_box->UVNZ[X] = -source_data->modified_box->UVZ[X];
	source_data->modified_box->UVNZ[Y] = -source_data->modified_box->UVZ[Y];
	source_data->modified_box->UVNZ[Z] = -source_data->modified_box->UVZ[Z];
	//axis given: Y (on Z axis and does not change Y axis) circle going only right/left, forward/backward
	source_data->modified_box->Xneg[X] = -source_data->modified_box->Xplus[X];
	source_data->modified_box->Xneg[Y] = -source_data->modified_box->Xplus[Y];
	source_data->modified_box->Xneg[Z] = -source_data->modified_box->Xplus[Z];
	//axis given: X (on Y axis and does not change X axis) circle going only up/down, forward/backward
	source_data->modified_box->Yneg[X] = -source_data->modified_box->Yplus[X];
	source_data->modified_box->Yneg[Y] = -source_data->modified_box->Yplus[Y];
	source_data->modified_box->Yneg[Z] = -source_data->modified_box->Yplus[Z];
	//axis given: Z (on X axis and does not change Z axis) Circle going only up/down, left/right
	source_data->modified_box->Zneg[X] = -source_data->modified_box->Zplus[X];
	source_data->modified_box->Zneg[Y] = -source_data->modified_box->Zplus[Y];
	source_data->modified_box->Zneg[Z] = -source_data->modified_box->Zplus[Z];
	//end of negative

	//Determine a velocity from the difference of current and last position
	segment_to_vector(source_data->modified_box->prevPos, source_data->modified_box->pos, source_data->modified_box->velocity);
	segment_to_vector(prevXpos, source_data->modified_box->Xplus, source_data->modified_box->veloX);
	segment_to_vector(prevYpos, source_data->modified_box->Yplus, source_data->modified_box->veloY);
	segment_to_vector(prevZpos, source_data->modified_box->Zplus, source_data->modified_box->veloZ);
	
	segment_to_vector(prevNXpos, source_data->modified_box->Xneg, source_data->modified_box->veloNX);
	segment_to_vector(prevNYpos, source_data->modified_box->Yneg, source_data->modified_box->veloNY);
	segment_to_vector(prevNZpos, source_data->modified_box->Zneg, source_data->modified_box->veloNZ);
	
	source_data->modified_box->nextPos[X] = source_data->x_location + fxm(source_data->modified_box->velocity[X], time_fixed_scale);
	source_data->modified_box->nextPos[Y] = source_data->y_location + fxm(source_data->modified_box->velocity[Y], time_fixed_scale);
	source_data->modified_box->nextPos[Z] = source_data->z_location + fxm(source_data->modified_box->velocity[Z], time_fixed_scale);
	
}


//Usage:
// param X, Y, Z: The location of the box (center)
// param xrot, yrot, zrot: the rotation of the box.
// param source_data->x_radius, source_data->y_radius, source_data->z_radius: the X, Y, and Z radius of the box.
// param bbox: the bounding box struct to be modified.
//Modified version to suit a Y axis gimbal lock. In other words, Y rotation is around a fixed post.
///Also, this doesn't make a box. It's just matrix generation, but with a radius and some velocities sprinkled on top; you can _make_ a box from it.
void	make2AxisBox(_object_arguments * source_data)
{
	FIXED prevXpos[XYZ]; 
	prevXpos[X] = source_data->modified_box->Xplus[X];
	prevXpos[Y] = source_data->modified_box->Xplus[Y];
	prevXpos[Z] = source_data->modified_box->Xplus[Z];
	FIXED prevYpos[XYZ]; 
	prevYpos[X] = source_data->modified_box->Yplus[X];
	prevYpos[Y] = source_data->modified_box->Yplus[Y]; 
	prevYpos[Z] = source_data->modified_box->Yplus[Z];
	FIXED prevZpos[XYZ];
	prevZpos[X] = source_data->modified_box->Zplus[X];
	prevZpos[Y] = source_data->modified_box->Zplus[Y];
	prevZpos[Z] = source_data->modified_box->Zplus[Z];
	FIXED prevNXpos[XYZ];
	prevNXpos[X] = source_data->modified_box->Xneg[X];
	prevNXpos[Y] = source_data->modified_box->Xneg[Y];
	prevNXpos[Z] = source_data->modified_box->Xneg[Z];
	FIXED prevNYpos[XYZ];
	prevNYpos[X] = source_data->modified_box->Yneg[X]; 
	prevNYpos[Y] = source_data->modified_box->Yneg[Y];
	prevNYpos[Z] = source_data->modified_box->Yneg[Z];
	FIXED prevNZpos[XYZ];
	prevNZpos[X] = source_data->modified_box->Zneg[X]; 
	prevNZpos[Y] = source_data->modified_box->Zneg[Y];
	prevNZpos[Z] = source_data->modified_box->Zneg[Z];
	//Give the box its previous location
	source_data->modified_box->prevPos[X] = source_data->modified_box->pos[X];
	source_data->modified_box->prevPos[Y] = source_data->modified_box->pos[Y];
	source_data->modified_box->prevPos[Z] = source_data->modified_box->pos[Z];
	//Give the box its location
	source_data->modified_box->pos[X] = source_data->x_location;
	source_data->modified_box->pos[Y] = source_data->y_location;
	source_data->modified_box->pos[Z] = source_data->z_location;
	//Give the box its rotation
	source_data->modified_box->boxRot[X] = source_data->x_rotation;
	source_data->modified_box->boxRot[Y] = source_data->y_rotation;
	source_data->modified_box->boxRot[Z] = source_data->z_rotation;
	//Give the box its radius
	source_data->modified_box->brad[X] = source_data->x_radius;
	source_data->modified_box->brad[Y] = source_data->y_radius;
	source_data->modified_box->brad[Z] = source_data->z_radius;

	 FIXED sinX = slSin(source_data->modified_box->boxRot[X]);
	 FIXED cosX = slCos(source_data->modified_box->boxRot[X]);
	 FIXED sinY = slSin(source_data->modified_box->boxRot[Y]);
	 FIXED cosY = slCos(source_data->modified_box->boxRot[Y]);
	 FIXED sinZ = slSin(source_data->modified_box->boxRot[Z]);
	 FIXED cosZ = slCos(source_data->modified_box->boxRot[Z]);
	//SETUP UNIT VECTOR X
	///left - right points. Affected as: X rotation causes no movement. Y rotation causes movement on X-Z axis. Z rotation causes movement on Y-X axis.
	source_data->modified_box->UVX[X] = fxm(cosY, cosZ) - fxm(fxm(sinY, -sinX), -sinZ);
	source_data->modified_box->UVX[Y] = fxm(sinZ, cosX);
	source_data->modified_box->UVX[Z] = -fxm(sinY, cosZ) - fxm(fxm(-sinX, -sinZ), cosY);
	
	source_data->modified_box->Xplus[X] = fxm((source_data->x_radius), source_data->modified_box->UVX[X]);
	source_data->modified_box->Xplus[Y] = fxm((source_data->x_radius), source_data->modified_box->UVX[Y]);
	source_data->modified_box->Xplus[Z] = fxm((source_data->x_radius), source_data->modified_box->UVX[Z]);

	//SETUP UNIT VECTOR Y
	///up - down points. Affected as: X rotation causes movement on Y-Z axis. Y rotation causes no movement. Z rotation causes movement on Y-X axis.
	source_data->modified_box->UVY[X] = -fxm(fxm(sinY, sinX), cosZ) - fxm(sinZ, cosY);
	source_data->modified_box->UVY[Y] = fxm(cosX, cosZ);
	source_data->modified_box->UVY[Z] = fxm(fxm(-sinX, cosY), cosZ) + fxm(sinZ, sinY);

	source_data->modified_box->Yplus[X] = fxm((source_data->y_radius), source_data->modified_box->UVY[X]);
	source_data->modified_box->Yplus[Y] = fxm((source_data->y_radius), source_data->modified_box->UVY[Y]);
	source_data->modified_box->Yplus[Z] = fxm((source_data->y_radius), source_data->modified_box->UVY[Z]);

	///Fwd - back points. Affected as: Y rotation causes movement on X-Z axis. X rotation causes movement on Y-Z. Z rotation causes no movement.
	//SETUP UNIT VECTOR Z
	source_data->modified_box->UVZ[X] = fxm(sinY, cosX);
	source_data->modified_box->UVZ[Y] = (sinX);
	source_data->modified_box->UVZ[Z] = fxm(cosX, cosY);
	
	source_data->modified_box->Zplus[X] = fxm((source_data->z_radius), source_data->modified_box->UVZ[X]);
	source_data->modified_box->Zplus[Y] = fxm((source_data->z_radius), source_data->modified_box->UVZ[Y]);
	source_data->modified_box->Zplus[Z] = fxm((source_data->z_radius), source_data->modified_box->UVZ[Z]);
	//---------------------------------------------------------------------------------------------------------------
	source_data->modified_box->UVNX[X] = -source_data->modified_box->UVX[X];
	source_data->modified_box->UVNX[Y] = -source_data->modified_box->UVX[Y];
	source_data->modified_box->UVNX[Z] = -source_data->modified_box->UVX[Z];
	source_data->modified_box->UVNY[X] = -source_data->modified_box->UVY[X];
	source_data->modified_box->UVNY[Y] = -source_data->modified_box->UVY[Y];
	source_data->modified_box->UVNY[Z] = -source_data->modified_box->UVY[Z];
	source_data->modified_box->UVNZ[X] = -source_data->modified_box->UVZ[X];
	source_data->modified_box->UVNZ[Y] = -source_data->modified_box->UVZ[Y];
	source_data->modified_box->UVNZ[Z] = -source_data->modified_box->UVZ[Z];
	//axis given: Y (on Z axis and does not change Y axis) circle going only right/left, forward/backward
	source_data->modified_box->Xneg[X] = -source_data->modified_box->Xplus[X];
	source_data->modified_box->Xneg[Y] = -source_data->modified_box->Xplus[Y];
	source_data->modified_box->Xneg[Z] = -source_data->modified_box->Xplus[Z];
	//axis given: X (on Y axis and does not change X axis) circle going only up/down, forward/backward
	source_data->modified_box->Yneg[X] = -source_data->modified_box->Yplus[X];
	source_data->modified_box->Yneg[Y] = -source_data->modified_box->Yplus[Y];
	source_data->modified_box->Yneg[Z] = -source_data->modified_box->Yplus[Z];
	//axis given: Z (on X axis and does not change Z axis) Circle going only up/down, left/right
	source_data->modified_box->Zneg[X] = -source_data->modified_box->Zplus[X];
	source_data->modified_box->Zneg[Y] = -source_data->modified_box->Zplus[Y];
	source_data->modified_box->Zneg[Z] = -source_data->modified_box->Zplus[Z];
	//end of negative

	//Determine a velocity from the difference of current and last position
	segment_to_vector(source_data->modified_box->prevPos, source_data->modified_box->pos, source_data->modified_box->velocity);
	segment_to_vector(prevXpos, source_data->modified_box->Xplus, source_data->modified_box->veloX);
	segment_to_vector(prevYpos, source_data->modified_box->Yplus, source_data->modified_box->veloY);
	segment_to_vector(prevZpos, source_data->modified_box->Zplus, source_data->modified_box->veloZ);
	
	segment_to_vector(prevNXpos, source_data->modified_box->Xneg, source_data->modified_box->veloNX);
	segment_to_vector(prevNYpos, source_data->modified_box->Yneg, source_data->modified_box->veloNY);
	segment_to_vector(prevNZpos, source_data->modified_box->Zneg, source_data->modified_box->veloNZ);
	
	source_data->modified_box->nextPos[X] = source_data->x_location + fxm(source_data->modified_box->velocity[X], time_fixed_scale);
	source_data->modified_box->nextPos[Y] = source_data->y_location + fxm(source_data->modified_box->velocity[Y], time_fixed_scale);
	source_data->modified_box->nextPos[Z] = source_data->z_location + fxm(source_data->modified_box->velocity[Z], time_fixed_scale);
	
}

void	finalize_collision_proxy(_boundBox * box)
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
	
	box->nmtbl[0] = box->UVNY;
	box->nmtbl[1] = box->UVNZ;
	box->nmtbl[2] = box->UVNX;
	box->nmtbl[3] = box->UVY;
	box->nmtbl[4] = box->UVZ;
	box->nmtbl[5] = box->UVX;
	box->cftbl[0] = box->Yneg;
	box->cftbl[1] = box->Zneg;
	box->cftbl[2] = box->Xneg;
	box->cftbl[3] = box->Yplus;
	box->cftbl[4] = box->Zplus;
	box->cftbl[5] = box->Xplus;
	
	box->pntbl[0][X] = (box->Xneg[X] +  box->Yneg[X] + box->Zneg[X]		+ box->pos[X]);
	box->pntbl[0][Y] = (box->Xneg[Y] +  box->Yneg[Y] + box->Zneg[Y]		+ box->pos[Y]);
	box->pntbl[0][Z] = (box->Xneg[Z] +  box->Yneg[Z] + box->Zneg[Z]		+ box->pos[Z]);
	box->pntbl[1][X] = (box->Xplus[X] + box->Yneg[X] + box->Zneg[X]		+ box->pos[X]);
	box->pntbl[1][Y] = (box->Xplus[Y] + box->Yneg[Y] + box->Zneg[Y]		+ box->pos[Y]);
	box->pntbl[1][Z] = (box->Xplus[Z] + box->Yneg[Z] + box->Zneg[Z]		+ box->pos[Z]);
	box->pntbl[2][X] = (box->Xplus[X] + box->Yneg[X] + box->Zplus[X]	+ box->pos[X]);
	box->pntbl[2][Y] = (box->Xplus[Y] + box->Yneg[Y] + box->Zplus[Y]	+ box->pos[Y]);
	box->pntbl[2][Z] = (box->Xplus[Z] + box->Yneg[Z] + box->Zplus[Z]	+ box->pos[Z]);
	box->pntbl[3][X] = (box->Xneg[X] +  box->Yneg[X] + box->Zplus[X]	+ box->pos[X]);
	box->pntbl[3][Y] = (box->Xneg[Y] +  box->Yneg[Y] + box->Zplus[Y]	+ box->pos[Y]);
	box->pntbl[3][Z] = (box->Xneg[Z] +  box->Yneg[Z] + box->Zplus[Z]	+ box->pos[Z]);
	box->pntbl[4][X] = (box->Xneg[X] +  box->Yplus[X] + box->Zneg[X]	+ box->pos[X]);
	box->pntbl[4][Y] = (box->Xneg[Y] +  box->Yplus[Y] + box->Zneg[Y]	+ box->pos[Y]);
	box->pntbl[4][Z] = (box->Xneg[Z] +  box->Yplus[Z] + box->Zneg[Z]	+ box->pos[Z]);
	box->pntbl[5][X] = (box->Xplus[X] + box->Yplus[X] + box->Zneg[X]	+ box->pos[X]);
	box->pntbl[5][Y] = (box->Xplus[Y] + box->Yplus[Y] + box->Zneg[Y]	+ box->pos[Y]);
	box->pntbl[5][Z] = (box->Xplus[Z] + box->Yplus[Z] + box->Zneg[Z]	+ box->pos[Z]);
	box->pntbl[6][X] = (box->Xplus[X] + box->Yplus[X] + box->Zplus[X]	+ box->pos[X]);
	box->pntbl[6][Y] = (box->Xplus[Y] + box->Yplus[Y] + box->Zplus[Y]	+ box->pos[Y]);
	box->pntbl[6][Z] = (box->Xplus[Z] + box->Yplus[Z] + box->Zplus[Z]	+ box->pos[Z]);
	box->pntbl[7][X] = (box->Xneg[X] +  box->Yplus[X] + box->Zplus[X]	+ box->pos[X]);
	box->pntbl[7][Y] = (box->Xneg[Y] +  box->Yplus[Y] + box->Zplus[Y]	+ box->pos[Y]);
	box->pntbl[7][Z] = (box->Xneg[Z] +  box->Yplus[Z] + box->Zplus[Z]	+ box->pos[Z]);
	
	box->pltbl[0][0] = box->pntbl[3];	
	box->pltbl[0][1] = box->pntbl[2];
	box->pltbl[0][2] = box->pntbl[1];
	box->pltbl[0][3] = box->pntbl[0];
	box->pltbl[1][0] = box->pntbl[0];	
	box->pltbl[1][1] = box->pntbl[1];
	box->pltbl[1][2] = box->pntbl[5];
	box->pltbl[1][3] = box->pntbl[4];
	box->pltbl[2][0] = box->pntbl[0];	
	box->pltbl[2][1] = box->pntbl[4];
	box->pltbl[2][2] = box->pntbl[7];
	box->pltbl[2][3] = box->pntbl[3];
	box->pltbl[3][0] = box->pntbl[4];	
	box->pltbl[3][1] = box->pntbl[5];
	box->pltbl[3][2] = box->pntbl[6];
	box->pltbl[3][3] = box->pntbl[7];
	box->pltbl[4][0] = box->pntbl[3];	
	box->pltbl[4][1] = box->pntbl[7];
	box->pltbl[4][2] = box->pntbl[6];
	box->pltbl[4][3] = box->pntbl[2];
	box->pltbl[5][0] = box->pntbl[1];	
	box->pltbl[5][1] = box->pntbl[2];
	box->pltbl[5][2] = box->pntbl[6];
	box->pltbl[5][3] = box->pntbl[5];
	
	box->status[3] = 'B';
	
}

void	set_box_scale(_boundBox * box, int sx, int sy, int sz)
{
	box->status[4] = 'S';
	box->renderScale[X] = sx;
	box->renderScale[Y] = sy;
	box->renderScale[Z] = sz;
	//To scale a box, multiply through the matrix values (UVX,UVY,UVZ).
	//Scaling a box is only valid for part of one frame. Be aware of that.
}

void	apply_box_scale(_boundBox * box)
{
	//A scaled box is not valid for collision.
	//It could be made valid, but I don't want to.
	box->status[1] = 'N';
	box->UVX[X] = fxm(box->UVX[X], box->renderScale[X]);
	box->UVX[Y] = fxm(box->UVX[Y], box->renderScale[Y]);
	box->UVX[Z] = fxm(box->UVX[Z], box->renderScale[Z]);
	box->UVY[X] = fxm(box->UVY[X], box->renderScale[X]);
	box->UVY[Y] = fxm(box->UVY[Y], box->renderScale[Y]);
	box->UVY[Z] = fxm(box->UVY[Z], box->renderScale[Z]);
	box->UVZ[X] = fxm(box->UVZ[X], box->renderScale[X]);
	box->UVZ[Y] = fxm(box->UVZ[Y], box->renderScale[Y]);
	box->UVZ[Z] = fxm(box->UVZ[Z], box->renderScale[Z]);
}

void	flush_boxes(int start)
{
	
////////////////////////////////////////////////////
//Flush boxes
//Very important for making sure things don't render when they aren't supposed to.
////////////////////////////////////////////////////
// First step: Copy the master's working RBB to the slave's working DBB.
////////////////////////////////////////////////////
//Afterwards, purge the boxes and proceed with the rest of the code which will update the boxes.
////////////////////////////////////////////////////
	for(int i = start; i < MAX_PHYS_PROXY; i++)
	{
		//If you reach a box marked as void, stop. Don't need to go any further.
		if(RBBs[i].boxID == BOXID_VOID) break;
		RBBs[i].boxID = BOXID_VOID;
		RBBs[i].collisionID = BOXID_VOID;
		RBBs[i].status[0] = 'N';
		RBBs[i].status[1] = 'N';
		RBBs[i].status[2] = 'N';
		RBBs[i].status[3] = 'N';
		RBBs[i].status[4] = 'N';
		RBBs[i].status[5] = 'N';
	}
////////////////////////////////////////////////////
	
}

void	initPhys(void){
		//The bound box / matrix parameter struct has to be uncached, since it is shared between master & slave.
		// But don't be stupid like I was for a long time. See, I used to put it in LWRAM. LWRAM is SLOW!!!!
		RBBs = (_boundBox *)((unsigned int)(&BoundBoxHost[0]));
		DBBs = (_boundBox *)((unsigned int)(&BoundBoxDraw[0]));
	for(Uint8 x = 0; x<MAX_PHYS_PROXY; x++){
		RBBs[x].status[0] = 'N';
		RBBs[x].status[1] = 'N';
		RBBs[x].status[2] = 'N';
		RBBs[x].status[3] = 'N';
		RBBs[x].status[4] = 'N';
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
		RBBs[x].boxID = -1;
		RBBs[x].collisionID = -1;
	}
}
