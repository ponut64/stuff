/*
This file is compiled separately.
*/

#include <sl_def.h>

#include "def.h"
#include "pcmsys.h"
#include "sound.h"
#include "mymath.h"
#include "input.h"

#include "pcmsys.h"
#include "render.h"
#include "particle.h"

#include "control.h"
#include "physobjet.h"

int spdfactr;
int fixCamRot;
int fixPlyrRot;
int fixCtrlRot;

_controlOptions usrCntrlOption = {.followForce = 1<<16, .cameraAccel = 45, .cameraCap = 0,
									.movementCam = 0, .facingCam = 1, .lockoutTime = 1<<16};

Bool holdCam = false;
Bool usePolyLine = false;

// D-PAD -> Move cardinally relative to camera (up -> fwd, right -> mov right, etc)
//		Y	
//	  A B C
//	are to be used as arrow key / WADS arrangement for camera movement.
void controls(void)
{
	if(is_key_down(DIGI_X)){
		you.rotState[X] -= usrCntrlOption.cameraAccel * framerate; //Look/turn left
		usrCntrlOption.lockTimer = usrCntrlOption.lockoutTime;
	}
	if(is_key_down(DIGI_B)){

		you.rotState[Y] -= usrCntrlOption.cameraAccel * framerate; //Look down
		usrCntrlOption.lockTimer = usrCntrlOption.lockoutTime;
	}
	if(is_key_down(DIGI_Z)){
		you.rotState[X] += usrCntrlOption.cameraAccel * framerate; //Look/turn right
		usrCntrlOption.lockTimer = usrCntrlOption.lockoutTime;
	}
	if (is_key_down(DIGI_Y))
    {
		you.rotState[Y] += usrCntrlOption.cameraAccel * framerate; //Look up
		usrCntrlOption.lockTimer = usrCntrlOption.lockoutTime;
    }
	
	//Before we try to handle directional input, first assume that there is none on this frame.
	you.dirInp = false;
	// deg * 182 = angle
	if(apd1.active != true)
	{
		////////////////////////////////////////////////////////////////////////////////
		//	DIGITAL PAD CONTROLS
		////////////////////////////////////////////////////////////////////////////////
		if(is_key_down(DIGI_UP) && is_key_down(DIGI_RIGHT)){
			you.rot2[Y] = (45 * 182);
			you.dirInp = true;
		} else if(is_key_down(DIGI_UP) && is_key_down(DIGI_LEFT)){
			you.rot2[Y] = (315 * 182);
			you.dirInp = true;
		} else if(is_key_down(DIGI_DOWN) && is_key_down(DIGI_RIGHT)){
			you.rot2[Y] = (135 * 182);
			you.dirInp = true;
		} else if(is_key_down(DIGI_DOWN) && is_key_down(DIGI_LEFT)){
			you.rot2[Y] = (225 * 182);
			you.dirInp = true;
		} else if(is_key_down(DIGI_UP)){
			you.rot2[Y] = 0;
			you.dirInp = true;
		} else if(is_key_down(DIGI_DOWN)){
			you.rot2[Y] = (180 * 182);
			you.dirInp = true;
		} else if(is_key_down(DIGI_LEFT)){
			you.rot2[Y] = (270 * 182);
			you.dirInp = true;
		} else if(is_key_down(DIGI_RIGHT)){
			you.rot2[Y] = (90 * 182);
			you.dirInp = true;
		}
		spdfactr = fxm(MAX_SPEED_FACTOR, time_fixed_scale);
	} else {
		////////////////////////////////////////////////////////////////////////////////
		// 3D PAD CONTROLS
		////////////////////////////////////////////////////////////////////////////////
		// The angle of the point (x,y) is the Y rotation axis, generally.
		// The magnitude of the point (x,y) is the "max throttle".
		// This is in a case where the center on each axis is 128. A small tolerance of +/- 4 (8 total) will be in place.
		
		//Shifting by 16 will overflow on square. Shifting by 15 will not. So we do 15.
		short sign_ax = (apd1.ax - 128);
		short sign_ay = -(apd1.ay - 128);
		//Centering the axis: Chop the axis (appropriately) to ensure it stays within +/- 120.
		// The axis is chopped to +/- 120 as a small tolerance for deadzone at the end of the stick.
		sign_ax = (sign_ax > STICK_MAX) ? STICK_MAX : sign_ax;
		sign_ax = (sign_ax < -STICK_MAX) ? -STICK_MAX : sign_ax;
		sign_ay = (sign_ay > STICK_MAX) ? STICK_MAX : sign_ay;
		sign_ay = (sign_ay < -STICK_MAX) ? -STICK_MAX : sign_ay;
		int control_axis_pt[3] = {sign_ax<<16, 0, sign_ay<<16};
		
	// nbg_sprintf(2, 11, "sx(%i)", sign_ax);
	// nbg_sprintf(2, 12, "sy(%i)", sign_ay);
		
		int norm_pt[3] = {0, 0, 0};
		
		accurate_normalize(control_axis_pt, norm_pt, 5);
		short angle = slAtan(norm_pt[Z], norm_pt[X]);
		
		// Next: Get the magnitude of each axis, relative to its (signed) maximum (STICK_MAX).
		int prop_ax = fxdiv(JO_ABS(control_axis_pt[X]), STICK_MAX<<16);
		int prop_ay = fxdiv(JO_ABS(control_axis_pt[Z]), STICK_MAX<<16);
		//We need to get the highest of these...
		int prop_max = JO_MAX(prop_ax, prop_ay);
		//"4096" is the deadzone.
		if(prop_max > 4096)
		{
		you.dirInp = true;
		spdfactr = fxm(fxm(prop_max, MAX_SPEED_FACTOR), time_fixed_scale);
		
		you.rot2[Y] = angle;
		}
	}
	
	//The game does weird stuff when master isn't busy for long enough for the controls handler to work.
	//Or... too busy? No idea...
	static int inputTimer;
	if(is_key_up(DIGI_C)) inputTimer = 0;
	if(is_key_down(DIGI_C) && inputTimer < 256)
	{
		int mark[3] = {0,0,0};
		mark[X] = -(you.shootDir[X]<<2);
		mark[Y] = -(you.shootDir[Y]<<2);
		mark[Z] = -(you.shootDir[Z]<<2);
		mark[X] += you.wvel[X];
		mark[Y] += you.wvel[Y];
		mark[Z] += you.wvel[Z];
		spawn_particle(&TestSpr, PROJ_TEST, you.shootPos, mark);
		
		for(int i = 0; i < MAX_PHYS_PROXY; i++)
		{
			_actor * act = &spawned_actors[i];
			
			if(act->info.flags.active)
			{
				actorPopulateGoalInfo(act, you.wpos, you.curSector);
			}
		}
		//you.guidePos[X] = you.wpos[X];//you.hitscanPt[X];
		//you.guidePos[Y] = you.wpos[Y];//you.hitscanPt[Y];
		//you.guidePos[Z] = you.wpos[Z];//you.hitscanPt[Z];
		inputTimer += delta_time;
	}
		
	you.rot[Y] = you.rot2[Y] - you.viewRot[Y];
	you.rot[Y] &= 0xFFFF;

	if(you.dirInp == true)
	{
		you.IPaccel += spdfactr<<2;
	}
		
		
	static FIXED rKeyTimer = 0;

	if(is_key_down(DIGI_A) ){
		if(rKeyTimer <= (66 / framerate))
		{ 
			if(you.jumpAllowed == true)
			{
				you.setJump = true;
				rKeyTimer += 50;
			}
			you.okayStepSnd = false;
		} else {
			you.okayStepSnd = true;
		}

		rKeyTimer++;
	} else {
		you.okayStepSnd = true;
		rKeyTimer = 0;
	}
	
	if(is_key_down(DIGI_R))
	{
		you.setJet = true;
	} else {
		you.setJet = false;
	}
	//

	if(is_key_down(DIGI_START)){
		//reset_player();
		you.inMenu = true;
	}
	
}


