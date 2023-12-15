
#include <sl_def.h>
#include <string.h>
#include "def.h"
#include "pcmsys.h"
#include "input.h"
#include "control.h"
#include "render.h"
#include "physobjet.h"
#include "collision.h"
#include "draw.h"
#include "mymath.h"
#include "hmap.h"
#include "particle.h"
#include "sound.h"

#define PLR_FWD_SPD (32768)
#define PLR_RVS_SPD (32768)
#define PLR_STRF_SPD (32768)

POINT alwaysLow = {0, -(1<<16), 0};
POINT alwaysHigh = {0, (1<<16), 0};
FIXED	lowPoint[XYZ] = {0, 0, 0};

/*

Maybe just straight-up add "jetpack"
You press button, move up slightly, but mostly you get directional   in the air.
Perhaps I can make the Y very weak if you're trying to move in a direction,
so jetting near the ground when skiing will be your control method.

I mean, this *was* an essential part of what made tribes fun to play. Limited capacity to move like an angel...

*/

void reset_player(void)
{
	you.mass = 250<<16;
	
	hmap_matrix_pos[X] = 0;
	hmap_matrix_pos[Z] = 0;
	you.hitMap = false;
	you.hitSurface = false;
	you.hitWall = false;
	you.climbing = false;
	you.hitObject = false;
	you.hitBox = false;
	you.ladder = false;
	you.okayStepSnd = true;
    you.pos[X]=you.startPos[X];
    you.pos[Y]=you.startPos[Y];
    you.pos[Z]=you.startPos[Z];
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
	you.dV[X]=0;
	you.dV[Y]=0;
	you.dV[Z]=0;
    you.slip[X]=0;
    you.slip[Y]=0;
    you.slip[Z]=0;
	you.IPaccel=0;
	you.id = 0;
	you.power = 0;
	you.maxPower = 4;
	you.avg_sanics = 0;
	you.sanic_samples = 0;
	you.distanceToMapFloor = 0;
	you.firstSurfHit = 0;
	you.airTime = 0;
	you.cancelTimers = true;
	you.resetTimers = false;
}


void pl_jump(void)
{
	//First: If we are jumping on a frame where we just touched the ground,
	// the Y velocity will not have been zeroed out to the ground.
	// Thus, jumping may fail to add enough velocity to overcome this.
	// To compensate, we must first remove velocity relative to the floor.
	if(you.velocity[Y] < 0)
	{
		you.velocity[Y] += fxm(you.velocity[Y], you.floorNorm[Y]);
		//you.velocity[Y] = 0;
	}
	you.hitSurface = false;
	you.jumpAllowed = false;
	//This should not be time-scaled, since it does not increment over multiple frames.
	you.velocity[X] -= fxm(229376, you.floorNorm[X]); 
	you.velocity[Y] -= fxm(229376, you.floorNorm[Y]);
	you.velocity[Z] -= fxm(229376, you.floorNorm[Z]);
	//Surface release
	you.pos[Y] += 6553 + (1<<16);
	pcm_play(snd_bstep, PCM_SEMI, 6);
}

void pl_jet(void){
	if(you.dirInp == true)
	{
		you.dV[X] += fxm(fxm(you.IPaccel, you.ControlUV[X]), 3000);
		you.dV[Y] += GRAVITY;
		you.dV[Z] += fxm(fxm(you.IPaccel, you.ControlUV[Z]), 3000);
	} else {
		you.dV[Y] += GRAVITY + (GRAVITY>>2);
	}

	//X/Z Speed Allowance for zero air thrust
	int hvscl = fxm(you.velocity[X], you.velocity[X]) + fxm(you.velocity[Z], you.velocity[Z]);

	if(you.jumpAllowed == true && you.power == you.maxPower)
	{
		//Surface release
		you.pos[Y] += 6553 - fxm(you.floorNorm[X], 1<<16) - fxm(you.floorNorm[Y], 1<<16) - fxm(you.floorNorm[Z], 1<<16);
		you.jumpAllowed = false;
		you.hitSurface = false;
		//Hop
		//This should not be time-scaled, since it does not increment over multiple frames.
		you.velocity[Y] += (1<<16) + 32768;
		//Stuff for the puffs
		you.wvel[X] -= you.dV[X]<<4;
		you.wvel[Y] -= (2<<16) - (GRAVITY<<1);
		you.wvel[Z] -= you.dV[Z]<<4;
		emit_particle_explosion(&HopPuff, PARTICLE_TYPE_NOCOL, you.wpos, you.wvel, 12<<16, 8192, 6);
	} else if(you.jumpAllowed == false && you.power == you.maxPower && hvscl < 16384)
	{
		you.dV[X] += fxm(32768, you.ControlUV[X]);
		you.dV[Z] += fxm(32768, you.ControlUV[Z]);
		//Stuff for the puffs
		you.wvel[X] -= you.dV[X]<<2;
		you.wvel[Y] -= (1<<16) - (GRAVITY<<1);
		you.wvel[Z] -= you.dV[Z]<<2;
		emit_particle_explosion(&HopPuff, PARTICLE_TYPE_NOCOL, you.wpos, you.wvel, 12<<16, 8192, 6);
	}

		
	you.power -= 1;
	pcm_play(snd_bwee, PCM_PROTECTED, 6);
}


void	pl_step_snd(void){
	
	/*
		HOOF POLY #
		Changes when exporting
		Intent:
		First, check if we're in a moving-animated state.
		Check a vertice from each hoof and see if it is above 294500 [approx. 4.5].
		Use this information to play a sound whenever any hoof clears this condition.
		If multiple hooves do, increase the volume?
	*/
	if(pl_model.file_done != true) return;
	static char hoofSetBools[5];
	static char oldHoofSetBools[5];
	char runSnd = 0;
	const int HoofLowValue = 300100;
	//int printPos = 0;
	int hf_vert[4] = {6, 28, 57, 102};
	POINT hf_pos;
	int partVelocity[XYZ] = {0, -1024, 0};

		if(you.hitSurface == true){
			for(int h = 0; h < 4; h++)
			{
				if(pl_model.pol->pntbl[hf_vert[h]][Y] > HoofLowValue)
				{
					hoofSetBools[h] = true;
				if(hoofSetBools[h] != oldHoofSetBools[h])
				{
					runSnd = 1;
					/*
					Puff of smoke to display when player steps
					*/
					transform_mesh_point(pl_model.pol->pntbl[hf_vert[h]], hf_pos, &pl_RBB);
					hf_pos[X] = hf_pos[X] - you.pos[X];
					hf_pos[Y] = hf_pos[Y] - you.pos[Y];
					hf_pos[Z] = hf_pos[Z] - you.pos[Z];
					spawn_particle(&SmallPuff, PARTICLE_TYPE_GHOST, hf_pos, partVelocity);
				}
				} else {
					hoofSetBools[h] = false;
				}
			}
		}

	hoofSetBools[4] = (you.hitSurface);
	
		runSnd = (hoofSetBools[4] != oldHoofSetBools[4]) ? 1 : runSnd;
	
	if(runSnd == 1)
	{
		pcm_play(snd_lstep, PCM_PROTECTED, 5);
	}
	
	oldHoofSetBools[0] = hoofSetBools[0];
	oldHoofSetBools[1] = hoofSetBools[1];
	oldHoofSetBools[2] = hoofSetBools[2];
	oldHoofSetBools[3] = hoofSetBools[3];
	oldHoofSetBools[4] = hoofSetBools[4];

}

void	smart_cam(void)
{
	///////////////////////////////////////////
	//Smart Camera Setup
	///////////////////////////////////////////
	if((you.rot2[Y] > (150 * 182) && you.rot2[Y] < (210 * 182) && you.dirInp) || usrCntrlOption.lockTimer > 0)
	{
		usrCntrlOption.lockout = true;
	}
	if(usrCntrlOption.lockTimer > 0)
	{
		usrCntrlOption.lockTimer -= delta_time;
	}
	///////////////////////////////////////////
	// Movement-following camera
	///////////////////////////////////////////
	// "uview" is the discrete vector notation of the player's viewport.
	VECTOR uview = {-slSin(you.viewRot[Y]), slSin(you.viewRot[X]), slCos(you.viewRot[Y])};
	// proportion_y is a mathemagical value that is the positive or negative proportion of the view vector,
	// as compared to the movement vector. While we are doing multiplication here, it's ostensibly division (these are <1 values).
	int proportion_y;
	//This is simply the difference between the movement vector's Y and the viewing vector's Y.
	//prop y and prop x are shifted down to scale it as desired. Magic shift, in other words.
	int proportion_x;
	//angDif_y is an expression of how different the movement vector and viewing vector is,
	//as it relates to the axis that Y view rotation controls (X and Z).
	short angDif_y;
	if(usrCntrlOption.movementCam)
	{
		if(!you.climbing)
		{
			proportion_y = (fxm((you.DirUV[X] - uview[X]),(you.DirUV[X] - uview[X])) + fxm((you.DirUV[Z] - uview[Z]),(you.DirUV[Z] - uview[Z])))>>7;
			proportion_x = (you.DirUV[Y] - uview[Y])>>7;
			angDif_y = (slAtan(you.DirUV[X], you.DirUV[Z]) - slAtan(uview[X], uview[Z]));
		} else {
			proportion_y = (fxm((you.floorNorm[X] - uview[X]),(you.floorNorm[X] - uview[X])) + fxm((you.floorNorm[Z] - uview[Z]),(you.floorNorm[Z] - uview[Z])))>>7;
			proportion_x = (you.floorNorm[Y] - uview[Y])>>7;
			angDif_y = (slAtan(you.floorNorm[X], you.floorNorm[Z]) - slAtan(uview[X], uview[Z]));
		}
	
		//This angle will amount to a proportion of angle we're not yet facing towards the ground.
		int proportion_facing_ground = (-32768 - uview[Y])>>7;
		//Will pivot camera towards direction of motion
		proportion_x = fxm(proportion_x, usrCntrlOption.followForce);
		proportion_y = fxm(proportion_y, usrCntrlOption.followForce);
		proportion_facing_ground = fxm(proportion_facing_ground, usrCntrlOption.followForce);
		if((JO_ABS(you.velocity[X]) > 1024 || JO_ABS(you.velocity[Z]) > 1024) &&  JO_ABS(angDif_y) > 1024 && !usrCntrlOption.lockout)
		{
			//Determines if we want to rotate view clockwise or counterclockwise (and then does)
			you.viewRot[Y] += (angDif_y > 0) ? (proportion_y * framerate)>>1 : -(proportion_y * framerate)>>1; 
			usrCntrlOption.lockout = true;
		}
		if((JO_ABS(you.velocity[Y]) > 1024 || you.dirInp == true))
		{
			//If we are on the ground, we want a camera that'll tilt up and down if we're going up or down.
			//We also do not want the camera to pan down if we are gliding/hopping.
			//If we are not, we just want the camera to tilt downwards so we can see where we are going to land.
			//If we are climbing, proportion_x was pre-adjusted so we'll try and look at the wall.
			if(you.hitSurface == true || you.climbing || (you.setJet && you.airTime > (1<<16)))
			{
				you.viewRot[X] +=  (proportion_x * framerate)>>1;
			} else {
				you.viewRot[X] += (proportion_facing_ground * framerate)>>1;
			}
			
			usrCntrlOption.lockout = true;
		}
	}
	//////////////////////////////////////////////
	// Facing-following camera
	//////////////////////////////////////////////
	//Push the facing-follow camera up a little bit. Just to control warping.
	if(usrCntrlOption.facingCam)
	{
		uview[Y] = slSin(you.viewRot[X] + (7 * 182));
		proportion_y = (fxm((you.ControlUV[X] - uview[X]),
		(you.ControlUV[X] - uview[X])) + fxm((you.ControlUV[Z] - uview[Z]),
		(you.ControlUV[Z] - uview[Z])))>>7;
		proportion_x = (you.ControlUV[Y] - uview[Y])>>7;
		angDif_y = (slAtan(you.ControlUV[X], you.ControlUV[Z]) - slAtan(uview[X], uview[Z]));
		
		proportion_x = fxm(proportion_x, usrCntrlOption.followForce);
		proportion_y = fxm(proportion_y, usrCntrlOption.followForce);
		
		//Determines if we want to rotate view clockwise or counterclockwise (and then does)
		if(!usrCntrlOption.lockout)
		{
			you.viewRot[Y] += (angDif_y > 0) ? (proportion_y * framerate)>>1 : -(proportion_y * framerate)>>1; 
			you.viewRot[X] += (proportion_x * framerate)>>1;
		}
	}
	usrCntrlOption.lockout = false;
}

void	construct_line_tables(void)
{
	
	//////////////////////////////////////////////////////////////
	// Add the position of the mover's box centre-faces to the mover's world position
	// "Get world-space point position"
	//////////////////////////////////////////////////////////////
	//
	// Have to be careful how collisions are checked; they affect where you end up being snapped to.
	// So be careful: This is NEEDED to successfully capture high-velocity collisions.
	// However, floor-snapping needs to be aware of the gap created by it, or else it screws up your speed.
	//
	// Also, two of the nearly-identical struct exist because somewhere along the line, I flipped the coordinate systems.
	// And I don't know how to unflip them.
	you.fwd_world_faces.xp0[X] = pl_RBB.Xplus[X] 	+ pl_RBB.pos[X];
	you.fwd_world_faces.xp0[Y] = pl_RBB.Xplus[Y] 	+ pl_RBB.pos[Y];
	you.fwd_world_faces.xp0[Z] = pl_RBB.Xplus[Z] 	+ pl_RBB.pos[Z];
	you.fwd_world_faces.xp1[X] = pl_RBB.Xneg[X] 	+ pl_RBB.pos[X];
	you.fwd_world_faces.xp1[Y] = pl_RBB.Xneg[Y] 	+ pl_RBB.pos[Y];
	you.fwd_world_faces.xp1[Z] = pl_RBB.Xneg[Z] 	+ pl_RBB.pos[Z];
	you.fwd_world_faces.yp0[X] = pl_RBB.Yplus[X] 	+ pl_RBB.pos[X];
	you.fwd_world_faces.yp0[Y] = pl_RBB.Yplus[Y] 	+ pl_RBB.pos[Y];
	you.fwd_world_faces.yp0[Z] = pl_RBB.Yplus[Z] 	+ pl_RBB.pos[Z];
	you.fwd_world_faces.yp1[X] = pl_RBB.Yneg[X] 	+ pl_RBB.pos[X];
	you.fwd_world_faces.yp1[Y] = pl_RBB.Yneg[Y] 	+ pl_RBB.pos[Y];
	you.fwd_world_faces.yp1[Z] = pl_RBB.Yneg[Z] 	+ pl_RBB.pos[Z];
	you.fwd_world_faces.zp0[X] = pl_RBB.Zplus[X] 	+ pl_RBB.pos[X];
	you.fwd_world_faces.zp0[Y] = pl_RBB.Zplus[Y] 	+ pl_RBB.pos[Y];
	you.fwd_world_faces.zp0[Z] = pl_RBB.Zplus[Z] 	+ pl_RBB.pos[Z];
	you.fwd_world_faces.zp1[X] = pl_RBB.Zneg[X] 	+ pl_RBB.pos[X];
	you.fwd_world_faces.zp1[Y] = pl_RBB.Zneg[Y] 	+ pl_RBB.pos[Y];
	you.fwd_world_faces.zp1[Z] = pl_RBB.Zneg[Z] 	+ pl_RBB.pos[Z];
	
	you.bwd_world_faces.xp0[X] = pl_RBB.Xplus[X] 	- pl_RBB.pos[X];
	you.bwd_world_faces.xp0[Y] = pl_RBB.Xplus[Y] 	- pl_RBB.pos[Y];
	you.bwd_world_faces.xp0[Z] = pl_RBB.Xplus[Z] 	- pl_RBB.pos[Z];
	you.bwd_world_faces.xp1[X] = pl_RBB.Xneg[X] 	- pl_RBB.pos[X];
	you.bwd_world_faces.xp1[Y] = pl_RBB.Xneg[Y] 	- pl_RBB.pos[Y];
	you.bwd_world_faces.xp1[Z] = pl_RBB.Xneg[Z] 	- pl_RBB.pos[Z];
	you.bwd_world_faces.yp0[X] = pl_RBB.Yplus[X] 	- pl_RBB.pos[X];
	you.bwd_world_faces.yp0[Y] = pl_RBB.Yplus[Y] 	- pl_RBB.pos[Y];
	you.bwd_world_faces.yp0[Z] = pl_RBB.Yplus[Z] 	- pl_RBB.pos[Z];
	you.bwd_world_faces.yp1[X] = pl_RBB.Yneg[X] 	- pl_RBB.pos[X];
	you.bwd_world_faces.yp1[Y] = pl_RBB.Yneg[Y] 	- pl_RBB.pos[Y];
	you.bwd_world_faces.yp1[Z] = pl_RBB.Yneg[Z] 	- pl_RBB.pos[Z];
	you.bwd_world_faces.zp0[X] = pl_RBB.Zplus[X] 	- pl_RBB.pos[X];
	you.bwd_world_faces.zp0[Y] = pl_RBB.Zplus[Y] 	- pl_RBB.pos[Y];
	you.bwd_world_faces.zp0[Z] = pl_RBB.Zplus[Z] 	- pl_RBB.pos[Z];
	you.bwd_world_faces.zp1[X] = pl_RBB.Zneg[X] 	- pl_RBB.pos[X];
	you.bwd_world_faces.zp1[Y] = pl_RBB.Zneg[Y] 	- pl_RBB.pos[Y];
	you.bwd_world_faces.zp1[Z] = pl_RBB.Zneg[Z] 	- pl_RBB.pos[Z];
	
	you.time_axis.xp0[X] = fxm(fxm(pl_RBB.UVX[X],  JO_ABS(pl_RBB.velocity[X])>>1), time_fixed_scale);
	you.time_axis.xp0[Y] = fxm(fxm(pl_RBB.UVX[Y],  JO_ABS(pl_RBB.velocity[Y])>>1), time_fixed_scale);
	you.time_axis.xp0[Z] = fxm(fxm(pl_RBB.UVX[Z],  JO_ABS(pl_RBB.velocity[Z])>>1), time_fixed_scale);
	you.time_axis.xp1[X] = fxm(fxm(pl_RBB.UVNX[X], JO_ABS(pl_RBB.velocity[X])>>1), time_fixed_scale);
	you.time_axis.xp1[Y] = fxm(fxm(pl_RBB.UVNX[Y], JO_ABS(pl_RBB.velocity[Y])>>1), time_fixed_scale);
	you.time_axis.xp1[Z] = fxm(fxm(pl_RBB.UVNX[Z], JO_ABS(pl_RBB.velocity[Z])>>1), time_fixed_scale);
	you.time_axis.yp0[X] = fxm(fxm(pl_RBB.UVY[X],  JO_ABS(pl_RBB.velocity[X])>>1), time_fixed_scale);
	you.time_axis.yp0[Y] = fxm(fxm(pl_RBB.UVY[Y],  JO_ABS(pl_RBB.velocity[Y])>>1), time_fixed_scale);
	you.time_axis.yp0[Z] = fxm(fxm(pl_RBB.UVY[Z],  JO_ABS(pl_RBB.velocity[Z])>>1), time_fixed_scale);
	you.time_axis.yp1[X] = fxm(fxm(pl_RBB.UVNY[X], JO_ABS(pl_RBB.velocity[X])>>1), time_fixed_scale);
	you.time_axis.yp1[Y] = fxm(fxm(pl_RBB.UVNY[Y], JO_ABS(pl_RBB.velocity[Y])>>1), time_fixed_scale);
	you.time_axis.yp1[Z] = fxm(fxm(pl_RBB.UVNY[Z], JO_ABS(pl_RBB.velocity[Z])>>1), time_fixed_scale);
	you.time_axis.zp0[X] = fxm(fxm(pl_RBB.UVZ[X],  JO_ABS(pl_RBB.velocity[X])>>1), time_fixed_scale);
	you.time_axis.zp0[Y] = fxm(fxm(pl_RBB.UVZ[Y],  JO_ABS(pl_RBB.velocity[Y])>>1), time_fixed_scale);
	you.time_axis.zp0[Z] = fxm(fxm(pl_RBB.UVZ[Z],  JO_ABS(pl_RBB.velocity[Z])>>1), time_fixed_scale);
	you.time_axis.zp1[X] = fxm(fxm(pl_RBB.UVNZ[X], JO_ABS(pl_RBB.velocity[X])>>1), time_fixed_scale);
	you.time_axis.zp1[Y] = fxm(fxm(pl_RBB.UVNZ[Y], JO_ABS(pl_RBB.velocity[Y])>>1), time_fixed_scale);
	you.time_axis.zp1[Z] = fxm(fxm(pl_RBB.UVNZ[Z], JO_ABS(pl_RBB.velocity[Z])>>1), time_fixed_scale);
	
	you.fwd_world_faces.xp0[X] += you.time_axis.xp0[X];
	you.fwd_world_faces.xp0[Y] += you.time_axis.xp0[Y];
	you.fwd_world_faces.xp0[Z] += you.time_axis.xp0[Z];
	you.fwd_world_faces.xp1[X] += you.time_axis.xp1[X];
	you.fwd_world_faces.xp1[Y] += you.time_axis.xp1[Y];
	you.fwd_world_faces.xp1[Z] += you.time_axis.xp1[Z];
	you.fwd_world_faces.yp0[X] += you.time_axis.yp0[X];
	you.fwd_world_faces.yp0[Y] += you.time_axis.yp0[Y];
	you.fwd_world_faces.yp0[Z] += you.time_axis.yp0[Z];
	you.fwd_world_faces.yp1[X] += you.time_axis.yp1[X];
	you.fwd_world_faces.yp1[Y] += you.time_axis.yp1[Y];
	you.fwd_world_faces.yp1[Z] += you.time_axis.yp1[Z];
	you.fwd_world_faces.zp0[X] += you.time_axis.zp0[X];
	you.fwd_world_faces.zp0[Y] += you.time_axis.zp0[Y];
	you.fwd_world_faces.zp0[Z] += you.time_axis.zp0[Z];
	you.fwd_world_faces.zp1[X] += you.time_axis.zp1[X];
	you.fwd_world_faces.zp1[Y] += you.time_axis.zp1[Y];
	you.fwd_world_faces.zp1[Z] += you.time_axis.zp1[Z];
	
	you.bwd_world_faces.xp0[X] += you.time_axis.xp0[X];
	you.bwd_world_faces.xp0[Y] += you.time_axis.xp0[Y];
	you.bwd_world_faces.xp0[Z] += you.time_axis.xp0[Z];
	you.bwd_world_faces.xp1[X] += you.time_axis.xp1[X];
	you.bwd_world_faces.xp1[Y] += you.time_axis.xp1[Y];
	you.bwd_world_faces.xp1[Z] += you.time_axis.xp1[Z];
	you.bwd_world_faces.yp0[X] += you.time_axis.yp0[X];
	you.bwd_world_faces.yp0[Y] += you.time_axis.yp0[Y];
	you.bwd_world_faces.yp0[Z] += you.time_axis.yp0[Z];
	you.bwd_world_faces.yp1[X] += you.time_axis.yp1[X];
	you.bwd_world_faces.yp1[Y] += you.time_axis.yp1[Y];
	you.bwd_world_faces.yp1[Z] += you.time_axis.yp1[Z];
	you.bwd_world_faces.zp0[X] += you.time_axis.zp0[X];
	you.bwd_world_faces.zp0[Y] += you.time_axis.zp0[Y];
	you.bwd_world_faces.zp0[Z] += you.time_axis.zp0[Z];
	you.bwd_world_faces.zp1[X] += you.time_axis.zp1[X];
	you.bwd_world_faces.zp1[Y] += you.time_axis.zp1[Y];
	you.bwd_world_faces.zp1[Z] += you.time_axis.zp1[Z];
	
		// nbg_sprintf(1, 6, "xx: (%i)", you.world_faces.xp0[X]);
		// nbg_sprintf(1, 7, "xy: (%i)", you.world_faces.xp0[Y]);
		// nbg_sprintf(1, 8, "xz: (%i)", you.world_faces.xp0[Z]);			
	
}
 
void	player_phys_affect(void)
{
	
		// nbg_sprintf(1, 6, "hitObject: (%i)", you.hitObject);
		// nbg_sprintf(1, 7, "hitBox: (%i)", you.hitBox);
		// nbg_sprintf(1, 8, "hitMap: (%i)", you.hitMap);			
		// nbg_sprintf(1, 9, "hitWall: (%i)", you.hitWall);
		// nbg_sprintf(1, 12, "hitSurface: (%i)", you.hitSurface);
		// slPrintFX(time_in_seconds, slLocate(1, 14));
	
	///////////////////////////////////////////////
	// Sanic capture for measuring speed over time
	///////////////////////////////////////////////
	you.sanic_samples++;
	int inversion = fxdiv(1<<16, you.sanic_samples<<16);
	you.avg_sanics -= fxm(you.avg_sanics, inversion);
	you.avg_sanics += fxm(you.sanics, inversion);
	
	//Derive three angles from two inputs. (Has to be time-scaled, because its incremental)
	you.viewRot[X] += you.rotState[Y] * framerate;
	you.viewRot[Y] -= you.rotState[X] * framerate;
	///////////////////////////////////////////////
	// Lock-in to +- 360 degrees
	you.viewRot[Y] = (you.viewRot[Y] < (-65536)) ? 0 : (you.viewRot[Y] > 65536) ? 0 : you.viewRot[Y];
	you.viewRot[X] = (you.viewRot[X] < (-65536)) ? 0 : (you.viewRot[X] > 65536) ? 0 : you.viewRot[X];
	///////////////////////////////////////////////
	// Optional X-rotation lock to +/- 90
	you.viewRot[X] = (you.viewRot[X] < (-16384)) ? -16384 : (you.viewRot[X] > 16384) ? 16384 : you.viewRot[X];
	///////////////////////////////////////////////
	// Control axis matrix
	///////////////////////////////////////////////
	static int slide_control_matrix[9];
	//The control unit vector is using the player's bound box / matrix parameters.
	//In this case, it's the forward vector.
	//When we are sliding, the player's orientation is locked, but the control vector is allowed to be off-axis.
	//To facilitate this, we have to rotate about the local axis once more.
	if(you.setSlide != true || you.climbing == true)
	{
	you.ControlUV[X] = pl_RBB.UVZ[X];
    you.ControlUV[Y] = pl_RBB.UVZ[Y];
    you.ControlUV[Z] = pl_RBB.UVZ[Z];
	} else if(you.setSlide == true)
	{
	player_sliding_particles();
	copy_matrix(slide_control_matrix, &pl_RBB.UVX[0]);
	fxRotLocalAxis(slide_control_matrix, alwaysHigh, -you.rot2[Y]);
	you.ControlUV[X] = slide_control_matrix[6];
    you.ControlUV[Y] = slide_control_matrix[7];
    you.ControlUV[Z] = slide_control_matrix[8];
	you.rot[Y] = -you.viewRot[Y];
	}

	smart_cam();

	///////////////////////////////////////////////
	// Velocity Constant Changes
	///////////////////////////////////////////////
	const int airFriction = 325; //very low friction (<1%)
		//There's always air .. unless we go into space, but whatevs, bruh
		you.dV[X] -= fxm(you.velocity[X], (airFriction));
		you.dV[Y] -= fxm(you.velocity[Y], (airFriction));
		you.dV[Z] -= fxm(you.velocity[Z], (airFriction));
		
		you.dV[Y] -= GRAVITY;

	///////////////////////////////////////////////
	// Jump & Jet Decisions
	///////////////////////////////////////////////
		if(you.setJump && you.jumpAllowed)
		{
			pl_jump();
			you.setJump = false;
		}
		
		static int powerTimer = 0;
		
		if( you.okayStepSnd ) pl_step_snd();
	
		if(you.setJet == true && you.power > 0)
		{
			pl_jet();
		} else if(you.power < you.maxPower){
			powerTimer += delta_time;
			if(powerTimer > (4096))
			{
				you.power += 1;
				powerTimer = 0;
			}
		}
		
	////////////////////////////////////////////////////
	//Input-speed response
	// Need a graceful way to handle slip
	////////////////////////////////////////////////////
	if(you.setSlide != true && you.hitSurface == true)
	{
		
		you.dV[X] += fxm(you.IPaccel, you.ControlUV[X]);
		you.dV[Y] += fxm(you.IPaccel, you.ControlUV[Y]);
		you.dV[Z] += fxm(you.IPaccel, you.ControlUV[Z]);
		
		you.accel[X] = you.dV[X];
		you.accel[Y] = you.dV[Y];
		you.accel[Z] = you.dV[Z];
	} else { 
	//If sliding or in the air
	//I don't want this to enable going faster, but I do want it to help you turn?
	//I also want to increase turning authority at higher speeds?
	
		you.dV[X] += fxm(fxm(you.IPaccel, you.ControlUV[X]), 3000);
		you.dV[Y] += fxm(fxm(you.IPaccel, you.ControlUV[Y]), 3000);
		you.dV[Z] += fxm(fxm(you.IPaccel, you.ControlUV[Z]), 3000);

		you.accel[X] = you.dV[X];
		you.accel[Y] = you.dV[Y];
		you.accel[Z] = you.dV[Z];
	}

	////////////////////////////////////////////////////
	//On-surface collision response
	////////////////////////////////////////////////////
	static int deflectionFactor = 0;
	if(you.hitSurface == true)
	{
		//These conditions force ground contact to occur when jumping repeatedly
		you.airTime = 0;
		you.allowJumpTimer = 0;
		you.jumpAllowed = true;
		
		if(you.climbing)
		{
			you.velocity[X] = fxm(you.IPaccel>>1, pl_RBB.UVZ[X]);
			you.velocity[Y] = fxm(you.IPaccel>>1, pl_RBB.UVZ[Y]);
			you.velocity[Z] = fxm(you.IPaccel>>1, pl_RBB.UVZ[Z]);
			you.wasClimbing = true;
		} else if(you.firstSurfHit)
		{
			/*
			My best explanation of what's happening here:
			Weight/gravity is pushing the player down, at all times.
			Right now the system does not abstract between force and acceleration; I know that's a major flaw.
			But for now it's a needed simplification so I can understand things.
			So we must interpret this "gravity" as the force itself, in the math, even though it is the weight.
			The weight applies a force to the surface, and this force must be deflected by the surface.
			There's no rebound here though, as the rebound is not controlling the collision state (like it is with walls).
			*/
			deflectionFactor = fxm(GRAVITY, you.floorNorm[Y]);

			you.dV[X] += fxm(you.floorNorm[X], deflectionFactor); 
			you.dV[Y] += fxm(you.floorNorm[Y], deflectionFactor); 
			you.dV[Z] += fxm(you.floorNorm[Z], deflectionFactor); 

			//You need to be able to hold on to some intent to move down, even if towards the floor.
			//"dV" is literally just acceleration; we should be able to hold onto intent to accelerate. Or really, force.
			//But my maths doesn't do force. Anyway, this is correct: dV not-floored, velocity is floored.
			//The bounciness that this causes, for all I understand, appears to just be correct.
			you.velocity[Y] += fxm(you.velocity[Y], you.floorNorm[Y]);
		}
		
		//Note the order of operations. This is placed after gravity is surface-deflected.
		///It is placed here for bounce control.
		if(!you.firstSurfHit)
		{
			
			if(you.sanics >= 3<<16)
			{
			pcm_play(snd_mstep, PCM_PROTECTED, 6);
			int nFloorPos[3] = {-you.floorPos[X], -you.floorPos[Y], -you.floorPos[Z]};
			emit_particle_explosion(&HitPuff, PARTICLE_TYPE_NOCOL, nFloorPos, you.floorNorm, 8<<16, 8192, 4);
			}
			
			//This is sourced from an article on Tribes physics. It really helps to understand *bounce*.
			//At first, I tried deflection formulas. Or just scaling the normal into the velocity.
			//Or scaling the velocity by some half and not-half of the normal.
			//What we have below is an "impact dot" by the deflection factor.
			//It's simple and it works because the impact dot is bigger the more oblique the impact angle, AND the faster you are going.
			//That's exactly the math I wanted but was too stupid to see how it should be implemented on the velocity.
			deflectionFactor = fxdot(you.velocity, you.floorNorm);
			//Plan: Rebound elasticity scales, starting at 0x4000, going all the way up to 0xFFFF.
			//It gets higher the faster you go, with 0xFFFF happening when moving past 5 sanics.

			// This is ESSENTIAL for the momentum gameplay to work properly 
			you.velocity[X] -= fxm(you.floorNorm[X], deflectionFactor + (REBOUND_ELASTICITY)); 
			you.velocity[Y] -= fxm(you.floorNorm[Y], deflectionFactor + (REBOUND_ELASTICITY)); 
			you.velocity[Z] -= fxm(you.floorNorm[Z], deflectionFactor + (REBOUND_ELASTICITY)); 
			//you.dV[Y] += fxm(REBOUND_ELASTICITY, you.floorNorm[Y]);
			you.firstSurfHit = true;
			
		}
		
		///Take note that friction is done last in order to not corrupt the basis vector of floor bounce.
		///Also note this is a really big part of how you stick to the floor.
		//If normally on surface without modifier, high friction
		you.surfFriction = (19660); //33%, high friction				
		//Skiing Decisions
		if(you.setSlide == true)
		{
			you.surfFriction = 155; //very very low friction
		}		
		
		//Special Condition - no friction on surface contact + jump
		if(!you.setJump)
		{
			you.accel[X] -= fxm(you.velocity[X], (you.surfFriction));
			you.accel[Y] -= fxm(you.velocity[Y], (you.surfFriction));
			you.accel[Z] -= fxm(you.velocity[Z], (you.surfFriction));
			//Friction decisions
			you.dV[X] -= fxm(you.velocity[X], (you.surfFriction));
			you.dV[Y] -= fxm(you.velocity[Y], (you.surfFriction));
			you.dV[Z] -= fxm(you.velocity[Z], (you.surfFriction));
		}
		
		//Stiction; low velocities will trap at zero on surface.
		if(!you.dirInp && !you.setSlide && !you.setJump)
		{
			you.velocity[X] = (JO_ABS(you.velocity[X]) > 6553) ? you.velocity[X] : 0;
			you.velocity[Y] = (JO_ABS(you.velocity[Y]) > 6553) ? you.velocity[Y] : 0;
			you.velocity[Z] = (JO_ABS(you.velocity[Z]) > 6553) ? you.velocity[Z] : 0;
			you.dV[X] = (JO_ABS(you.velocity[X]) > 6553) ? you.dV[X] : 0;
			you.dV[Y] = (JO_ABS(you.velocity[Y]) > 6553) ? you.dV[Y] : 0;
			you.dV[Z] = (JO_ABS(you.velocity[Z]) > 6553) ? you.dV[Z] : 0;
		}
		
		you.pos[X] = (you.floorPos[X]);
		you.pos[Y] = (you.floorPos[Y]);
		you.pos[Z] = (you.floorPos[Z]);
		
	} else {
		you.allowJumpTimer++;
		you.firstSurfHit = false;
		you.airTime += delta_time;
		
		//Allow jump at least two frames after being released from a surface
		if(you.allowJumpTimer > 2)
		{
			you.jumpAllowed = false;
		}
	}
	
	////////////////////////////////////////////////////
	// Wall collision response
	////////////////////////////////////////////////////
	if(you.hitWall == true)
	{
			//Push away.
			you.dV[X] -= you.wallNorm[X]>>2;
			you.dV[Y] -= you.wallNorm[Y]>>2;
			you.dV[Z] -= you.wallNorm[Z]>>2;
			/*
	This code is inspired by/taken partly from Tribes.
	I wouldn't have independently come up with a solution that works this well.
	Instead, I was fixated on simply multiplying the wallNorm by velocity and subtracting that away from velocity.
	My goal with that was to zero-out all direction towards the wall. That only mostly works.
	This code however will let you bounce off of surfaces by an amount from 0-1 determined by the rebound elasticity.
	Most notably, this works even if rebound elasticity is set to zero.
			*/
			deflectionFactor = fxdot(you.velocity, you.wallNorm);
			//This shouldn't be time-scaled, should it?
			you.velocity[X] -= fxm(you.wallNorm[X], deflectionFactor + REBOUND_ELASTICITY); 
			you.velocity[Y] -= fxm(you.wallNorm[Y], deflectionFactor + REBOUND_ELASTICITY); 
			you.velocity[Z] -= fxm(you.wallNorm[Z], deflectionFactor + REBOUND_ELASTICITY); 
		
			you.hitWall = false;
			
			if(you.sanics >= 7<<16 && !you.setJet && !you.setJump) pcm_play(snd_smack, PCM_SEMI, 7);
			
			if(you.sanics >= 3<<16)
			{
			pcm_play(snd_mstep, PCM_SEMI, 6);
			emit_particle_explosion(&HitPuff, PARTICLE_TYPE_NOCOL, you.wallPos, you.wallNorm, 8<<16, 8192, 4);
			}
	} 

	//Ladder/Climb Escape Sequence
	//At least in one test, this was pretty much perfect!
	if(!you.climbing && you.wasClimbing)
	{
		you.dV[X] += fxm(you.floorNorm[X], 32768);
        you.dV[Y] += 32768;
        you.dV[Z] += fxm(you.floorNorm[Z], 32768);
		you.wasClimbing = false;
	}
	///////////////////////////////////////////////
	// Increase the velocity by the velocity change, multiplied by the timescale
	///////////////////////////////////////////////
	you.velocity[X] += fxm(you.dV[X], time_fixed_scale);
	you.velocity[Y] += fxm(you.dV[Y], time_fixed_scale);
	you.velocity[Z] += fxm(you.dV[Z], time_fixed_scale);
	///////////////////////////////////////////////
	// Position change
	///////////////////////////////////////////////
	you.pos[X] += fxm(you.velocity[X], time_fixed_scale);
	you.pos[Y] += fxm(you.velocity[Y], time_fixed_scale);
	you.pos[Z] += fxm(you.velocity[Z], time_fixed_scale);
	//Create a true direction vector, independent of control vector
	static VECTOR tempDif = {0, 0, 0};
	tempDif[X] = you.pos[X] - you.prevPos[X];
	tempDif[Y] = you.pos[Y] - you.prevPos[Y];
	tempDif[Z] = you.pos[Z] - you.prevPos[Z];
	normalize(tempDif, you.DirUV);
	you.sanics = slSquartFX(fxm(tempDif[X], tempDif[X]) + fxm(tempDif[Y], tempDif[Y]) + fxm(tempDif[Z], tempDif[Z]));
	you.sanics = fxm((you.sanics>>5), time_delta_scale);
	//Set prev pos
	you.prevPos[X] = you.pos[X];
	you.prevPos[Y] = you.pos[Y];
	you.prevPos[Z] = you.pos[Z];

	//Sound that plays louder the faster you go. Only initiates at all once you are past 3 in sanics.
	unsigned char windVol = ((you.sanics>>17) < 7) ? ((you.sanics>>17)+1) : 7;
	if(you.sanics > (3<<16))
	{
	pcm_play(snd_wind, PCM_FWD_LOOP, windVol); 
	} else {
	pcm_cease(snd_wind);
	}
	//slPrintFX(you.sanics, slLocate(0, 8));
		
	////////////////////////////////////////////////////////////
	//Movement and rotation speed maximum and minimums
	////////////////////////////////////////////////////////////
	if(you.IPaccel >= PLR_FWD_SPD) you.IPaccel = PLR_FWD_SPD;
	if(you.IPaccel <= -PLR_FWD_SPD) you.IPaccel = -PLR_FWD_SPD;
	//
	if(JO_ABS(you.rotState[X]) < usrCntrlOption.cameraAccel>>1) you.rotState[X] = 0;
	if(JO_ABS(you.rotState[Y]) < usrCntrlOption.cameraAccel>>1) you.rotState[Y] = 0;
	
	if(usrCntrlOption.cameraCap != 0)
	{
		you.rotState[X] = (you.rotState[X] > usrCntrlOption.cameraCap) ? usrCntrlOption.cameraCap : you.rotState[X];
		you.rotState[Y] = (you.rotState[Y] > usrCntrlOption.cameraCap) ? usrCntrlOption.cameraCap : you.rotState[Y];
		you.rotState[X] = (you.rotState[X] < -usrCntrlOption.cameraCap) ? -usrCntrlOption.cameraCap : you.rotState[X];
		you.rotState[Y] = (you.rotState[Y] < -usrCntrlOption.cameraCap) ? -usrCntrlOption.cameraCap : you.rotState[Y];
	}
	////////////////////////////////////////////////////////////
	//De-rating user-input rotation
	////////////////////////////////////////////////////////////
	if( is_key_up(DIGI_X) && you.rotState[X] < 0) you.rotState[X] += fxm(time_fixed_scale, fxm(JO_ABS(you.rotState[X]), 16384));//A
	if( is_key_up(DIGI_B) && you.rotState[Y] < 0) you.rotState[Y] += fxm(time_fixed_scale, fxm(JO_ABS(you.rotState[Y]), 16384));//S
	if( is_key_up(DIGI_Z) && you.rotState[X] > 0) you.rotState[X] -= fxm(time_fixed_scale, fxm(JO_ABS(you.rotState[X]), 16384));//D
	if( is_key_up(DIGI_Y) && you.rotState[Y] > 0) you.rotState[Y] -= fxm(time_fixed_scale, fxm(JO_ABS(you.rotState[Y]), 16384));//W
	////////////////////////////////////////////////////////////
	//De-rating speed
	////////////////////////////////////////////////////////////
	if(you.IPaccel > 0 && you.dirInp != true) you.IPaccel -= fxm(time_fixed_scale, 1500); 
	if(you.IPaccel < 0 && you.dirInp != true) you.IPaccel = 0;

	if(you.hitSurface != true)
	{
		FIXED setXrotDrate = fxm(fxm((6553), JO_ABS(you.rot[X])), time_fixed_scale);
		FIXED setZrotDrate = fxm(fxm((6553), JO_ABS(you.rot[Z])), time_fixed_scale);
		if(you.rot[X] > 0){
			you.rot[X] -= setXrotDrate;
		}
		if(you.rot[Z] > 0){
			you.rot[Z] -= setZrotDrate;
		}
		if(you.rot[X] < 0){
			you.rot[X] += setXrotDrate;
		}
		if(you.rot[Z] < 0){
			you.rot[Z] += setZrotDrate;
		}
	} //OFF SURFACE ROTATION DERATING ENDIF
		
	//Moment
	you.moment[X] = fxm(you.mass, you.velocity[X]);
	you.moment[Y] = fxm(you.mass, you.velocity[Y]);
	you.moment[Z] = fxm(you.mass, you.velocity[Z]);
	//
	//Delta-V has been time-scaled; it can be reset before we start accumulating it.
	you.dV[X] = 0;
	you.dV[Y] = 0;
	you.dV[Z] = 0;
	
	//you.renderRot[X] = 0;
	//you.renderRot[Y] = 0;
	//you.renderRot[Z] = 0;
	
	// static POINT testUnit = {0, 0, 65536};
	// standing_surface_alignment(testUnit, you.renderRot);
	
	// nbg_sprintf(1, 6, "x(%i)", you.renderRot[X]);
	// nbg_sprintf(1, 7, "y(%i)", you.renderRot[Y]);
	// nbg_sprintf(1, 8, "z(%i)", you.renderRot[Z]);

	
	bound_box_starter.modified_box = &pl_RBB;
	bound_box_starter.x_location = you.pos[X];
	bound_box_starter.y_location = you.pos[Y];
	bound_box_starter.z_location = you.pos[Z];
	
	bound_box_starter.x_rotation = you.renderRot[X];
	bound_box_starter.y_rotation = you.renderRot[Y];
	bound_box_starter.z_rotation = you.renderRot[Z];
	
	bound_box_starter.x_radius = 2<<16;
	bound_box_starter.y_radius = 5<<16;
	bound_box_starter.z_radius = 5<<16;
			
		make2AxisBox(&bound_box_starter);
		//Why is this here?
		// I used to align by angles. Now I align to a matrix.
		// Aligning by angles was technically more efficient since the matrix was only calculated once, in the prior function.
		// By aligning with a matrix, a new matrix is used instead of the one from make2AxisBox.
		// So that is pasted in to the box here.
//		if(pl_RBB.collisionID == BOXID_VOID)
//		{
				//Release from surface
				you.hitMap = false;
				you.hitObject = false;
				you.hitBox = false;
				you.hitSurface = false;
				you.hitWall = false;
				you.aboveObject = false;
		if(pl_RBB.status[0] == 'A')
		{
		finalize_alignment(bound_box_starter.modified_box);
		}
		//The player's velocity is calculated independent of an actual value, so use it here instead.
		you.wvel[X] = -you.velocity[X];
		you.wvel[Y] = -you.velocity[Y];
		you.wvel[Z] = -you.velocity[Z];
		pl_RBB.velocity[X] = you.velocity[X];
		pl_RBB.velocity[Y] = you.velocity[Y];
		pl_RBB.velocity[Z] = you.velocity[Z];

		you.renderRot[X] = you.rot[X];
		you.renderRot[Y] = you.rot[Y];
		you.renderRot[Z] = you.rot[Z];
		
		you.wpos[X] = -you.pos[X];
		you.wpos[Y] = -you.pos[Y];
		you.wpos[Z] = -you.pos[Z];
		
		//Patchwork logic: Every frame you aren't climbing, you need to start as if not climbing.
		//If collisions thusly calculate that you are, great!
		//This is a weird, bad, patchwork system that later on in my career I'll learn how to do better.
		//For now, there's all sorts of weird one-off exception rules that have to happen like this.
		you.climbing = false;
		you.ladder = false;

/*

		short dirZP[3] = {pl_RBB.UVZ[X]>>3,	  pl_RBB.UVZ[Y]>>3, 	 pl_RBB.UVZ[Z]>>3};
		short dirZN[3] = {pl_RBB.UVNZ[X]>>3, pl_RBB.UVNZ[Y]>>3, 	pl_RBB.UVNZ[Z]>>3};
		short dirXP[3] = {pl_RBB.UVX[X]>>3,	  pl_RBB.UVX[Y]>>3, 	 pl_RBB.UVX[Z]>>3};
		short dirXN[3] = {pl_RBB.UVNX[X]>>3, pl_RBB.UVNX[Y]>>3,		pl_RBB.UVNX[Z]>>3};
		short dirYP[3] = {pl_RBB.UVY[X]>>3,	  pl_RBB.UVY[Y]>>3, 	 pl_RBB.UVY[Z]>>3};
		short dirYN[3] = {pl_RBB.UVNY[X]>>3, pl_RBB.UVNY[Y]>>3,		pl_RBB.UVNY[Z]>>3};
		add_to_sprite_list(you.wpos, dirXP, 0,   16	+ (0 * 64), 0, 		'l', 0, 1500);
		add_to_sprite_list(you.scaled_faces.xp0, dirXP, 0, 16	+ (0 * 64), 0, 	'l', 0, 1500);
		add_to_sprite_list(you.scaled_faces.xp1, dirXN, 0, 16	+ (0 * 64), 0, 	'l', 0, 1500);
		add_to_sprite_list(you.wpos, dirYP, 0,   19	+ (0 * 64), 0, 		'l', 0, 1500);
		add_to_sprite_list(you.scaled_faces.yp0, dirYP, 0, 19	+ (0 * 64), 0, 	'l', 0, 1500);
		add_to_sprite_list(you.scaled_faces.yp1, dirYN, 0, 19	+ (0 * 64), 0, 	'l', 0, 1500);
		add_to_sprite_list(you.wpos, dirZP, 0,   17	+ (0 * 64), 0, 		'l', 0, 1500);
		add_to_sprite_list(you.scaled_faces.zp0, dirZP, 0, 17	+ (0 * 64), 0, 	'l', 0, 1500);
		add_to_sprite_list(you.scaled_faces.zp1, dirZN, 0, 17	+ (0 * 64), 0, 	'l', 0, 1500);
 */
	construct_line_tables();
	pl_RBB.boxID = BOXID_PLAYER;
	pl_RBB.collisionID = BOXID_VOID;
	pl_RBB.status[0] = 'R';	
	pl_RBB.status[1] = 'C';	
	pl_RBB.status[2] = 'L';	
}

void	collide_with_heightmap(_boundBox * sbox, _lineTable * moverCFs, _lineTable * moverTimeAxis)
{

POINT below_player = {you.pos[X], you.pos[Y] - (1<<16), you.pos[Z]};
int len_A;
int len_B;

static _pquad nySmp;

static POINT nyNearTriCF1 = {0, 0, 0};
static VECTOR nyTriNorm1 = {0, 0, 0};
static POINT nyNearTriCF2 = {0, 0, 0};
static VECTOR nyTriNorm2 = {0, 0, 0};
static FIXED nyToTri1 = 0;
static FIXED nyToTri2 = 0;

generate_cell_from_position(moverCFs->yp1, &nySmp);


//
// stuff2(nySmp.verts[0], 0);
// stuff2(nySmp.verts[1], 0);
// stuff2(nySmp.verts[2], 0);
// stuff2(nySmp.verts[3], 0);
//

divide_cell_return_cfnorms(&nySmp, nyNearTriCF1, nyTriNorm1, nyNearTriCF2, nyTriNorm2);

//Why add, and not subtract? Because coordinate systems
you.distanceToMapFloor = sbox->pos[Y] + ((nySmp.verts[0][Y] + nySmp.verts[1][Y] + nySmp.verts[2][Y] + nySmp.verts[3][Y])>>2);

FIXED ny_Dist1 =  slSquartFX(fxm(nySmp.verts[1][X] + moverCFs->yp1[X], nySmp.verts[1][X] + moverCFs->yp1[X]) + fxm(nySmp.verts[1][Z] + moverCFs->yp1[Z], nySmp.verts[1][Z] + moverCFs->yp1[Z]));
FIXED ny_Dist2 =  slSquartFX(fxm(nySmp.verts[3][X] + moverCFs->yp1[X], nySmp.verts[3][X] + moverCFs->yp1[X]) + fxm(nySmp.verts[3][Z] + moverCFs->yp1[Z], nySmp.verts[3][Z] + moverCFs->yp1[Z]));

nyToTri1 = realpt_to_plane(moverCFs->yp1, nyTriNorm1, nyNearTriCF1);
nyToTri2 = realpt_to_plane(moverCFs->yp1, nyTriNorm2, nyNearTriCF2);
//Collision detection is using heightmap data

if(nyToTri2 >= 8192 && ny_Dist1 >= ny_Dist2 && (pl_RBB.collisionID == BOXID_VOID || you.hitSurface == false))
{
	you.hitMap = true;
	
	if(!you.setJet)
	{
		you.hitSurface = true;
		
		line_hit_plane_here(moverCFs->yp1, moverCFs->yp0, nyNearTriCF2, nyTriNorm2, alwaysLow, 1<<16, lowPoint);
		you.floorNorm[X] = -nyTriNorm2[X];
		you.floorNorm[Y] = -nyTriNorm2[Y];
		you.floorNorm[Z] = -nyTriNorm2[Z];
		standing_surface_alignment(you.floorNorm);
	
		you.floorPos[X] = ((lowPoint[X]) - (sbox->Yneg[X]) - moverTimeAxis->yp1[X]);
		you.floorPos[Y] = ((lowPoint[Y]) - (sbox->Yneg[Y]) - moverTimeAxis->yp1[Y]);
		you.floorPos[Z] = ((lowPoint[Z]) - (sbox->Yneg[Z]) - moverTimeAxis->yp1[Z]);
		you.shadowPos[X] = lowPoint[X];
		you.shadowPos[Y] = lowPoint[Y];
		you.shadowPos[Z] = lowPoint[Z];
	} else {
		you.wallNorm[X] = -nyTriNorm1[X];
		you.wallNorm[Y] = -nyTriNorm1[Y];
		you.wallNorm[Z] = -nyTriNorm1[Z];
		you.wallPos[X] = -lowPoint[X];
		you.wallPos[Y] = -lowPoint[Y];
		you.wallPos[Z] = -lowPoint[Z];
		
		you.hitWall = true;
	}
	
	pl_RBB.collisionID = BOXID_MAP;
} else if(nyToTri1 >= 8192 && ny_Dist1 < ny_Dist2 && (pl_RBB.collisionID == BOXID_VOID || you.hitSurface == false))
{
	you.hitMap = true;
	
	if(!you.setJet)
	{
		you.hitSurface = true;
		
		line_hit_plane_here(moverCFs->yp1, moverCFs->yp0, nyNearTriCF1, nyTriNorm1, alwaysLow, 1<<16, lowPoint);
		you.floorNorm[X] = -nyTriNorm1[X];
		you.floorNorm[Y] = -nyTriNorm1[Y];
		you.floorNorm[Z] = -nyTriNorm1[Z];
		standing_surface_alignment(you.floorNorm);
	
		you.floorPos[X] = ((lowPoint[X]) - (sbox->Yneg[X]) - moverTimeAxis->yp1[X]);
		you.floorPos[Y] = ((lowPoint[Y]) - (sbox->Yneg[Y]) - moverTimeAxis->yp1[Y]);
		you.floorPos[Z] = ((lowPoint[Z]) - (sbox->Yneg[Z]) - moverTimeAxis->yp1[Z]);
		you.shadowPos[X] = lowPoint[X];
		you.shadowPos[Y] = lowPoint[Y];
		you.shadowPos[Z] = lowPoint[Z];
	} else {
		you.wallNorm[X] = -nyTriNorm1[X];
		you.wallNorm[Y] = -nyTriNorm1[Y];
		you.wallNorm[Z] = -nyTriNorm1[Z];
		you.wallPos[X] = -lowPoint[X];
		you.wallPos[Y] = -lowPoint[Y];
		you.wallPos[Z] = -lowPoint[Z];
		
		you.hitWall = true;
	}
	
	pl_RBB.collisionID = BOXID_MAP;
} else {
	//Find Floor on Map, if not above an object
			if(you.aboveObject != true)
			{
	len_A = unfix_length(you.pos, nyNearTriCF1);
	len_B = unfix_length(you.pos, nyNearTriCF2);
		if(len_A < len_B)
		{
		line_hit_plane_here(you.pos, below_player, nyNearTriCF1, nyTriNorm1, alwaysLow, 1<<16, you.shadowPos);
		} else {
		line_hit_plane_here(you.pos, below_player, nyNearTriCF2, nyTriNorm2, alwaysLow, 1<<16, you.shadowPos);
		}
		
			//slPrintFX(you.shadowPos[X], slLocate(9, 7));
			//slPrintFX(you.shadowPos[Y], slLocate(19, 7));
			//slPrintFX(you.shadowPos[Z], slLocate(29, 7));
			}
	//
	you.hitMap = false;
}

	// nbg_sprintf(0, 11, "(%i)", cellCenterPos[0]);
	// nbg_sprintf(5, 11, "(%i)", cellCenterPos[1]);	

}


