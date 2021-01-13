
#ifndef __DEF_H__
# define __DEF_H__

//def.h -- the catch-all "i dunno where else this goes" file
#include "input.h"
#include "pcmsys.h"

//////////////////////////////////
// Uniform grid cell information / shorthands
//////////////////////////////////
#define CELL_SIZE (2621440) // 40 << 16
#define INV_CELL_SIZE (1638) // 40 / 1
#define CELL_SIZE_INT (40)
#define MAP_V_SCALE (17) //Map data is shifted left by this amount
//////////////////////////////////
#define	HIMEM	(100679680)
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
//	The line width of the polygon map area, in vertices (pix) and polygons (ply).
//	The total size is the square of these values.
//////////////////////////////////
#define LCL_MAP_PIX (17)
#define LCL_MAP_PLY (16)
//////////////////////////////////
//	Numerical Normal ID Shorthands
//////////////////////////////////
#define N_Xp (0)
#define N_Xn (1)
#define N_Yp (2)
#define N_Yn (3)
#define N_Zp (4)
#define N_Zn (5)
//////////////////////////////////////////////////////////////////////////////
//Sound Numbers
//////////////////////////////////////////////////////////////////////////////
extern int snd_dash;
extern int snd_lstep;
extern int snd_wind;
extern int snd_bstep;
extern int snd_click;
extern int snd_button;
extern int snd_cronch;
extern int snd_alarm;
extern int snd_win;
extern int snd_bwee;
//////////////////////////////////////////////////////////////////////////////

//Structs
typedef struct {
	FIXED pos[XYZ];
	int cellPos[XY];
	int dispPos[XY];
	int prevCellPos[XY];
	int prevDispPos[XY];
	FIXED prevPos[XYZ];
	int id;
	int rot[XYZ]; //It needs to be an INT... NO IDEA WHY...
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
	bool dirInp;
	bool setJump;
	bool setSlide;
	bool okayStepSnd;
	int sanics;
	
	int power;
	bool setJet;
	
	int surfaceHeight;
	POINT	shadowPos;
	POINT	floorPos;
	POINT	wallPos;
	POINT	viewpoint;
	VECTOR	floorNorm;
	VECTOR	wallNorm;
	FIXED surfFriction;
	
	int points;
	
	bool aboveObject;
	bool hitMap;
	bool hitObject;
	bool hitBox;
	bool hitSurface;
	bool hitWall;
} _player;

//////////////////////////////////////////////////////////////////////////////
// Player data struct
//////////////////////////////////////////////////////////////////////////////
extern _player you;

//Variables
extern bool usePolyLine;
extern POINT zPt;
//Lives in main.c
extern short * division_table;
//System
extern unsigned char * dirty_buf;
extern void * currentAddress;
extern int framerate;
extern int frmul;
extern volatile Uint32 * scuireg;

typedef struct
{
	Bool uniform;
	Uint8 arate[64];
    Uint16 currentFrm;
    Uint8 currentKeyFrm;
    Uint8 startFrm;
    Uint8 endFrm;
} animationControl;

extern animationControl walk;
extern animationControl run;
extern animationControl dbound;

extern animationControl runshoot;
extern animationControl runmelee;

extern animationControl melee;
extern animationControl shoot;

extern animationControl idle;

extern animationControl jump;
extern animationControl stop;

extern animationControl airShoot;
extern animationControl airMelee;

extern animationControl airIdle;
extern animationControl airRight;
extern animationControl airLeft;
extern animationControl slideIdle;
extern animationControl slideRln;
extern animationControl slideLln;


#endif

