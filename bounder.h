#ifndef __BOUNDER_H__
#define __BOUNDER_H__


#define MAX_PHYS_PROXY (32)
//Structs

typedef struct {
////////////////////////////////////////////
// Pre-matrix section
// The following four arrays complete the 3D matrix representing the object's orientation and position.
////////////////////////////////////////////
	//Positive orientation angles
	FIXED UVX[XYZ];
	FIXED UVY[XYZ];
	FIXED UVZ[XYZ];
	//The boxes center [XYZ]
	FIXED pos[XYZ];
////////////////////////////////////////////
	//Negative orientation angles
	FIXED UVNX[XYZ];
	FIXED UVNY[XYZ];
	FIXED UVNZ[XYZ];
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
	//The box' radius
	FIXED brad[XYZ];
	//The box' rotation (expressed as ANGLE type data)
	ANGLE	boxRot[XYZ];
	//Three separate flags to tell the engine if the box is populated, and ready to ..
	// [0] == 'R', render, [1] == 'C', collide, [2] == 'L', light.
	char status[3];
	//ID
	Uint8 boxID;
} _boundBox;

typedef struct {
	int x_location;
	int y_location;
	int z_location;
	short x_rotation;
	short y_rotation;
	short z_rotation;
	int x_radius;
	int y_radius;
	int z_radius;
	_boundBox * modified_box;
} _object_arguments;

extern _object_arguments bound_box_starter;

extern _boundBox BoundBoxHost[MAX_PHYS_PROXY];
extern _boundBox * RBBs;
extern _boundBox pl_RBB;

//------------------------------------------------------------------------------------
//FUNCTION SECTIONS FOR VARIOUS FILES
//------------------------------------------------------------------------------------

void	makeBoundBox(_object_arguments * source_data);
void	make2AxisBox(_object_arguments * source_data);
void	initPhys(void);
void	flush_boxes(int start);

#endif

