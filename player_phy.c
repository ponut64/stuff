
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
#include "particle.h"
#include "sound.h"

#define PLR_FWD_SPD (65536)
#define PLR_RVS_SPD (65536)
#define PLR_STRF_SPD (65536)

POINT alwaysLow = {0, -(1<<16), 0};
POINT alwaysHigh = {0, (1<<16), 0};
FIXED	lowPoint[XYZ] = {0, 0, 0};

void reset_player(void)
{
	you.mass = 250<<16;

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
	you.IPaccel=0;
	you.timeSinceWallHit = 0;
	you.avg_sanics = 0;
	you.sanic_samples = 0;
	you.distanceToMapFloor = 0;
	you.firstSurfHit = 0;
	you.airTime = 0;
	you.cancelTimers = true;
	you.resetTimers = false;
	
	you.curSector = INVALID_SECTOR;
	you.prevSector = INVALID_SECTOR;
	
	you.actionKeyDef = DIGI_L;
	
	set_viewmodel_from_slot(0);
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
	pcm_play(snd_bstep, PCM_SEMI, 210);
}

void	pl_step_snd(void)
{
	
	if(pl_model.file_done != true) return;
	char runSnd = 0;

	if(runSnd == 1)
	{
		pcm_play(snd_lstep, PCM_PROTECTED, 190);
	}

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

	//All the stuff above this line might get deleted.
	for(int s = 0; s < 3; s++)
	{
		you.realTimeAxis.xp0[s] = you.wpos[s] + you.box.Xplus[s] + 	fxm(you.wvel[s], time_fixed_scale);
		you.realTimeAxis.xp1[s] = you.wpos[s] + you.box.Xneg[s] + 	fxm(you.wvel[s], time_fixed_scale);
		you.realTimeAxis.yp0[s] = you.wpos[s] + you.box.Yplus[s] +	fxm(you.wvel[s], time_fixed_scale);
		you.realTimeAxis.yp1[s] = you.wpos[s] + you.box.Yneg[s] +	fxm(you.wvel[s], time_fixed_scale);
		you.realTimeAxis.zp0[s] = you.wpos[s] + you.box.Zplus[s] + 	fxm(you.wvel[s], time_fixed_scale);
		you.realTimeAxis.zp1[s] = you.wpos[s] + you.box.Zneg[s] + 	fxm(you.wvel[s], time_fixed_scale);
	
	}
	

	
}

void	player_hit_wall(int * wallNorm, int * wallPos)
{
	//Push away.
	//This might be the factor which is making this less functional,
	//considering 1 meter has gone from 8 units to 64 units.
	you.dV[X] -= wallNorm[X]<<1;
	you.dV[Y] -= wallNorm[Y]<<1;
	you.dV[Z] -= wallNorm[Z]<<1;
	
	you.IPaccel = fxm(you.IPaccel, 32768);
	
	//this should be restructured to be less bouncy and more strict
	
	int deflectionFactor = fxdot(you.velocity, wallNorm);
	//This shouldn't be time-scaled, should it?
	you.velocity[X] -= fxm(wallNorm[X], deflectionFactor + REBOUND_ELASTICITY); 
	you.velocity[Y] -= fxm(wallNorm[Y], deflectionFactor + REBOUND_ELASTICITY); 
	you.velocity[Z] -= fxm(wallNorm[Z], deflectionFactor + REBOUND_ELASTICITY); 
		
	you.hitWall = true;
	you.timeSinceWallHit = 1<<16;
	//if(you.sanics >= 7<<16 && !you.setJump) pcm_play(snd_smack, PCM_SEMI, 7);
	
	if(you.sanics >= 3<<16)
	{
	pcm_play(snd_mstep, PCM_SEMI, 200);
	emit_particle_explosion(&HitPuff, PARTICLE_TYPE_NOCOL, wallPos, wallNorm, 8<<16, 8192, 4);
	}
	
	you.wallNorm[X] = wallNorm[X];
	you.wallNorm[Y] = wallNorm[Y];
	you.wallNorm[Z] = wallNorm[Z];
	you.wallPos[X] = wallPos[X];
	you.wallPos[Y] = wallPos[Y];
	you.wallPos[Z] = wallPos[Z];
}

void	smart_cam(void)
{
	///////////////////////////////////////////
	//Smart Camera Setup
	///////////////////////////////////////////
	if(usrCntrlOption.lockTimer > 0)
	{
		usrCntrlOption.lockout = true;
	}
	if(usrCntrlOption.lockTimer > 0)
	{
		usrCntrlOption.lockTimer -= delta_time;
	}


	//////////////////////////////////////////////
	// Re-centering camera
	//////////////////////////////////////////////
	if(usrCntrlOption.facingCam)
	{
		//Determines if we want to rotate view clockwise or counterclockwise (and then does)
		if(!usrCntrlOption.lockout)
		{
			if(JO_ABS(you.viewRot[X]) > 360)
			{
				you.viewRot[X] += (you.viewRot[X] < 0) ? (3 * 182) : -(3 * 182);
			} else {
				you.viewRot[X] = 0;
			}
		}
	}
	usrCntrlOption.lockout = false;
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
	//The control unit vector is using the player's bound box / matrix parameters.
	//In this case, it's the forward vector.
	you.ControlUV[X] = pl_RBB.UVZ[X];
    you.ControlUV[Y] = pl_RBB.UVZ[Y];
    you.ControlUV[Z] = pl_RBB.UVZ[Z];

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
		
		if( you.okayStepSnd ) pl_step_snd();

	////////////////////////////////////////////////////
	//Input-speed response
	////////////////////////////////////////////////////
	if(you.hitSurface == true)
	{
		
		you.dV[X] += fxm(you.IPaccel, you.ControlUV[X]);
		you.dV[Y] += fxm(you.IPaccel, you.ControlUV[Y]);
		you.dV[Z] += fxm(you.IPaccel, you.ControlUV[Z]);

	} else { 
	//If sliding or in the air
	//I don't want this to enable going faster, but I do want it to help you turn?
	//I also want to increase turning authority at higher speeds?
	
		you.dV[X] += fxm(fxm(you.IPaccel, you.ControlUV[X]), 3000);
		you.dV[Y] += fxm(fxm(you.IPaccel, you.ControlUV[Y]), 3000);
		you.dV[Z] += fxm(fxm(you.IPaccel, you.ControlUV[Z]), 3000);

	}

	////////////////////////////////////////////////////
	//On-surface collision response
	////////////////////////////////////////////////////
	int deflectionFactor = 0;
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
			pcm_play(snd_mstep, PCM_PROTECTED, 210);
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
		
		//Special Condition - no friction on surface contact + jump
		if(!you.setJump)
		{
			//Friction decisions
			you.dV[X] -= fxm(you.velocity[X], (you.surfFriction));
			you.dV[Y] -= fxm(you.velocity[Y], (you.surfFriction));
			you.dV[Z] -= fxm(you.velocity[Z], (you.surfFriction));
		}
		
		//Stiction; low velocities will trap at zero on surface.
		if(!you.dirInp && !you.setJump)
		{
			you.velocity[X] = (JO_ABS(you.velocity[X]) > 6553) ? you.velocity[X] : 0;
			you.velocity[Y] = (JO_ABS(you.velocity[Y]) > 6553) ? you.velocity[Y] : 0;
			you.velocity[Z] = (JO_ABS(you.velocity[Z]) > 6553) ? you.velocity[Z] : 0;
			you.dV[X] = (JO_ABS(you.velocity[X]) > 6553) ? you.dV[X] : 0;
			you.dV[Y] = (JO_ABS(you.velocity[Y]) > 6553) ? you.dV[Y] : 0;
			you.dV[Z] = (JO_ABS(you.velocity[Z]) > 6553) ? you.dV[Z] : 0;
		}
		
		you.pos[X] = (you.floorPos[X]);
		you.pos[Y] = (you.floorPos[Y]) + (you.box.radius[Y] - (1<<16));
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
	if(you.timeSinceWallHit > 0 && you.curSector == INVALID_SECTOR)
	{
		you.pos[X] = you.prevPos[X];
		you.pos[Y] = you.prevPos[Y];
		you.pos[Z] = you.prevPos[Z];
		you.curSector = you.prevSector;
		you.timeSinceWallHit = 1<<16;
	}
	
		//Clear wall flag, decrement wall timer
		you.hitWall = false;
		you.timeSinceWallHit -= delta_time;

	//Ladder/Climb Escape Sequence
	//At least in one test, this was pretty much perfect!
	if(!you.climbing && you.wasClimbing)
	{
		you.dV[X] += fxm(you.floorNorm[X], 32768);
        you.dV[Y] += 32768;
        you.dV[Z] += fxm(you.floorNorm[Z], 32768);
		you.wasClimbing = false;
	}
	if(you.hitSurface && pl_RBB.surfID >= 0)
	{
		_boundBox * on_box = &RBBs[dWorldObjects[pl_RBB.surfID].bbnum];
		//Not sure how to handle this part.
		//Need to add velocity to player in a way that doesn't vanish when player exits
		//I guess friction applies here? There is probably some ratio I don't understand.
		you.velocity[X] -= fxm(on_box->velocity[X], you.surfFriction);
		you.velocity[Y] -= fxm(on_box->velocity[Y], you.surfFriction);
		you.velocity[Z] -= fxm(on_box->velocity[Z], you.surfFriction);
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
	quick_normalize(tempDif, you.DirUV);
	you.sanics = slSquartFX(fxm(tempDif[X], tempDif[X]) + fxm(tempDif[Y], tempDif[Y]) + fxm(tempDif[Z], tempDif[Z]));
	you.sanics = fxm((you.sanics>>5), time_delta_scale);
	//Set prev pos
	if(you.curSector != INVALID_SECTOR)
	{
	you.prevPos[X] = you.pos[X];
	you.prevPos[Y] = you.pos[Y];
	you.prevPos[Z] = you.pos[Z];
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
	//Deriving the view unit vector from the final orientation
	////////////////////////////////////////////////////////////
	you.uview[X] = 0;
	you.uview[Y] = 0;
	you.uview[Z] = -(1<<16);
	int passa[3] = {0, 0, 0};
	fxrotX(you.uview, passa, -you.viewRot[X]);
	fxrotY(passa, you.uview, -you.viewRot[Y]); 
	
	you.viewPos[X] = you.wpos[X];
	you.viewPos[Y] = you.wpos[Y] - ((PLAYER_Y_SIZE>>1) + (2<<16));
	you.viewPos[Z] = you.wpos[Z];
	/////////////////////////////
	// Set the position projectiles will emanate from
	// Kind of difficult / not straight-forward
	// So this has to be a point, placed at a known location, which is then sine/cosined...
	you.shootPos[X] = -(5<<16); //Bit to the right
	you.shootPos[Y] = 7<<16; //Bit down
	you.shootPos[Z] = 10<<16; //Bit forward
	fxrotX(you.shootPos, passa, -you.viewRot[X]);
	fxrotY(passa, you.shootPos, -you.viewRot[Y]);
	you.shootPos[X] += you.viewPos[X];
	you.shootPos[Y] += you.viewPos[Y];
	you.shootPos[Z] += you.viewPos[Z];
	
	if(you.hasValidAim)
	{
		passa[X] = (you.shootPos[X] - you.hitscanPt[X])>>4;
		passa[Y] = (you.shootPos[Y] - you.hitscanPt[Y])>>4;
		passa[Z] = (you.shootPos[Z] - you.hitscanPt[Z])>>4;
		quick_normalize(passa, you.shootDir);
	} else {
		you.shootDir[X] = -you.uview[X];
		you.shootDir[Y] = -you.uview[Y];
		you.shootDir[Z] = -you.uview[Z];
		you.hitscanPt[X] = you.viewPos[X] + (you.uview[X]<<8);
		you.hitscanPt[Y] = you.viewPos[Y] + (you.uview[Y]<<8);
		you.hitscanPt[Z] = you.viewPos[Z] + (you.uview[Z]<<8);
	}
	//Initial state setting for aim point
	you.hasValidAim = false;
	////////////////////////////////////////////////////////////
	// Set the crosshairs to be drawn
	////////////////////////////////////////////////////////////
	short sprSpan[3] = {10, 10, 10};
	//I need this to *not* be what it is... drawn on master, no interaction with list.
	sprite_prep.info.drawMode = SPRITE_TYPE_UNSCALED_BILLBOARD;
	sprite_prep.info.drawOnce = 1;
	sprite_prep.info.mesh = 0;
	sprite_prep.info.sorted = 0;
	add_to_sprite_list(you.hitscanPt, sprSpan, 'X', 5, sprite_prep, 0, 0);
	////////////////////////////////////////////////////////////
	//De-rating speed
	////////////////////////////////////////////////////////////
	if(you.IPaccel > 0 && you.dirInp != true) you.IPaccel -= fxm(time_fixed_scale, MOVEMENT_DECAY_RATE); 
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
	
	//Screen Viewmodel ("the gun") view bob
	//first, get a fixed-point scale from 0 to 4 sanics
	int bobrate = you.sanics;
	if(bobrate >= (8<<16)) bobrate = (8<<16);
	bobrate = fxdiv(bobrate, 8<<16);
	static int bobAngle = 0;
	bobAngle += fxm(bobrate, 1024);
	you.viewmodel_offset[X] = fxm(slCos(bobAngle), 2<<16)>>16;
	you.viewmodel_offset[Y] = (fxm(slSin(bobAngle), 4<<16)>>16)-4;
		
	//The player's velocity is calculated independent of an actual value, so use it here instead.
	you.wvel[X] = -you.velocity[X];
	you.wvel[Y] = -you.velocity[Y];
	you.wvel[Z] = -you.velocity[Z];

	you.renderRot[X] = you.rot[X];
	you.renderRot[Y] = you.rot[Y];
	you.renderRot[Z] = you.rot[Z];
	
	you.wpos[X] = -you.pos[X];
	you.wpos[Y] = -you.pos[Y];
	you.wpos[Z] = -you.pos[Z];
		
	//Moment
	you.moment[X] = fxm(you.mass, you.velocity[X]);
	you.moment[Y] = fxm(you.mass, you.velocity[Y]);
	you.moment[Z] = fxm(you.mass, you.velocity[Z]);
	//
	//Delta-V has been time-scaled; it can be reset before we start accumulating it.
	you.dV[X] = 0;
	you.dV[Y] = 0;
	you.dV[Z] = 0;
	
	bound_box_starter.modified_box = &pl_RBB;
	bound_box_starter.x_location = you.pos[X];
	bound_box_starter.y_location = you.pos[Y];
	bound_box_starter.z_location = you.pos[Z];
	
	bound_box_starter.x_rotation = you.renderRot[X];
	bound_box_starter.y_rotation = you.renderRot[Y];
	bound_box_starter.z_rotation = you.renderRot[Z];
	
	bound_box_starter.x_radius = PLAYER_X_SIZE>>1;
	bound_box_starter.y_radius = PLAYER_Y_SIZE>>1;
	bound_box_starter.z_radius = PLAYER_Z_SIZE>>1;
			
	pl_RBB.velocity[X] = you.velocity[X];
	pl_RBB.velocity[Y] = you.velocity[Y];
	pl_RBB.velocity[Z] = you.velocity[Z];
	/////////////////////////////////////////////////////////////////////
	// world pos?
	/////////////////////////////////////////////////////////////////////
		make2AxisBox(&bound_box_starter);
		
	bound_box_starter.modified_box = &you.box;
	bound_box_starter.x_location = you.wpos[X];
	bound_box_starter.y_location = you.wpos[Y];
	bound_box_starter.z_location = you.wpos[Z];
	
	bound_box_starter.x_rotation = 0;
	bound_box_starter.y_rotation = 0;
	bound_box_starter.z_rotation = 0;
	
	you.box.velocity[X] = you.wvel[X];
	you.box.velocity[Y] = you.wvel[Y];
	you.box.velocity[Z] = you.wvel[Z];
		
		make2AxisBox(&bound_box_starter);
		
		//Why is this here?
		// I used to align by angles. Now I align to a matrix.
		// Aligning by angles was technically more efficient since the matrix was only calculated once, in the prior function.
		// By aligning with a matrix, a new matrix is used instead of the one from make2AxisBox.
		// So that is pasted in to the box here.
				//Release from surface
				you.hitMap = false;
				you.hitObject = false;
				you.hitBox = false;
				you.hitSurface = false;
				you.aboveObject = false;
		if(pl_RBB.status[0] == 'A')
		{
		finalize_alignment(bound_box_starter.modified_box);
		}
		
		//Patchwork logic: Every frame you aren't climbing, you need to start as if not climbing.
		//If collisions thusly calculate that you are, great!
		//This is a weird, bad, patchwork system that later on in my career I'll learn how to do better.
		//For now, there's all sorts of weird one-off exception rules that have to happen like this.
		you.climbing = false;
		you.ladder = false;

	construct_line_tables();
	pl_RBB.boxID = BOXID_PLAYER;
	pl_RBB.collisionID = BOXID_VOID;
	pl_RBB.surfID = BOXID_VOID;
	pl_RBB.status[0] = 'R';	
	pl_RBB.status[1] = 'C';	
	pl_RBB.status[2] = 'L';	
}

