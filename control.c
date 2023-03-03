/*
This file is compiled separately.
*/

#include <sl_def.h>

#include "def.h"
#include "mymath.h"
#include "input.h"
#include "bounder.h"
#include "player_phy.h"

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
		you.rotState[X] -= 91 * framerate; //Look/turn left
	}
	if(is_key_down(DIGI_B)){

		you.rotState[Y] -= 91 * framerate; //Look down
	}
	if(is_key_down(DIGI_Z)){
		you.rotState[X] += 91 * framerate; //Look/turn right
	}
    if (is_key_down(DIGI_X))
    {
		
    }
	if (is_key_down(DIGI_Y))
    {
		you.rotState[Y] += 91 * framerate; //Look up
    }
	if(is_key_down(DIGI_Z)){

	}

	// deg * 182 = angle
		you.dirInp = false;
	if(is_key_down(DIGI_UP) && is_key_down(DIGI_RIGHT)){
		you.rot[Y] = (45 * 182) - you.viewRot[Y]; 
		you.rot2[Y] = (45 * 182);
		you.dirInp = true;
	} else if(is_key_down(DIGI_UP) && is_key_down(DIGI_LEFT)){
		you.rot[Y] = -(45 * 182) - you.viewRot[Y]; 
		you.rot2[Y] = -(45 * 182);
		you.dirInp = true;
	} else if(is_key_down(DIGI_DOWN) && is_key_down(DIGI_RIGHT)){
		you.rot[Y] = (135 * 182) - you.viewRot[Y]; 
		you.rot2[Y] = (135 * 182);
		you.dirInp = true;
	} else if(is_key_down(DIGI_DOWN) && is_key_down(DIGI_LEFT)){
		you.rot[Y] = -(135 * 182) - you.viewRot[Y]; 
		you.rot2[Y] = -(135 * 182);
		you.dirInp = true;
	} else if(is_key_down(DIGI_UP)){
		you.rot[Y] = -you.viewRot[Y]; 
		you.rot2[Y] = 0;
		you.dirInp = true;
	} else if(is_key_down(DIGI_DOWN)){
		you.rot[Y] = -(180 * 182) - you.viewRot[Y]; 
		you.rot2[Y] = -(180 * 182);
		you.dirInp = true;
	} else if(is_key_down(DIGI_LEFT)){
		you.rot[Y] = -(90 * 182) - you.viewRot[Y]; 
		you.rot2[Y] = -(90 * 182);
		you.dirInp = true;
	} else if(is_key_down(DIGI_RIGHT)){
		you.rot[Y] = (90 * 182) - you.viewRot[Y]; 
		you.rot2[Y] = (90 * 182);
		you.dirInp = true;
	}
	
	
		spdfactr = fxm(300, frmul);
	if(you.dirInp == true){
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


	if(is_key_pressed(DIGI_START)){
		reset_player();
	}
	
}

void	mypad(void)
{
	
	controls();
}

