/*
This file is compiled separately.
*/

#include <jo/jo.h>

#include "physobjet.h"
#include "bounder.h"
#include "control.h"
#include "mymath.h"
#include "msfs.h"

Sint32 testNum[XYZ];
int spdfactr;

Bool holdCam = false;
bool usePolyLine = false;

void reset_player(void)
{
	
	testNum[X] = 0;
	testNum[Y] = 0;
	testNum[Z] = 0;
	
	you.mass = 250<<16;
	
	hmap_matrix_pos[X] = 0;
	hmap_matrix_pos[Z] = 0;
	you.onSurface = false;
	you.hitMap = false;
	you.hitSurface = false;
	you.hitWall = false;
	you.okayStepSnd = true;
    you.pos[X]=0;
    you.pos[Y]=(255<<16);
    you.pos[Z]=0;
    you.prevPos[X]=0;
    you.prevPos[Y]=(255<<16);
    you.prevPos[Z]=0;
    you.rot[X]=0;
    you.rot[Y]=0;
    you.rot[Z]=0;
    you.viewRot[X]=0;
    you.viewRot[Y]=0;
    you.viewRot[Z]=0;
    you.moment[X]=0;
    you.moment[Y]=0;
    you.moment[Z]=0;
    you.Velocity[X]=0;
    you.Velocity[Y]=0;
    you.Velocity[Z]=0;
    you.Force[X]=0;
    you.Force[Y]=0;
    you.Force[Z]=0;
	you.IPaccel=0;
	you.id = 0;
}

// D-PAD -> Move cardinally relative to camera (up -> fwd, right -> mov right, etc)
//		Y	
//	  A B C
//	are to be used as arrow key / WADS arrangement for camera movement.
void controls(void)
{
	Uint16 id = you.id;

	if(jo_is_input_key_pressed(id, JO_KEY_A )){
		you.rotState[X] -=91 * framerate;
	}
	if(jo_is_input_key_pressed(id, JO_KEY_B) ){

		you.rotState[Y] -=91 * framerate;
	}
	if(jo_is_input_key_pressed(id, JO_KEY_C)){
		you.rotState[X] +=91 * framerate;
	}
    if (jo_is_input_key_pressed(id, JO_KEY_X))
    {
		// SynchConst = 3;
		usePolyLine = false;
		// testNum[X]+=91;
    }
	if (jo_is_input_key_pressed(id, JO_KEY_Y))
    {
		you.rotState[Y] +=91 * framerate;
    }
	if(jo_is_input_key_pressed(id, JO_KEY_Z)){
		// SynchConst = 1;
		usePolyLine = true;
		// testNum[X]-=91;
	}


	/*
	Major Process Error.
	Render angle and control angle must be separated.
	I always want the D-pad inputs to change control angle.
	Only sometimes do I want them to change the render angle.
	*/

		you.dirInp = false;
	if(jo_is_input_key_pressed(id, JO_KEY_UP) && jo_is_input_key_pressed(id, JO_KEY_RIGHT)){
		you.rot[Y] = DEGtoANG(45) - you.viewRot[Y]; 
		you.dirInp = true;
	} else if(jo_is_input_key_pressed(id, JO_KEY_UP) && jo_is_input_key_pressed(id, JO_KEY_LEFT)){
		you.rot[Y] = -DEGtoANG(45) - you.viewRot[Y]; 
		you.dirInp = true;
	} else if(jo_is_input_key_pressed(id, JO_KEY_DOWN) && jo_is_input_key_pressed(id, JO_KEY_RIGHT)){
		you.rot[Y] = DEGtoANG(135) - you.viewRot[Y]; 
		you.dirInp = true;
	} else if(jo_is_input_key_pressed(id, JO_KEY_DOWN) && jo_is_input_key_pressed(id, JO_KEY_LEFT)){
		you.rot[Y] = -DEGtoANG(135) - you.viewRot[Y]; 
		you.dirInp = true;
	} else if(jo_is_input_key_pressed(id, JO_KEY_UP)){
		you.rot[Y] = -you.viewRot[Y]; 
		you.dirInp = true;
	} else if(jo_is_input_key_pressed(id, JO_KEY_DOWN)){
		you.rot[Y] = -DEGtoANG(180) - you.viewRot[Y]; 
		you.dirInp = true;
	} else if(jo_is_input_key_pressed(id, JO_KEY_LEFT)){
		you.rot[Y] = -DEGtoANG(90) - you.viewRot[Y]; 
		you.dirInp = true;
	} else if(jo_is_input_key_pressed(id, JO_KEY_RIGHT)){
		you.rot[Y] = DEGtoANG(90) - you.viewRot[Y]; 
		you.dirInp = true;
	}
	
	
		spdfactr = fxm(300, framerate<<16);
	if(you.dirInp == true){
		you.IPaccel += spdfactr;
	}
		
	if(jo_is_input_key_pressed(id, JO_KEY_L) ){
		you.setSlide = true;
	} else {
		you.setSlide = false;
	}
	static FIXED rKeyTimer = 0;
	//This bit, my friends, is spaghetti.
	//There is a timer here for three purposes:
	//One, to ensure we can't just hold down the jump button and repeatedly jump.
	//Two, to ensure that there is some time after pressing the jump button that we may be allowed to jump.
	//Three, to ensure that the step sound doesn't destroy the jump sound if the timer is in a condition that would allow it to jump.
	//They share a channel and they never need to play at the same time, but they could logically, so this skirts around that gingerly.
	if(jo_is_input_key_pressed(id, JO_KEY_R) ){
		if(rKeyTimer <= (66 / framerate)){ 
			if(you.onSurface == true){
				you.setJump = true;
				rKeyTimer += 50;
			}
			you.okayStepSnd = false;
		} else {
			you.okayStepSnd = true;
		}
//		you.Velocity[Y] += fxm((13106), (framerate)<<16);
		rKeyTimer++;
	} else {
		you.okayStepSnd = true;
		rKeyTimer = 0;
	}
	//


	//
	//This is here because the _player struct is already here and all the math for it is here	
	if(jo_is_input_key_pressed(id, JO_KEY_START)){
		reset_player();
	}
	
}

void	mypad(void)
{
	if(!jo_is_pad1_available()) return;
	
	controls();
}
