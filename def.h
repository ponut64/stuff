
#ifndef __DEF_H__
# define __DEF_H__

#define CELL_SIZE (25<<16)
#define INV_CELL_SIZE (slDivFX(25<<16, 1<<16))
#define CELL_SIZE_INT (25)
#define UNCACHE (0x20000000)
#define FLIPV (32)
#define FLIPH (16)
#define FLIPHV (48)
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
	
	jo_camera rcam;
	
	int surfaceHeight;
	POINT	shadowPos;
	POINT	floorPos;
	POINT	wallPos;
	VECTOR	floorNorm;
	VECTOR	wallNorm;
	FIXED surfFriction;
	
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
extern Sint32 framerate;
extern Sint8 SynchConst;
extern FIXED outTime;
extern int lasttime;
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

