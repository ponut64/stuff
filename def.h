
#ifndef __DEF_H__
# define __DEF_H__

//def.h -- the catch-all "i dunno where else this goes" files
#define true	(1)
#define false	(0)
//////////////////////////////////
// Uniform grid cell information / shorthands
//////////////////////////////////
#define CELL_SIZE (2621440) // 40 << 16
#define INV_CELL_SIZE (1638) // 40 / 1
#define CELL_SIZE_INT (40)
#define MAP_V_SCALE (17) //Map data is shifted left by this amount
//////////////////////////////////
#define	HIMEM	(100679680)
#define HWRAM_MODEL_DATA_HEAP_SIZE (256 * 1024)
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
//////////////////////////////////

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

#define GRAVITY (6553)

#define NUM_LEVELS	(8)
///////////////////////////////////
// Game logic timing data
///////////////////////////////////
extern int delta_time;
extern int time_in_seconds;
//////////////////////////////////////////////////////////////////////////////
extern int flagIconTexno;

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
	ANGLE viewRot[XYZ];
	int rotState[XYZ];
	FIXED velocity[XYZ];
	FIXED gravAccel;
	FIXED Accel[XYZ];
	FIXED Force[XYZ];
	FIXED ControlUV[XYZ];
	FIXED DirUV[XYZ];
	FIXED SurfUV[XYZ];
	FIXED IPaccel;
	
	FIXED moment[XYZ];
	FIXED mass;
	Bool dirInp;
	Bool setJump;
	Bool setSlide;
	Bool okayStepSnd;
	Bool climbing;
	Bool ladder;
	Bool wasClimbing;
	Bool inMenu;
	int sanics;
	
	short power;
	short maxPower;
	Bool setJet;
	
	int surfaceHeight;
	POINT	shadowPos;
	POINT	floorPos;
	POINT	wallPos;
	POINT	viewpoint;
	VECTOR	floorNorm;
	VECTOR	wallNorm;
	FIXED surfFriction;
	
	int points;
	
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

//Variables
extern POINT zPt;
extern POINT alwaysLow;
//Lives in main.c
extern short * division_table;
//extern short * sine_table;
extern void * HWRAM_ldptr;
extern void * HWRAM_hldptr;
//System
extern unsigned char * dirty_buf;
extern void * currentAddress;
extern int framerate;
extern int frmul;
extern volatile Uint32 * scuireg;

#endif

