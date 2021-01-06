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
    you.velocity[X]=0;
    you.velocity[Y]=0;
    you.velocity[Z]=0;
    you.Force[X]=0;
    you.Force[Y]=0;
    you.Force[Z]=0;
	you.IPaccel=0;
	you.id = 0;
	you.power = 0;
}

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
		you.dirInp = true;
	} else if(is_key_down(DIGI_UP) && is_key_down(DIGI_LEFT)){
		you.rot[Y] = -(45 * 182) - you.viewRot[Y]; 
		you.dirInp = true;
	} else if(is_key_down(DIGI_DOWN) && is_key_down(DIGI_RIGHT)){
		you.rot[Y] = (135 * 182) - you.viewRot[Y]; 
		you.dirInp = true;
	} else if(is_key_down(DIGI_DOWN) && is_key_down(DIGI_LEFT)){
		you.rot[Y] = -(135 * 182) - you.viewRot[Y]; 
		you.dirInp = true;
	} else if(is_key_down(DIGI_UP)){
		you.rot[Y] = -you.viewRot[Y]; 
		you.dirInp = true;
	} else if(is_key_down(DIGI_DOWN)){
		you.rot[Y] = -(180 * 182) - you.viewRot[Y]; 
		you.dirInp = true;
	} else if(is_key_down(DIGI_LEFT)){
		you.rot[Y] = -(90 * 182) - you.viewRot[Y]; 
		you.dirInp = true;
	} else if(is_key_down(DIGI_RIGHT)){
		you.rot[Y] = (90 * 182) - you.viewRot[Y]; 
		you.dirInp = true;
	}
	
	
		spdfactr = fxm(300, frmul);
	if(you.dirInp == true){
		you.IPaccel += spdfactr;
	}
		
	if(is_key_down(DIGI_L) ){
		you.setSlide = true;
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

