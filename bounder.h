#pragma once


#define MAX_PHYS_PROXY (32)
#define BOXID_PLAYER	(-2)
#define BOXID_MAP		(-3)
#define BOXID_VOID		(-1)

#define EULER_OPTION_XZY	(0)
#define EULER_OPTION_XYZ	(1)
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
	FIXED nextPos[XYZ];
	FIXED prevPos[XYZ];
	FIXED velocity[XYZ];
	FIXED veloX[XYZ];
	FIXED veloY[XYZ];
	FIXED veloZ[XYZ];
	FIXED veloNX[XYZ];
	FIXED veloNY[XYZ];
	FIXED veloNZ[XYZ];
	FIXED renderScale[XYZ];
	//The box' radius
	FIXED brad[XYZ];
	//The box' rotation (expressed as ANGLE type data)
	ANGLE	boxRot[XYZ];
	//Expanded polygon/pointer data
	int pntbl[8][3];
	int * pltbl[6][4];
	int * nmtbl[6];
	int * cftbl[6];
	//Three separate flags to tell the engine if the box is populated, and ready to ..
	// [0] == 'R', render,
//	[1] == 'C', collide,
// [2] == 'L', light,
// [3] == 'B', collision box proxy ready
// [4] == 'S', box is receiving a scale
// [5] == 'r', was rendered, else 0.
// could be bitflags tbh
	char status[6];
	//ID# for collision. Usually throws the object # collided with. -1 signals no collision.
	short collisionID;
	//ID#. Used to verify the box object #. -1 signals the box is void.
	short boxID;
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
extern _boundBox BoundBoxDraw[MAX_PHYS_PROXY];
extern _boundBox * RBBs;
extern _boundBox * DBBs;
extern _boundBox pl_RBB;
extern _boundBox sl_RBB;

//------------------------------------------------------------------------------------
//FUNCTION SECTIONS FOR VARIOUS FILES
//------------------------------------------------------------------------------------

void	makeBoundBox(_object_arguments * source_data, int euler);
void	make2AxisBox(_object_arguments * source_data);
void	finalize_collision_proxy(_boundBox * box);
void	set_box_scale(_boundBox * box, int sx, int sy, int sz);
void	apply_box_scale(_boundBox * box);
void	initPhys(void);
void	flush_boxes(int start);


