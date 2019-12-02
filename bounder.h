#ifndef __BOUNDER_H__
# define __BOUNDER_H__

#include <jo/jo.h>
#include "mymath.h"
#include "control.h"
#include "def.h"

#define MAX_PHYS_PROXY (16)
//Structs

typedef struct {
	//Six axis normal vector star.
	FIXED UVX[XYZ];
	FIXED UVY[XYZ];
	FIXED UVZ[XYZ];
	FIXED UVNX[XYZ];
	FIXED UVNY[XYZ];
	FIXED UVNZ[XYZ];
	//Matrix Axis (calculated from inverted angles)
	FIXED maX[XYZ];
	FIXED maY[XYZ];
	FIXED maZ[XYZ];
	//The center-faces.
	FIXED Xplus[XYZ];
	FIXED Xneg[XYZ];
	FIXED Yplus[XYZ];
	FIXED Yneg[XYZ];
	FIXED Zplus[XYZ];
	FIXED Zneg[XYZ];
	//Velocities of the various center-faces & box.
	//
	FIXED prevPos[XYZ];
	FIXED velocity[XYZ];
	FIXED veloX[XYZ];
	FIXED veloY[XYZ];
	FIXED veloZ[XYZ];
	FIXED veloNX[XYZ];
	FIXED veloNY[XYZ];
	FIXED veloNZ[XYZ];
	//The boxes center [XYZ]
	FIXED pos[XYZ];
	//The box' radius
	FIXED brad[XYZ];
	//The box' rotation (expressed as ANGLE type data)
	ANGLE	boxRot[XYZ];
	//A flag for whether or not this box data is populated.
	Bool isBoxPop;
	//ID
	Uint8 boxID;
} _boundBox;


extern _boundBox RBBs[MAX_PHYS_PROXY];
extern _boundBox pl_RBB;

//------------------------------------------------------------------------------------
//FUNCTION SECTIONS FOR VARIOUS FILES
//------------------------------------------------------------------------------------

void	makeBoundBox(FIXED x, FIXED y, FIXED z, ANGLE xrot, ANGLE yrot, ANGLE zrot, FIXED r1x, FIXED r2y, FIXED r3z, _boundBox * ModifiedBox);
void	make2AxisBox(FIXED x, FIXED y, FIXED z, ANGLE xrot, ANGLE yrot, ANGLE zrot, FIXED r1x, FIXED r2y, FIXED r3z, _boundBox * bbox);
void	initPhys(void);


#endif

