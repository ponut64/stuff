
#ifndef __DEF_H__
# define __DEF_H__

//def.h -- the catch-all "i dunno where else this goes" file
#include "timer.h"
#include "input.h"
#include "pcmsys.h"

#define CELL_SIZE (25<<16)
#define INV_CELL_SIZE (slDivFX(25<<16, 1<<16))
#define CELL_SIZE_INT (25)
#define UNCACHE (0x20000000)
#define VDP2_RAMBASE (0x25E00000)
#define LWRAM	(2097152)
#define FLIPV (32)
#define FLIPH (16)
#define FLIPHV (48)
#define SQUARE_MAX (9633792) //147<<16
#define LCL_MAP_PIX (25)
#define LCL_MAP_PLY (24)

//////////////////////////////////////////////////////////////////////////////
//Sound Numbers
//////////////////////////////////////////////////////////////////////////////
int snd_lstep;
int snd_wind;
int snd_bstep;
int snd_click;
int snd_button;
int snd_cronch;
int snd_alarm;
int snd_win;
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
	FIXED Velocity[XYZ];
	FIXED gravAccel;
	FIXED Accel[XYZ];
	FIXED Force[XYZ];
	FIXED ControlUV[XYZ];
	FIXED DirUV[XYZ];
	FIXED SurfUV[XYZ];
	FIXED strafeState;
	FIXED IPaccel;
	
	FIXED moment[XYZ];
	FIXED mass;
	bool dirInp;
	bool setJump;
	bool setSlide;
	bool okayStepSnd;
	int sanics;
	
	int surfaceHeight;
	POINT	shadowPos;
	POINT	floorPos;
	POINT	wallPos;
	VECTOR	floorNorm;
	VECTOR	wallNorm;
	FIXED surfFriction;
	
	int points;
	
	bool aboveObject;
	bool hitMap;
	bool hitSurface;
	bool hitWall;
	Bool onSurface;
} _player;

_player you;

//Variables
extern bool usePolyLine;
extern POINT zPt;
//Lives in main.c
//System
extern unsigned char * dirty_buf;
extern int framerate;
extern int frmul;
extern volatile Uint32 * scuireg;

typedef struct
{
	Bool uniform;
	Uint8 arate[256];
    Uint16 currentFrm;
    Uint8 currentKeyFrm;
    Uint8 startFrm;
    Uint8 endFrm;
} animationControl;

animationControl forward;
animationControl run;
animationControl dbound;
animationControl jump;
animationControl fall;
animationControl idle;
animationControl stop;

animationControl jump2;
animationControl airIdle;
animationControl airRight;
animationControl airLeft;
animationControl airBack;
animationControl slideIdle;
animationControl slideFwd;
animationControl slideRvs;
animationControl slideRln;
animationControl slideLln;


#endif

