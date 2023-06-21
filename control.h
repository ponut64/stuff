//
//control.h 
//include for use of control functions

#ifndef __CONTROL_H__
#define __CONTROL_H__

#define MAX_SPEED_FACTOR	(300)
#define STICK_MAX			(122)

extern int spdfactr;

extern int reval;
extern int target;

extern Bool holdCam;

void	controls(void);

#endif

