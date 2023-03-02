
#include <sl_def.h>
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

#include "player_phy.h"

#define PLR_FWD_SPD (32768)
#define PLR_RVS_SPD (32768)
#define PLR_STRF_SPD (32768)
#define GRAVITY (6553)

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
    you.Force[X]=0;
    you.Force[Y]=0;
    you.Force[Z]=0;
	you.IPaccel=0;
	you.id = 0;
	you.power = 0;
	you.maxPower = 4;
}


void pl_jump(void){
		if(you.velocity[Y] > 0){
			you.velocity[Y] = -fxm(you.floorNorm[Y], you.velocity[Y]);
		} else {
			you.velocity[Y] = 0;
		}
		you.hitSurface = false;
		you.pos[Y] += (GRAVITY);
			you.velocity[X] -= fxm(196608, you.floorNorm[X]); 
			you.velocity[Y] -= fxm(196608, you.floorNorm[Y]);
			you.velocity[Z] -= fxm(196608, you.floorNorm[Z]);
		pcm_play(snd_bstep, PCM_SEMI, 7);
}

void pl_jet(void){
		if(you.hitSurface == true && you.power == you.maxPower)
		{
			//I guess a micro-jump?
			//It helps release you from the surface when jetting.
			you.velocity[Y] += fxm(GRAVITY<<1, frmul)<<2;
		}
	
		you.hitSurface = false;
		you.power -= 1;
			if(you.dirInp == true)
			{
			//Double-up when in jet mode. Same as in-air or sliding velocity addition.
			you.velocity[X] += fxm(fxm(you.IPaccel, you.ControlUV[X]), 3000);
			you.velocity[Y] += fxm(GRAVITY, frmul); 
			you.velocity[Z] += fxm(fxm(you.IPaccel, you.ControlUV[Z]), 3000);
			} else {
			you.velocity[Y] += fxm(GRAVITY, frmul); 
			}
		pcm_play(snd_bwee, PCM_PROTECTED, 7);
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
	short spr_span[3] = {1,1,1};

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
					add_to_sprite_list(hf_pos, spr_span /*Span*/, 0 /*texno*/, 1 /*mesh Bool*/, 'B', 0 /*no clip*/, 1<<16);
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
		pcm_play(snd_lstep, PCM_SEMI, 6);
	}
	
	oldHoofSetBools[0] = hoofSetBools[0];
	oldHoofSetBools[1] = hoofSetBools[1];
	oldHoofSetBools[2] = hoofSetBools[2];
	oldHoofSetBools[3] = hoofSetBools[3];
	oldHoofSetBools[4] = hoofSetBools[4];

}

void	smart_cam(void)
{
	
	//Smart Camera Setup
	// "uview" is the discrete vector notation of the player's viewport.
	VECTOR uview = {-slSin(you.viewRot[Y]), slSin(you.viewRot[X]), slCos(you.viewRot[Y])};
	// proportion_y is a mathemagical value that is the positive or negative propotion of the view vector,
	// as compared to the movement vector. While we are doing multiplication here, it's ostensibly division (these are <1 values).
	int proportion_y = (fxm((you.DirUV[X] - uview[X]),(you.DirUV[X] - uview[X])) + fxm((you.DirUV[Z] - uview[Z]),(you.DirUV[Z] - uview[Z])))>>7;
	//angDif_y is an expression of how different the movement vector and viewing vector is,
	//as it relates to the axis that Y view rotation controls (X and Z).
	short angDif_y = (slAtan(you.DirUV[X], you.DirUV[Z]) - slAtan(uview[X], uview[Z]));
	
	//This is simply the difference between the movement vector's Y and the viewing vector's Y.
	//prop y and prop x are shifted down to scale it as desired. Magic shift, in other words.
	int proportion_x = (you.DirUV[Y] - uview[Y])>>7;
	//This angle will amount to a proportion of angle we're not yet facing towards the ground.
	int propotion_facing_ground = (-32768 - uview[Y])>>7;
	//Will pivot camera towards direction of motion
	if((JO_ABS(you.velocity[X]) > 1024 || JO_ABS(you.velocity[Z]) > 1024) &&  JO_ABS(angDif_y) > 1024 &&
	(is_key_up(DIGI_DOWN) | is_key_down(DIGI_LEFT) | is_key_down(DIGI_RIGHT)))
	{
		//Determines if we want to rotate view clockwise or counterclockwise (and then does)
		you.viewRot[Y] += (angDif_y > 0) ? (proportion_y * framerate)>>1 : -(proportion_y * framerate)>>1; 
	}
	if(JO_ABS(you.velocity[Y]) > 1024 || you.dirInp == true)
	{
		//If we are on the ground, we want a camera that'll tilt up and down if we're going up or down.
		//If we are not, we just want the camera to tilt downwards so we can see where we are going to land.
		you.viewRot[X] += (you.hitSurface == true) ? (proportion_x * framerate)>>1 : (propotion_facing_ground * framerate)>>1;

	}
	
}
 
static const int airFriction = 65400;
 
void	player_phys_affect(void)
{
	if(you.hitBox != 1 && you.hitObject != 1 && you.hitMap != 1)
	{
				//Release from surface
				//you.hitWall = false;
				you.hitSurface = false; 
	}
	
		// nbg_sprintf(1, 6, "hitObject: (%i)", you.hitObject);
		// nbg_sprintf(1, 7, "hitBox: (%i)", you.hitBox);
		// nbg_sprintf(1, 8, "hitMap: (%i)", you.hitMap);			
		// nbg_sprintf(1, 9, "hitWall: (%i)", you.hitWall);
		// nbg_sprintf(1, 12, "hitSurface: (%i)", you.hitSurface);
		// slPrintFX(time_in_seconds, slLocate(1, 14));
	
	//Derive three angles from two inputs.
	you.viewRot[X] += you.rotState[Y];
	you.viewRot[Y] -= you.rotState[X];
	
	//Make the velocity unit vector from the current player's rotation.
	// Why isn't this the Z-unit vector of the player's bound box?
	// Sometimes, you can push player off-axis of model's rotation.
	if(you.setSlide != true)
	{
	you.ControlUV[X] = fxm(slSin(you.rot[Y]), slCos(you.renderRot[X]));
	you.ControlUV[Y] = slSin(you.renderRot[X]);
	you.ControlUV[Z] = fxm(slCos(you.rot[Y]), slCos(you.renderRot[X]));
	// you.ControlUV[X] = pl_RBB.UVZ[X];
    // you.ControlUV[Y] = pl_RBB.UVZ[Y];
    // you.ControlUV[Z] = pl_RBB.UVZ[Z];
	} else if(you.setSlide == true){
	you.ControlUV[X] = fxm(slSin(you.rot2[Y]), slCos(you.renderRot[X]));
	you.ControlUV[Y] = slSin(you.renderRot[X]);
	you.ControlUV[Z] = fxm(slCos(you.rot2[Y]), slCos(you.renderRot[X]));
	} else if(you.climbing == true)
	{
	you.ControlUV[X] = pl_RBB.UVZ[X];
    you.ControlUV[Y] = pl_RBB.UVZ[Y];
    you.ControlUV[Z] = pl_RBB.UVZ[Z];
	}
	
	if(!you.climbing)
	{
	smart_cam();
	}
	//
	
	//F = m * a : This comment means nothing. This math isn't here nor there.
	//A = F / M :
	
		//There's always air .. unless we go into space, but whatevs, bruh
		you.velocity[X] = fxm(you.velocity[X], (airFriction));
		you.velocity[Y] = fxm(you.velocity[Y], (airFriction));
		you.velocity[Z] = fxm(you.velocity[Z], (airFriction));

							if(you.hitSurface == true){
		//Skiing Decisions
		if(you.setSlide == true){
			you.surfFriction = 65000;
		}
								
		//Friction decisions
			you.velocity[X] = fxm(you.velocity[X], (you.surfFriction));
			you.velocity[Y] = fxm(you.velocity[Y], (you.surfFriction));
			you.velocity[Z] = fxm(you.velocity[Z], (you.surfFriction));

		if(!you.dirInp)
		{
			you.velocity[X] = (JO_ABS(you.velocity[X]) > 6553) ? you.velocity[X] : 0;
			you.velocity[Y] = (JO_ABS(you.velocity[Y]) > 6553) ? you.velocity[Y] : 0;
			you.velocity[Z] = (JO_ABS(you.velocity[Z]) > 6553) ? you.velocity[Z] : 0;
		}
							}
						

	//Surface / gravity decisions
	static VECTOR gravAcc;
	if(you.hitSurface == true){
		///When on surface, I need to make sure Y velocity applied here by gravity does not increase in a way that opposes the surface normal.
		///Also, stiction. You shouldn't ALWAYS slide :)
		gravAcc[X] = -fxm(fxm((GRAVITY), frmul), you.floorNorm[X]); //Transform gravity by the surface
		gravAcc[Y] = -fxm(fxm((GRAVITY), frmul), you.floorNorm[Y]);
		gravAcc[Z] = -fxm(fxm((GRAVITY), frmul), you.floorNorm[Z]);
		you.velocity[X] += (JO_ABS(gravAcc[X]) >= 16384 || you.setSlide == true || you.sanics >= 65536) ? gravAcc[X] : 0;
		you.velocity[Y] += (JO_ABS(gravAcc[Y]) >= 16384 || you.setSlide == true || you.sanics >= 65536) ? gravAcc[Y] : 0;
		you.velocity[Z] += (JO_ABS(gravAcc[Z]) >= 16384 || you.setSlide == true || you.sanics >= 65536) ? gravAcc[Z] : 0;
		you.velocity[Y] += fxm(you.velocity[Y], you.floorNorm[Y]); //Don't get Y velocity against the floor
		//'floorPos' is a positive world-space position. Your velocity is added to it if you hit an object.
		if(you.hitObject || you.hitBox){
			//Because your velocity is added to the floor position in this case, subtract it.
			you.pos[X] = (you.floorPos[X]) - you.velocity[X];
			you.pos[Y] = (you.floorPos[Y]) - you.velocity[Y];
			you.pos[Z] = (you.floorPos[Z]) - you.velocity[Z];
		} else {
			you.pos[X] = (you.floorPos[X]);
			you.pos[Y] = (you.floorPos[Y]);
			you.pos[Z] = (you.floorPos[Z]);
		}
	} else {
		you.velocity[Y] -= fxm((GRAVITY), frmul);
	}
	
	/////////////////////////////
	// Jump & Jet Decisions
	/////////////////////////////
		if(you.setJump == true){
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
			if(powerTimer > (255))
			{
				you.power += 1;
				powerTimer = 0;
			}
		}
		nbg_sprintf(1, 4, "Fuel: (%i)", you.power);

	//
		//velocity add by input decisions
		//Acclimate speed on each axis to your rotation on each axis defined by two-axis input
		if(you.setSlide != true && you.hitSurface == true)
		{
			you.velocity[X] += fxm(you.IPaccel, you.ControlUV[X]);
			you.velocity[Y] += fxm(you.IPaccel, you.ControlUV[Y]);
			you.velocity[Z] += fxm(you.IPaccel, you.ControlUV[Z]);
		} else { 
		//If sliding or in the air
		//I don't want this to enable going faster, but I do want it to help you turn.
		//I also want to increase turning authority at higher speeds.
		//Also, when you're sliding, you can't go backwards lol
			you.velocity[X] += fxm(fxm(you.IPaccel, you.ControlUV[X]), 3000);
			you.velocity[Y] += fxm(fxm(you.IPaccel, you.ControlUV[Y]), 3000);
			you.velocity[Z] += fxm(fxm(you.IPaccel, you.ControlUV[Z]), 3000);
		}
		
	//Wall Collision Decisions
	if(you.hitWall == true){
		//'wallPos' is a negative world-space position, that is calculated with player velocity added. So subtract it.
	// you.pos[X] = you.prevPos[X] + (you.prevPos[X] + you.wallPos[X]) - you.velocity[X];
	// you.pos[Y] = you.prevPos[Y] + (you.prevPos[Y] + you.wallPos[Y]) - you.velocity[Y];
	// you.pos[Z] = you.prevPos[Z] + (you.prevPos[Z] + you.wallPos[Z]) - you.velocity[Z];
	
	// you.velocity[X] += (you.velocity[X] > 0) ? -fxm(you.wallNorm[X], you.velocity[X]) : fxm(you.wallNorm[X], you.velocity[X]);
	// you.velocity[Y] += (you.velocity[Y] > 0) ? -fxm(you.wallNorm[Y], you.velocity[Y]) : fxm(you.wallNorm[Y], you.velocity[Y]);
	// you.velocity[Z] += (you.velocity[Z] > 0) ? -fxm(you.wallNorm[Z], you.velocity[Z]) : fxm(you.wallNorm[Z], you.velocity[Z]);

	// you.hitWall = false;
	
	you.climbing = true;
	you.pos[X] = -(you.wallPos[X]);// + pl_RBB.Yneg[X]);
	you.pos[Y] = -(you.wallPos[Y]);// + pl_RBB.Yneg[Y]);
	you.pos[Z] = -(you.wallPos[Z]);// + pl_RBB.Yneg[Z]);
	
	you.velocity[X] = fxm(you.IPaccel>>1, pl_RBB.UVZ[X]);
	you.velocity[Y] = fxm(you.IPaccel>>1, pl_RBB.UVZ[Y]);
	you.velocity[Z] = fxm(you.IPaccel>>1, pl_RBB.UVZ[Z]);
	
	you.sanics = 0;
	// you.rot[Y] = 0;
	// you.renderRot[Y] = 0;
	//standing_surface_alignment(you.wallNorm, you.renderRot);
	if(you.sanics >= 5<<16) pcm_play(snd_smack, PCM_SEMI, 7);
	}

	//Add your speed to your position (incremental / per-frame)
	you.pos[X] += fxm(you.velocity[X], frmul);
	you.pos[Y] += fxm(you.velocity[Y], frmul);
	you.pos[Z] += fxm(you.velocity[Z], frmul);
	//Create a true direction vector, independent of control vector
	static VECTOR tempDif = {0, 0, 0};
	tempDif[X] = you.pos[X] - you.prevPos[X];
	tempDif[Y] = you.pos[Y] - you.prevPos[Y];
	tempDif[Z] = you.pos[Z] - you.prevPos[Z];
	normalize(tempDif, you.DirUV);
	you.sanics = slSquartFX(fxm(tempDif[X], tempDif[X]) + fxm(tempDif[Y], tempDif[Y]) + fxm(tempDif[Z], tempDif[Z]));

	//Sound that plays louder the faster you go. Only initiates at all once you are past 3 in sanics.
	unsigned char windVol = ((you.sanics>>17) < 7) ? ((you.sanics>>17)+1) : 7;
	if(you.sanics > (3<<16))
	{
	pcm_play(snd_wind, PCM_FWD_LOOP, windVol); 
	} else {
	pcm_cease(snd_wind);
	}
	//slPrintFX(you.sanics, slLocate(0, 8));
		
	//Movement and rotation speed maximum and minimums
	if(you.IPaccel >= PLR_FWD_SPD) you.IPaccel = PLR_FWD_SPD;
	if(you.IPaccel <= -PLR_FWD_SPD) you.IPaccel = -PLR_FWD_SPD;
	//
	if(JO_ABS(you.rotState[X]) < 90) you.rotState[X] = 0;
	if(JO_ABS(you.rotState[Y]) < 90) you.rotState[Y] = 0;
	//De-rating
	if( is_key_up(DIGI_X) && you.rotState[X] < 0) you.rotState[X] += fxm(frmul, fxm(JO_ABS(you.rotState[X]), 16384));//A
	if( is_key_up(DIGI_B) && you.rotState[Y] < 0) you.rotState[Y] += fxm(frmul, fxm(JO_ABS(you.rotState[Y]), 16384));//S
	if( is_key_up(DIGI_Z) && you.rotState[X] > 0) you.rotState[X] -= fxm(frmul, fxm(JO_ABS(you.rotState[X]), 16384));//D
	if( is_key_up(DIGI_Y) && you.rotState[Y] > 0) you.rotState[Y] -= fxm(frmul, fxm(JO_ABS(you.rotState[Y]), 16384));//W
	
	if(you.IPaccel > 0 && you.dirInp != true) you.IPaccel = fxm(slDivFX(frmul , 65536), you.IPaccel);
	if(you.IPaccel < 0 && you.dirInp != true) you.IPaccel += spdfactr;

		if(you.hitSurface != true){
			FIXED setXrotDrate = fxm(fxm((6553), JO_ABS(you.rot[X])), frmul);
			FIXED setZrotDrate = fxm(fxm((6553), JO_ABS(you.rot[Z])), frmul);
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
	if(you.hitWall != true || you.climbing == true){
	you.prevPos[X] = you.pos[X];
	you.prevPos[Y] = you.pos[Y];
	you.prevPos[Z] = you.pos[Z];
	}
	
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
		finalize_alignment(bound_box_starter.modified_box);
		//The player's velocity is calculated independent of an actual value, so use it here instead.
		pl_RBB.velocity[X] = you.velocity[X];
		pl_RBB.velocity[Y] = you.velocity[Y];
		pl_RBB.velocity[Z] = you.velocity[Z];

		you.renderRot[X] = you.rot[X];
		you.renderRot[Y] = you.rot[Y];
		you.renderRot[Z] = you.rot[Z];


	pl_RBB.boxID = 0;
	pl_RBB.status[0] = 'R';	
	pl_RBB.status[1] = 'C';	
	pl_RBB.status[2] = 'L';	
}

void	collide_with_heightmap(_boundBox * sbox)
{

static Bool firstSurfHit = false;

static _lineTable realCFs;

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

realCFs.xp0[X] = sbox->Xplus[X] + sbox->pos[X]; realCFs.xp0[Y] = sbox->Xplus[Y] + sbox->pos[Y]; realCFs.xp0[Z] = sbox->Xplus[Z] + sbox->pos[Z];
realCFs.xp1[X] = sbox->Xneg[X] + sbox->pos[X]; realCFs.xp1[Y] = sbox->Xneg[Y] + sbox->pos[Y]; realCFs.xp1[Z] = sbox->Xneg[Z] + sbox->pos[Z];
realCFs.yp0[X] = sbox->Yplus[X] + sbox->pos[X]; realCFs.yp0[Y] = sbox->Yplus[Y] + sbox->pos[Y]; realCFs.yp0[Z] = sbox->Yplus[Z] + sbox->pos[Z];
realCFs.yp1[X] = sbox->Yneg[X] + sbox->pos[X]; realCFs.yp1[Y] = sbox->Yneg[Y] + sbox->pos[Y]; realCFs.yp1[Z] = sbox->Yneg[Z] + sbox->pos[Z];
realCFs.zp0[X] = sbox->Zplus[X] + sbox->pos[X]; realCFs.zp0[Y] = sbox->Zplus[Y] + sbox->pos[Y]; realCFs.zp0[Z] = sbox->Zplus[Z] + sbox->pos[Z];
realCFs.zp1[X] = sbox->Zneg[X] + sbox->pos[X]; realCFs.zp1[Y] = sbox->Zneg[Y] + sbox->pos[Y]; realCFs.zp1[Z] = sbox->Zneg[Z] + sbox->pos[Z];

generate_cell_from_position(realCFs.yp1, &nySmp);


//
// stuff2(nySmp.verts[0], 0);
// stuff2(nySmp.verts[1], 0);
// stuff2(nySmp.verts[2], 0);
// stuff2(nySmp.verts[3], 0);
//

divide_cell_return_cfnorms(nySmp, nyNearTriCF1, nyTriNorm1, nyNearTriCF2, nyTriNorm2);

FIXED ny_Dist1 =  slSquartFX(fxm(nySmp.verts[1][X] + realCFs.yp1[X], nySmp.verts[1][X] + realCFs.yp1[X]) + fxm(nySmp.verts[1][Z] + realCFs.yp1[Z], nySmp.verts[1][Z] + realCFs.yp1[Z]));
FIXED ny_Dist2 =  slSquartFX(fxm(nySmp.verts[3][X] + realCFs.yp1[X], nySmp.verts[3][X] + realCFs.yp1[X]) + fxm(nySmp.verts[3][Z] + realCFs.yp1[Z], nySmp.verts[3][Z] + realCFs.yp1[Z]));

nyToTri1 = realpt_to_plane(realCFs.yp1, nyTriNorm1, nyNearTriCF1);
nyToTri2 = realpt_to_plane(realCFs.yp1, nyTriNorm2, nyNearTriCF2);
//Collision detection is using heightmap data

if(nyToTri2 >= 8192 && ny_Dist1 >= ny_Dist2 && (you.hitObject == false && you.hitBox == false)){
	you.hitMap = true;
	you.hitSurface = true;
	
	line_hit_plane_here(realCFs.yp1, realCFs.yp0, nyNearTriCF2, nyTriNorm2, alwaysLow, 1<<16, lowPoint);
	you.floorNorm[X] = -nyTriNorm2[X];
	you.floorNorm[Y] = -nyTriNorm2[Y];
	you.floorNorm[Z] = -nyTriNorm2[Z];
	standing_surface_alignment(you.floorNorm, you.renderRot);

	you.floorPos[X] = ((lowPoint[X]) - (sbox->Yneg[X]));
	you.floorPos[Y] = ((lowPoint[Y]) - (sbox->Yneg[Y]));
	you.floorPos[Z] = ((lowPoint[Z]) - (sbox->Yneg[Z]));
	you.shadowPos[X] = lowPoint[X];
	you.shadowPos[Y] = lowPoint[Y];
	you.shadowPos[Z] = lowPoint[Z];
} else if(nyToTri1 >= 8192 && ny_Dist1 < ny_Dist2 && (you.hitObject == false && you.hitBox == false)){
	you.hitMap = true;
	you.hitSurface = true;
	
	line_hit_plane_here(realCFs.yp1, realCFs.yp0, nyNearTriCF1, nyTriNorm1, alwaysLow, 1<<16, lowPoint);
	you.floorNorm[X] = -nyTriNorm1[X];
	you.floorNorm[Y] = -nyTriNorm1[Y];
	you.floorNorm[Z] = -nyTriNorm1[Z];
	standing_surface_alignment(you.floorNorm, you.renderRot);

	you.floorPos[X] = ((lowPoint[X]) - (sbox->Yneg[X]));
	you.floorPos[Y] = ((lowPoint[Y]) - (sbox->Yneg[Y]));
	you.floorPos[Z] = ((lowPoint[Z]) - (sbox->Yneg[Z]));
	you.shadowPos[X] = lowPoint[X];
	you.shadowPos[Y] = lowPoint[Y];
	you.shadowPos[Z] = lowPoint[Z];
	
} else {
	//Find Floor on Map, if not above an object
			if(you.aboveObject != true){
	len_A = unfix_length(you.pos, nyNearTriCF1);
	len_B = unfix_length(you.pos, nyNearTriCF2);
		if(len_A < len_B)
		{
		line_hit_plane_here(you.pos, below_player, nyNearTriCF1, nyTriNorm1, alwaysLow, 1<<16, you.shadowPos);
		} else {
		line_hit_plane_here(you.pos, below_player, nyNearTriCF2, nyTriNorm2, alwaysLow, 1<<16, you.shadowPos);
		}
			}
	//
	you.hitMap = false;
}

	if(you.hitSurface == true || you.hitMap == true){
		
		you.surfFriction = (45875);

		if(firstSurfHit == false || (JO_ABS(you.floorNorm[Y]) < 49152 /*&& you.setSlide != true*/)){

			//Bounce and, incidentally, a decent way to discourage you from going up steep slopes.
			//This is sourced from an article on Tribes physics. It really helps to understand *bounce*.
			//At first, I tried deflection formulas. Or just scaling the normal into the velocity.
			//Or scaling the velocity by some half and not-half of the normal.
			//What we have below is an "impact dot" by the deflection factor.
			//It's simple and it works because the impact dot is bigger the more oblique the impact angle, AND the faster you are going.
			//That's exactly the math I wanted but was too stupid to see how it should be implemented on the velocity.
			//"0xFFFF" is the elasticity factor. Here, it's just 1.
			FIXED deflectionFactor = fxdot(you.velocity, you.floorNorm);
			// This is ESSENTIAL for the momentum gameplay to work properly 
			you.velocity[X] -= fxm(you.floorNorm[X], deflectionFactor + 0xFFFF); 
			you.velocity[Y] -= fxm(you.floorNorm[Y], deflectionFactor + 0xFFFF); 
			you.velocity[Z] -= fxm(you.floorNorm[Z], deflectionFactor + 0xFFFF); 
			

			firstSurfHit = true;
		}
		
	} else {
		firstSurfHit = false;
	}

	// nbg_sprintf(0, 11, "(%i)", cellCenterPos[0]);
	// nbg_sprintf(5, 11, "(%i)", cellCenterPos[1]);	

}


