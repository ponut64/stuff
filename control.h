//
//control.h 
//include for use of control functions

#ifndef __CONTROL_H__
# define __CONTROL_H__

#include "def.h"

extern int spdfactr;

extern Sint32 testNum[XYZ];

extern int reval;
extern int target;

extern Bool holdCam;

void	reset_player(void);

void	controls(void);

void	mypad(void);

#endif

