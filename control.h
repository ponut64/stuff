//
//control.h 
//include for use of control functions

#ifndef __CONTROL_H__
#define __CONTROL_H__

#define MAX_SPEED_FACTOR	(300)
#define STICK_MAX			(122)

//Can also add deadzone and allow/unallow analog lock-out here.
typedef struct {
	int followForce;
	int cameraAccel;
	int cameraCap;
	int movementCam;
	int facingCam;
	int lockoutTime;
	int lockTimer;
	int lockout;
} _controlOptions;

extern _controlOptions usrCntrlOption;

extern int spdfactr;

extern int reval;
extern int target;

extern Bool holdCam;

void	controls(void);

#endif

