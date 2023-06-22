/*
This file is compiled separately.
*/

#include <sl_def.h>

#include "def.h"
#include "mymath.h"
#include "input.h"
#include "bounder.h"
#include "player_phy.h"
#include "pcmsys.h"
#include "render.h"

#include "control.h"

int spdfactr;

Bool holdCam = false;
Bool usePolyLine = false;

// D-PAD -> Move cardinally relative to camera (up -> fwd, right -> mov right, etc)
//		Y	
//	  A B C
//	are to be used as arrow key / WADS arrangement for camera movement.
void controls(void)
{
	if(is_key_down(DIGI_X)){
		you.rotState[X] -= 45 * framerate; //Look/turn left
	}
	if(is_key_down(DIGI_B)){

		you.rotState[Y] -= 45 * framerate; //Look down
	}
	if(is_key_down(DIGI_Z)){
		you.rotState[X] += 45 * framerate; //Look/turn right
	}
    if (is_key_down(DIGI_X))
    {
		
    }
	if (is_key_down(DIGI_Y))
    {
		you.rotState[Y] += 45 * framerate; //Look up
    }
	if(is_key_down(DIGI_Z)){

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
		
		accurate_normalize(control_axis_pt, norm_pt);
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
	
		
	if(you.dirInp == true){
		you.rot[Y] = you.rot2[Y] - you.viewRot[Y];
		you.IPaccel += spdfactr;
	}
		
	if(is_key_down(DIGI_L) ){
		you.setSlide = true;
		//you.rot[Y] = -you.viewRot[Y];
	} else {
		you.setSlide = false;
	}
	static FIXED rKeyTimer = 0;

	if(is_key_down(DIGI_A) ){
		if(rKeyTimer <= (66 / framerate)){ 
			if(you.hitSurface == true){
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


