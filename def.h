
#ifndef __DEF_H__
# define __DEF_H__

//def.h -- the catch-all "i dunno where else this goes" file
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
	FIXED Velocity[XYZ];
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

//////////////////////////////////////////////////////////////////////////////
// Player data struct
//////////////////////////////////////////////////////////////////////////////
extern _player you;

//Variables
extern bool usePolyLine;
extern POINT zPt;
//Lives in main.c
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

