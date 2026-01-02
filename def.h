
#pragma once

//def.h -- the catch-all "i dunno where else this goes" files
#define true	(1)
#define false	(0)
//////////////////////////////////
// Uniform grid cell information / shorthands
//////////////////////////////////
//The cell size needs to be increased to 64. Or even 128.
#define CELL_SIZE (2621440) // 40 << 16
#define INV_CELL_SIZE (1638) // 40 / 1
#define CELL_SIZE_INT (40)
#define MAP_V_SCALE (17) //Map data is shifted left by this amount
//////////////////////////////////
#define	HIMEM	(100679680)
#define HWRAM_MODEL_DATA_HEAP_SIZE (150 * 1024)
//////////////////////////////////
#define UNCACHE (0x20000000)
#define VDP2_RAMBASE (0x25E00000)
#define LWRAM (0x200000)
#define LWRAM_END (LWRAM + 0x100000)
//////////////////////////////////
// Polygon draw direction flipping flags
//////////////////////////////////
#define FLIPV (32)
#define FLIPH (16)
#define FLIPHV (48)
//////////////////////////////////
// Fixed point safe-square value
//////////////////////////////////
#define SQUARE_MAX (9633792) //147<<16
////////////////////////////////// (64 is vaguely one meter)
#define PLAYER_X_SIZE	(32<<16)
#define PLAYER_Y_SIZE	(72<<16)
#define PLAYER_Z_SIZE	(32<<16)

//////////////////////////////////
#define LCL_MAP_PIX (21)
#define LCL_MAP_PLY (20)
//////////////////////////////////
//	Numerical Normal ID Shorthands
//////////////////////////////////
#define N_Xp (0)
#define N_Xn (1)
#define N_Yp (2)
#define N_Yn (3)
#define N_Zp (4)
#define N_Zn (5)
/////////////////////////////////
#define ANIM_SHIFT		(6)
#define ANIM_TIME(x)	fxdiv(fxdiv(16<<16,x<<16), 30<<16);

#define GRAVITY (6553)
#define MOVEMENT_DECAY_RATE (6000)

#define NUM_LEVELS	(9)

//Warning: 32 seems to be a max right now.
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
	FIXED renderScale[XYZ];
	//The box' radius
	FIXED radius[XYZ];
	//The box' rotation (expressed as ANGLE type data)
	ANGLE	boxRot[XYZ];
	//Expanded polygon/pointer data
	int pntbl[8][3];
	int * pltbl[6][4];
	int * nmtbl[6];
	int * cftbl[6];
	int	maxtbl[6];
	void * animation; //Current animation (if any - most objects are incompatible with this setting).
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
	//ID#. Stores the boxID of the object that this object is standing on.
	short surfID;
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


///////////////////////////////////
// Game logic timing data
///////////////////////////////////
extern int delta_time;
extern int time_in_seconds;
extern int bad_frames;
extern int framerate;
extern int time_delta_scale; //Will be HIGHER at LOWER FRAME-TIME (higher frame-rate)
extern int time_fixed_scale; //Will be LOWER at LOWER FRAME-TIME
//////////////////////////////////////////////////////////////////////////////
extern int flagIconTexno;


typedef struct {
	FIXED xp0[XYZ];
	FIXED xp1[XYZ];
	FIXED yp0[XYZ];
	FIXED yp1[XYZ];
	FIXED zp0[XYZ];
	FIXED zp1[XYZ];
} _lineTable;


//Structs
typedef struct {
	FIXED pos[XYZ];
	FIXED wpos[XYZ];
	int startPos[XYZ];
	int cellPos[XY];
	int dispPos[XY];
	int prevCellPos[XY];
	int prevDispPos[XY];
	FIXED prevPos[XYZ];
	int id;
	int rot[XYZ]; //It needs to be an INT... NO IDEA WHY...
	int rot2[XYZ];
	int renderRot[XYZ];
	int viewRot[XYZ];
	int rotState[XYZ];
	FIXED velocity[XYZ];
	int wvel[XYZ];
	int	dV[XYZ]; //Delta velocity (to be timescaled)
	FIXED gravAccel;
	int uview[XYZ];
	int hitscanPt[XYZ];
	int hitscanNm[XYZ];
	int viewPos[XYZ];
	int shootPos[XYZ];
	int shootDir[XYZ];
	FIXED ControlUV[XYZ];
	FIXED DirUV[XYZ];
	FIXED SurfUV[XYZ];
	FIXED IPaccel;
	
	int guidePos[3];//test value
	int viewmodel_offset[2];
	
	FIXED moment[XYZ];
	FIXED mass;
	short dirInp;
	short setJump;
	short okayStepSnd;
	short climbing;
	short ladder;
	short wasClimbing;
	short inMenu;
	short cancelTimers;
	short resetTimers;
	short jumpAllowed;
	int allowJumpTimer;
	int sanics;
	int avg_sanics;
	int sanic_samples;
	int end_average;
	int airTime;
	int parTime;
	int hasValidAim;
	
	short firstSurfHit;
	
	int surfaceHeight;
	int distanceToMapFloor;
	POINT	shadowPos;
	POINT	floorPos;
	POINT	wallPos;
	POINT	viewpoint;
	VECTOR	floorNorm;
	VECTOR	wallNorm;
	FIXED surfFriction;
	
	int score;
	int curSector;
	int prevSector;
	
	_lineTable fwd_world_faces;
	_lineTable bwd_world_faces;
	_lineTable time_axis;
	
	_lineTable realTimeAxis;
	
	_boundBox box;
	
	unsigned short actionKeyDef;
	
	Bool aboveObject;
	Bool hitMap;
	Bool hitObject;
	Bool hitBox;
	Bool hitSurface;
	Bool hitWall;
} _player;

//////////////////////////////////////////////////////////////////////////////
// Player data struct
//////////////////////////////////////////////////////////////////////////////
extern _player you;

extern int cutTex;

//Variables
extern POINT zPt;
extern POINT alwaysLow;
//Lives in main.c
extern void * HWRAM_ldptr;
extern void * HWRAM_hldptr;
extern int * zTable;
//System
extern unsigned char * dirty_buf;
extern unsigned char * dirtier_buf;
extern void * currentAddress;
extern volatile Uint32 * scuireg;
extern int levelPos[3];

void	p64MapRequest(short levelNo);

	/*
	
	SUBDIVISION TABLES
	+ rule:
	1 | 2
	- + -
	3 | 4
	
	- rule:
	1
	-
	2
	
	| rule:
	1	|	2
	
	How will the tables work?
	t_rules[224][4];
	Put in the texture # being subdivided, it spits out the texture # of the subdivisions.
	So, thusly, if you assign the texture # correctly, you can just walk that down to the final textures all the time.
	Now I just have to write this table back into C.
	
	In this system, I also want to allow tiled textures, in addition to UV cut textures.
	but I have to get the UV cut textures working properly first
	
	For UV cuts from 64x64
	1 = 8 (in u and v)
	
	Starting from:
	x1y1,x8y8 = 1 (downscaled),		+++	(64x64)
		1+:	30
		1+:	31
		1+:	32
		1+:	33
			
	x1y1,x4y8 = 2 (downscaled),		-++	(32x64)
		1-:	30
		1-: 32
			
	x5y1,x8y8 = 3 (downscaled),		-++
		1-: 31
		1-: 33
			
	x1y1,x8y4 = 4 (downscaled),		|++	(64x32)
		1|:	30
		1|:	31
			
	x1y5,x8y8 = 5 (downscaled),		|++
		1|:	32
		1|:	33
			
	x1y1,x2y8 = 6 (downscaled),		--+	(16x64)
		1-:	34
		1-:	38
			
	x3y1,x4y8 = 7 (downscaled),		--+
		1-:	32
		1-:	39
			
	x5y1,x6y8 = 8 (downscaled),		--+
		1-:	31
		1-:	36
			
	x7y1,x8y8 = 9 (downscaled),		--+
		1-:	37
		1-:	41
			
	x1y1,x8y2 = 10 (downscaled,		||+	(64x16)
		1|:	42
		1|:	44
	
	x1y3,x8y4 = 11 (downscaled),	||+
		1|:	43
		1|:	45
	
	x1y5,x8y6 = 12 (downscaled),	||+
		1|:	46
		1|:	48
	
	x1y7,x8y8 = 13 (downscaled),	||+
		1|:	47
		1|:	49
	
	x1y1,x1y8 = 14 (downscaled),	---	(8x64)
		1-: 50
		1-: 58
	x2y1,x2y8 = 15 (downscaled),	---
		1-: 51
		1-: 59
	x3y1,x3y8 = 16 (downscaled),	---
		1-: 52
		1-: 60
	x4y1,x4y8 = 17 (downscaled),	---
		1-: 53
		1-: 61
	x5y1,x5y8 = 18 (downscaled),	---
		1-: 54
		1-: 62
	x6y1,x6y8 = 19 (downscaled),	---
		1-: 55
		1-: 63
	x7y1,x7y8 = 20 (downscaled),	---
		1-: 56
		1-: 64
	x8y1,x8y8 = 21 (downscaled),	---	
		1-: 57
		1-: 65
	
	x1y1,x8y1 = 22 (downscaled),	|||	(64x8)
		1|: 66
		1|: 70
	x1y2,x8y2 = 23 (downscaled),	|||
		1|: 67
		1|: 71
	x1y3,x8y3 = 24 (downscaled),	|||
		1|: 68
		1|: 72
	x1y4,x8y4 = 25 (downscaled),	|||
		1|: 69
		1|: 73
	x1y5,x8y5 = 26 (downscaled),	|||
		1|: 74
		1|: 78
	x1y6,x8y6 = 27 (downscaled),	|||
		1|: 75
		1|: 79
	x1y7,x8y7 = 28 (downscaled),	|||
		1|: 76
		1|: 80
	x1y8,x8y8 = 29 (downscaled),	|||
		1|: 77
		1|: 81
	
	x1y1,x4y4 = 30,					++	(32x32)
		1+:	82
		1+: 83
		1+: 86
		1+: 87

	x5y1,x8y4 = 31,					++
		1+: 84
		1+: 85
		1+: 88
		1+: 89
	
	x1y5,x4y8 = 32,					++
		1+: 90
		1+: 91
		1+: 94
		1+: 95
	
	x5y5,x8y8 = 33,					++
		1+: 92
		1+: 93
		1+: 96
		1+: 97
	
	x1y1,x2y4 = 34,					-+	(16x32)
		1-: 82
		1-: 86
	x3y1,x4y4 = 35,					-+
		1-: 83
		1-: 87
	x5y1,x6y4 = 36,					-+
		1-: 84
		1-: 88
	x7y1,x8y4 = 37,					-+
		1-: 85
		1-: 89
	x1y5,x2y8 = 38,					-+
		1-: 90
		1-: 94
	x3y5,x4y8 = 39,					-+
		1-: 91
		1-: 95
	x5y5,x6y8 = 40,					-+
		1-: 92
		1-: 96
	x7y5,x8y8 = 41,					-+
		1-: 93
		1-: 97
	
	-------------------------------------------
	(alerta: zona de accidentes)
	x1y1,x4y2 = 42,					|+	(32x16)
		1|: 82
		1|: 83
	x1y3,x4y4 = 43,					|+
		1|: 86
		1|: 87
	x5y1,x8y2 = 44,					|+
		1|: 84
		1|: 85
	x5y3,x8y4 = 45,					|+
		1|: 88
		1|: 89
	x1y5,x4y6 = 46,					|+
		1|: 90
		1|: 91
	x1y7,x4y8 = 47,					|+
		1|: 94
		1|: 95
	x5y5,x8y6 = 48,					|+
		1|: 92
		1|: 93
	x5y7,x8y8 = 49,					|+
		1|: 96
		1|: 97
	--------------------------------------------
	x1y1,x1y4 = 50,					--	(8x32)
		1-: 98
		1-: 99
	x2y1,x2y4 = 51,					--
		1-: 100
		1-: 101
	x3y1,x3y4 = 52,					--
		1-: 102
		1-: 103
	x4y1,x4y4 = 53,					--
		1-: 104
		1-: 105
	x5y1,x5y4 = 54,					--
		1-: 106
		1-: 107
	x6y1,x6y4 = 55,					--
		1-: 108
		1-: 109
	x7y1,x7y4 = 56,					--
		1-: 110
		1-: 111
	x8y1,x8y4 = 57,					--
		1-: 112
		1-: 113
	x1y5,x1y8 = 58,					--
		1-: 114
		1-: 115
    x2y5,x2y8 = 59,					--
		1-: 116
		1-: 117
    x3y5,x3y8 = 60,					--
		1-: 118
		1-: 119
    x4y5,x4y8 = 61,					--
		1-: 120
		1-: 121
    x5y5,x5y8 = 62,					--
		1-: 122
		1-: 123
    x6y5,x6y8 = 63,					--
		1-: 124
		1-: 125
    x7y5,x7y8 = 64,					--
		1-: 126
		1-: 127
    x8y5,x8y8 = 65,					--
		1-: 128
		1-: 129
	--------------------------------------------
	alerta: zona de accidentes
	x1y1,x4y1 = 66,					||	(32x8)
		1|: 130
		1|: 132
	x1y2,x4y2 = 67,					||
		1|: 131
		1|: 133
	x1y3,x4y3 = 68,					||
		1|: 138
		1|: 140
	x1y4,x4y4 = 69,					||
		1|: 139
		1|: 141
	
	x5y1,x8y1 = 70,					||
		1|: 134
		1|: 136
	x5y2,x8y2 = 71,					||
		1|: 135
		1|: 137
	x5y3,x8y3 = 72,					||
		1|: 142
		1|: 144
	x5y4,x8y4 = 73,					||
		1|: 143
		1|: 145
	
	x1y5,x4y5 = 74,					||
		1|: 146
		1|: 148
    x1y6,x4y6 = 75,					||
		1|: 147
		1|: 149
    x1y7,x4y7 = 76,					||
		1|: 154
		1|: 156
    x1y8,x4y8 = 77,					||
		1|: 155
		1|: 157
	
    x5y5,x8y5 = 78,					||
		1|: 150
		1|: 152
    x5y6,x8y6 = 79,					||
		1|: 151
		1|: 153
    x5y7,x8y7 = 80,					||
		1|: 158
		1|: 160
    x5y8,x8y8 = 81,					||	
		1|: 159
		1|: 161
	--------------------------------------------
	x1y1,x2y2 = 82					+	(16x16)
	162, 166, 163, 167
	x3y1,x4y2 = 83					+
	170, 174, 171, 175
	x5y1,x6y2 = 84					+
	178, 182, 179, 183
	x7y1,x8y2 = 85					+
	186, 190, 187, 191
	x1y3,x2y4 = 86					+	
	164, 168, 165, 169
	x3y3,x4y4 = 87					+
	172, 176, 173, 177
	x5y3,x6y4 = 88					+
	180, 184, 181, 185
	x7y3,x8y4 = 89					+
	188, 192, 189, 193
	x1y5,x2y6 = 90					+	
	194, 198, 195, 199
	x3y5,x4y6 = 91					+
	202, 206, 203, 207
	x5y5,x6y6 = 92					+
	210, 214, 211, 215
	x7y5,x8y6 = 93					+
	218, 222, 219, 223
	x1y7,x2y8 = 94					+	
	196, 200, 197, 201
	x3y7,x4y8 = 95					+
	204, 208, 205, 209
	x5y7,x6y8 = 96					+
	212, 216, 213, 217
	x7y7,x8y8 = 97					+
	220, 224, 221, 225
	--------------------------------------------
	alerta: zona de accidentes
	
	x1y1,x1y2 = 98,					-	(8x16)
	162, 163
	x1y3,x1y4 = 99,					-
	164, 165
	x2y1,x2y2 = 100,				-
	166, 167
	x2y3,x2y4 = 101,				-	
	168, 169
	x3y1,x3y2 = 102,				-
	170, 171
	x3y3,x3y4 = 103,				-	
	172, 173
	x4y1,x4y2 = 104,				-
	174, 175
	x4y3,x4y4 = 105,				-	
	176, 177
	x5y1,x5y2 = 106,				-
	178, 179
	x5y3,x5y4 = 107,				-
	180, 181
	x6y1,x6y2 = 108,				-
	182, 183
	x6y3,x6y4 = 109,				-
	184, 185
	x7y1,x7y2 = 110,				-
	186, 187
	x7y3,x7y4 = 111,				-
	188, 189
	x8y1,x8y2 = 112,				-
	190, 191
	x8y3,x8y4 = 113,				-
	192, 193
	
	x1y5,x1y6 = 114,				-	
	194, 195
	x1y7,x1y8 = 115,				-
	196, 197
	x2y5,x2y6 = 116,				-
	198, 199
	x2y7,x2y8 = 117,				-	
	200, 201
	x3y5,x3y6 = 118,				-
	202, 203
	x3y7,x3y8 = 119,				-	
	204, 205
	x4y5,x4y6 = 120,				-
	206, 207
	x4y7,x4y8 = 121,				-	
	208, 209
	x5y5,x5y6 = 122,				-
	210, 211
	x5y7,x5y8 = 123,				-
	212, 213
	x6y5,x6y6 = 124,				-
	214, 215
	x6y7,x6y8 = 125,				-
	216, 217
	x7y5,x7y6 = 126,				-
	218, 219
	x7y7,x7y8 = 127,				-
	220, 221
	x8y5,x8y6 = 128,				-
	222, 223
	x8y7,x8y8 = 129,				-
	224, 225
	--------------------------------------------
	mora zona de accidentes
	
	x1y1,x2y1 = 130,				|	(16x8)
	162, 166
	x1y2,x2y2 = 131,				|
	163, 167
	x3y1,x4y1 = 132,				|
	170, 174
	x3y2,x4y2 = 133,				|	
	171, 175
	x5y1,x6y1 = 134,				|
	178, 182
	x5y2,x6y2 = 135,				|
	179, 183
	x7y1,x8y1 = 136,				|
	186, 190
	x7y2,x8y2 = 137,				|	
	187, 191

	x1y3,x2y3 = 138,				|
	164, 168
	x1y4,x2y4 = 139,				|
	165, 169
	x3y3,x4y3 = 140,				|
	172, 176
	x3y4,x4y4 = 141,				|
	173, 177
	x5y3,x6y3 = 142,				|
	180, 184
	x5y4,x6y4 = 143,				|
	181, 185
	x7y3,x8y3 = 144,				|
	188, 192
	x7y4,x8y4 = 145,				|
	189, 193

	x1y5,x2y5 = 146,				|
	194, 198
	x1y6,x2y6 = 147,				|
	195, 199
	x3y5,x4y5 = 148,				|
	202, 206
	x3y6,x4y6 = 149,				|
	203, 207
	x5y5,x6y5 = 150,				|
	210, 214
	x5y6,x6y6 = 151,				|
	211, 215
	x7y5,x8y5 = 152,				|
	218, 222
	x7y6,x8y6 = 153,				|
	219, 223
	
	x1y7,x2y7 = 154,				|
	196, 200
	x1y8,x2y8 = 155,				|
	197, 201
	x3y7,x4y7 = 156,				|
	204, 208
	x3y8,x4y8 = 157,				|
	205, 209
	x5y7,x6y7 = 158,				|
	212, 216
	x5y8,x6y8 = 159,				|
	213, 217
	x7y7,x8y7 = 160,				|
	220, 224
	x7y8,x8y8 = 161,				|
	221, 225
	
	--------------------------------------------
	mora zona de accidentes
	
	x1y1 = 162,
	x1y2 = 163,
	x1y3 = 164,
	x1y4 = 165,
	x2y1 = 166,
	x2y2 = 167,
	x2y3 = 168,
	x2y4 = 169,
	x3y1 = 170,
	x3y2 = 171,
	x3y3 = 172,
	x3y4 = 173,
	x4y1 = 174,
	x4y2 = 175,
	x4y3 = 176,
	x4y4 = 177,
	x5y1 = 178,
	x5y2 = 179,
	x5y3 = 180,
	x5y4 = 181,
	x6y1 = 182
	x6y2 = 183
	x6y3 = 184
	x6y4 = 185
	x7y1 = 186
	x7y2 = 187
	x7y3 = 188
	x7y4 = 189
	x8y1 = 190
	x8y2 = 191
	x8y3 = 192
	x8y4 = 193
	
	x1y5 = 194
	x1y6 = 195
	x1y7 = 196
	x1y8 = 197
	x2y5 = 198
	x2y6 = 199
	x2y7 = 200
	x2y8 = 201
	x3y5 = 202
	x3y6 = 203
	x3y7 = 204
	x3y8 = 205
	x4y5 = 206
	x4y6 = 207
	x4y7 = 208
	x4y8 = 209
	x5y5 = 210
	x5y6 = 211
	x5y7 = 212
	x5y8 = 213
	x6y5 = 214
	x6y6 = 215
	x6y7 = 216
	x6y8 = 217
	x7y5 = 218
	x7y6 = 219
	x7y7 = 220
	x7y8 = 221
	x8y5 = 222
	x8y6 = 223
	x8y7 = 224
	x8y8 = 225
	
	*/


