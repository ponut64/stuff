
#include "player_phy.h"
#define PLR_FWD_SPD (32768)
#define PLR_RVS_SPD (32768)
#define PLR_STRF_SPD (32768)
#define GRAVITY (6553)

POINT alwaysLow = {0, (-1<<16), 0};
POINT alwaysHigh = {0, (1<<16), 0};
FIXED	lowPoint[XYZ] = {0, 0, 0};

void pl_jump(void){
		if(you.Velocity[Y] > 0){
			you.Velocity[Y] = fxm(you.floorNorm[Y], you.Velocity[Y]);
		} else {
			you.Velocity[Y] = 0;
		}
		you.onSurface = false;
		you.pos[Y] += (6553);
			you.Velocity[X] += fxm(196608, you.floorNorm[X]); 
			you.Velocity[Y] += fxm(196608, you.floorNorm[Y]);
			you.Velocity[Z] += fxm(196608, you.floorNorm[Z]);
		pcm_play(snd_bstep, PCM_SEMI, 7);
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
	const int HoofLowValue = 310000;

		if(you.onSurface == true){
	hoofSetBools[0] = (pl_model.pol[0]->pntbl[pl_model.pol[0]->pltbl[202].Vertices[0]][Y] > HoofLowValue) ? true : false;
	hoofSetBools[1] = (pl_model.pol[0]->pntbl[pl_model.pol[0]->pltbl[203].Vertices[0]][Y] > HoofLowValue) ? true : false;
	hoofSetBools[2] = (pl_model.pol[0]->pntbl[pl_model.pol[0]->pltbl[204].Vertices[0]][Y] > HoofLowValue) ? true : false;
	hoofSetBools[3] = (pl_model.pol[0]->pntbl[pl_model.pol[0]->pltbl[205].Vertices[0]][Y] > HoofLowValue) ? true : false;
		}

	hoofSetBools[4] = (you.onSurface);
		
		runSnd = (hoofSetBools[0] != oldHoofSetBools[0]) ? 1 : runSnd;
		runSnd = (hoofSetBools[1] != oldHoofSetBools[1]) ? 1 : runSnd;
		runSnd = (hoofSetBools[2] != oldHoofSetBools[2]) ? 1 : runSnd;
		runSnd = (hoofSetBools[3] != oldHoofSetBools[3]) ? 1 : runSnd;
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
 
static const int airFriction = 65400;
 
void	player_phys_affect(void)
{
	
	static short oldRot = 0;
	//Derive three angles from two inputs.
	you.viewRot[X] += you.rotState[Y];
	you.viewRot[Y] -= you.rotState[X];
	
	//Make the Velocity unit vector from the current player's rotation.
	you.ControlUV[X] = -slSin((you.rot[Y]));
	you.ControlUV[Y] = slSin(you.rot[X]);
	you.ControlUV[Z] = -slCos((you.rot[Y]));
	
	//Smart Camera Setup
	VECTOR uview = {-slSin(you.viewRot[Y]), 0, slCos(you.viewRot[Y])};
	int proportion = (fxm((you.DirUV[X] - uview[X]),(you.DirUV[X] - uview[X])) + fxm((you.DirUV[Z] - uview[Z]),(you.DirUV[Z] - uview[Z])))>>7;
	short angDif = (slAtan(you.DirUV[X], you.DirUV[Z]) - slAtan(uview[X], uview[Z]));
	//Will pivot camera towards direction of motion
	if( /*(you.setSlide == true || you.onSurface != true) &&*/ (JO_ABS(you.Velocity[X]) > 1024 || JO_ABS(you.Velocity[Z]) > 1024) &&  JO_ABS(angDif) > 1024 &&
	is_key_up(DIGI_DOWN))
	{
		you.viewRot[Y] += (angDif > 0) ? proportion : -proportion; //Determines if we want to rotate view clockwise or counterclockwise
	}
	//
	
	//F = m * a : This comment means nothing. This math isn't here nor there.
	//A = F / M :
	
		//There's always air .. unless we go into space, but whatevs, bruh
		you.Velocity[X] = fxm(you.Velocity[X], (airFriction));
		you.Velocity[Y] = fxm(you.Velocity[Y], (airFriction));
		you.Velocity[Z] = fxm(you.Velocity[Z], (airFriction));

							if(you.onSurface == true){
		//Skiing Decisions
		if(you.setSlide == true){
			you.surfFriction = 65000;
		}
								
		//Friction decisions
		if(JO_ABS(you.Velocity[X]) > (6553)){
			you.Velocity[X] = fxm(you.Velocity[X], (you.surfFriction));
			}
		if(JO_ABS(you.Velocity[X]) <= (6553) &&
		is_key_up(DIGI_UP) &&
		is_key_up(DIGI_DOWN) &&
		is_key_up(DIGI_LEFT) &&
		is_key_up(DIGI_RIGHT)) you.Velocity[X] = 0;

		if(JO_ABS(you.Velocity[Y]) > (6553)){you.Velocity[Y] = fxm(you.Velocity[Y], (you.surfFriction));}
		if(JO_ABS(you.Velocity[Y]) <= (6553) &&
		is_key_up(DIGI_UP) &&
		is_key_up(DIGI_DOWN) &&
		is_key_up(DIGI_LEFT) &&
		is_key_up(DIGI_RIGHT)) you.Velocity[Y] = 0;

		if(JO_ABS(you.Velocity[Z]) > (6553)){
			you.Velocity[Z] = fxm(you.Velocity[Z], (you.surfFriction));
			}
		if(JO_ABS(you.Velocity[Z]) <= (6553) &&
		is_key_up(DIGI_UP) &&
		is_key_up(DIGI_DOWN) &&
		is_key_up(DIGI_LEFT) &&
		is_key_up(DIGI_RIGHT)) you.Velocity[Z] = 0;

		}
							
		//Velocity add by input decisions
		//Acclimate speed on each axis to your rotation on each axis defined by two-axis input
		if(you.setSlide != true && you.onSurface == true){
		you.Velocity[X] += fxm(-you.IPaccel, you.ControlUV[X]);
		you.Velocity[Y] += fxm(you.IPaccel, you.ControlUV[Y]);
		you.Velocity[Z] += fxm(-you.IPaccel, you.ControlUV[Z]);
		} else { //If sliding, or in air
		you.Velocity[X] += fxm(fxm(-you.IPaccel, you.ControlUV[X]), 3000);
		you.Velocity[Y] += fxm(fxm(you.IPaccel, you.ControlUV[Y]), 3000);
		you.Velocity[Z] += fxm(fxm(-you.IPaccel, you.ControlUV[Z]), 3000);
		}

		if(you.setJump == true){
			pl_jump();
			you.setJump = false;
		}
		if( you.okayStepSnd ) pl_step_snd();

	//Surface / gravity decisions
	static VECTOR gravAcc;
	static FIXED	surfLimitX;
	static FIXED	surfLimitZ;
	if(you.onSurface == true){
		///When on surface, I need to make sure Y velocity applied here by gravity does not increase in a way that opposes the surface normal.
		///Also, stiction. You shouldn't ALWAYS slide :)
		gravAcc[X] = fxm(fxm((GRAVITY), frmul), you.floorNorm[X]); //Transform gravity by the surface
		gravAcc[Y] = fxm(fxm((GRAVITY), frmul), you.floorNorm[Y]);
		gravAcc[Z] = fxm(fxm((GRAVITY), frmul), you.floorNorm[Z]);
		you.Velocity[X] += (JO_ABS(gravAcc[X]) >= 16384 || you.setSlide == true || you.sanics >= 65536) ? gravAcc[X] : 0;
		you.Velocity[Y] -= (JO_ABS(gravAcc[Y]) >= 16384 || you.setSlide == true || you.sanics >= 65536) ? gravAcc[Y] : 0;
		you.Velocity[Z] += (JO_ABS(gravAcc[Z]) >= 16384 || you.setSlide == true || you.sanics >= 65536) ? gravAcc[Z] : 0;
		
		//The below treats some oblique angles as wall-ish
		//Problem: The heightmap surfaces are actually triangles, so it's kind of easy to manipulate the X and Z difference
		//to be minimal, and climb up just about any surface. [climb the seam between the triangles]
		surfLimitX = fxm(you.ControlUV[X], you.floorNorm[X]);
		surfLimitZ = fxm(you.ControlUV[Z], you.floorNorm[Z]);
		
		//slPrintFX(surfLimitX, slLocate(0, 9));
		//slPrintFX(surfLimitZ, slLocate(0, 10));
		
		if( surfLimitX > 26000 ){
			you.dirInp = false;
			you.Velocity[X] += (you.setSlide == true) ? 0 : fxm(32768, you.floorNorm[X]);
		}

		if( surfLimitZ > 26000 ){
			you.dirInp = false;
			you.Velocity[Z] += (you.setSlide == true) ? 0 : fxm(32768, you.floorNorm[Z]);
		}
		
	} else {
		you.Velocity[Y] -= fxm((GRAVITY), frmul);
	}

	//Wall Collision Decisions
	if(you.hitWall == true){
	you.pos[X] = you.prevPos[X] + (you.prevPos[X] + you.wallPos[X]);
	you.pos[Y] =	you.prevPos[Y] + (you.prevPos[Y] + you.wallPos[Y]);
	you.pos[Z] = you.prevPos[Z] + (you.prevPos[Z] + you.wallPos[Z]);
	
	you.Velocity[X] += (you.Velocity[X] > 0) ? -fxm(you.wallNorm[X], you.Velocity[X])<<1 : fxm(you.wallNorm[X], you.Velocity[X])<<1;
	you.Velocity[Y] += (you.Velocity[Y] > 0) ? -fxm(you.wallNorm[Y], you.Velocity[Y])<<1 : fxm(you.wallNorm[Y], you.Velocity[Y])<<1;
	you.Velocity[Z] += (you.Velocity[Z] > 0) ? -fxm(you.wallNorm[Z], you.Velocity[Z])<<1 : fxm(you.wallNorm[Z], you.Velocity[Z])<<1;
	
	}
	//
	//Add your speed to your position (incremental / per-frame)
	you.pos[X] += fxm(you.Velocity[X], frmul);
	you.pos[Y] += fxm(you.Velocity[Y], frmul);
	you.pos[Z] += fxm(you.Velocity[Z], frmul);
	//Create a true direction vector, independent of control vector
	///I also want to make a one-dimensional "speed" metric which finds out how fast you're going in serms of sanics, important for collisions
	static VECTOR tempDif = {0, 0, 0};
	tempDif[X] = you.pos[X] - you.prevPos[X];
	tempDif[Y] = you.pos[Y] - you.prevPos[Y];
	tempDif[Z] = you.pos[Z] - you.prevPos[Z];
	normalize(tempDif, you.DirUV);
	you.sanics = slSquartFX(fxm(tempDif[X], tempDif[X]) + fxm(tempDif[Y], tempDif[Y]) + fxm(tempDif[Z], tempDif[Z]));
	
	unsigned char windVol = ((you.sanics>>17) < 7) ? ((you.sanics>>17)+1) : 7;
	pcm_play(snd_wind, PCM_FWD_LOOP, windVol); //Sound that plays louder the faster you go. Only initiates at all once you are past 3 in sanics.
	
	//slPrintFX(you.sanics, slLocate(0, 8));
		
	//Movement and rotation speed maximum and minimums
	if(you.strafeState >= PLR_STRF_SPD) you.strafeState = PLR_STRF_SPD;
	if(you.strafeState <= -PLR_STRF_SPD) you.strafeState = -PLR_STRF_SPD;
	//
	if(you.IPaccel >= PLR_FWD_SPD) you.IPaccel = PLR_FWD_SPD;
	if(you.IPaccel <= -PLR_FWD_SPD) you.IPaccel = -PLR_FWD_SPD;
	//
	if(JO_ABS(you.rotState[X]) < 90) you.rotState[X] = 0;
	if(JO_ABS(you.rotState[Y]) < 90) you.rotState[Y] = 0;
	//De-rating
	if( is_key_up(DIGI_A) && you.rotState[X] < 0) you.rotState[X] += fxm(frmul, fxm(JO_ABS(you.rotState[X]), 16384));//A
	if( is_key_up(DIGI_B) && you.rotState[Y] < 0) you.rotState[Y] += fxm(frmul, fxm(JO_ABS(you.rotState[Y]), 16384));//S
	if( is_key_up(DIGI_C) && you.rotState[X] > 0) you.rotState[X] -= fxm(frmul, fxm(JO_ABS(you.rotState[X]), 16384));//D
	if( is_key_up(DIGI_Y) && you.rotState[Y] > 0) you.rotState[Y] -= fxm(frmul, fxm(JO_ABS(you.rotState[Y]), 16384));//W
	
	if(you.IPaccel > 0 && you.dirInp != true) you.IPaccel = fxm(spdfactr*40,you.IPaccel);
	if(you.IPaccel < 0 && you.dirInp != true) you.IPaccel += fxm(300, frmul);

		if(you.onSurface != true){
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
	you.moment[X] = fxm(you.mass, you.Velocity[X]);
	you.moment[Y] = fxm(you.mass, you.Velocity[Y]);
	you.moment[Z] = fxm(you.mass, you.Velocity[Z]);
	//
	if(you.hitWall != true){
		oldRot = you.rot[Y];
	you.prevPos[X] = you.pos[X];
	you.prevPos[Y] = you.pos[Y];
	you.prevPos[Z] = you.pos[Z];
	}
	
	//I wanna work on momentum.
	make2AxisBox(you.pos[X], you.pos[Y], you.pos[Z], you.renderRot[X], (you.renderRot[Y]), you.renderRot[Z], (2<<16), (5<<16), (5<<16), &pl_RBB);

		if(you.setSlide) //Rotational logic changes based on what movement state you are in.
		{				//This is *after* the rotation is set to the matrix so that when it is drawn (by slave SH2), it is appropriate.
				you.renderRot[X] = you.rot[X];
				you.renderRot[Y] = -you.viewRot[Y];
				you.renderRot[Z] = you.rot[Z];
		} else {
				you.renderRot[X] = you.rot[X];
				you.renderRot[Y] = you.rot[Y];
				you.renderRot[Z] = you.rot[Z];
		}

	pl_RBB.boxID = 0;
	pl_RBB.isBoxPop = true;	
}

void	collide_with_heightmap(_boundBox * sbox)
{

static int prevCellCenterPos[2] = {0, 0};
static int cellCenterPos[2] = {0, 0};
static bool firstSurfHit = false;

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

prevCellCenterPos[0] = cellCenterPos[0];
prevCellCenterPos[1] = cellCenterPos[1];

cellCenterPos[0] = (sbox->pos[X] > 0) ? ((fxm((INV_CELL_SIZE), sbox->pos[X])>>16)) + 1 : ((fxm((INV_CELL_SIZE), sbox->pos[X])>>16)); // - 1?
cellCenterPos[1] = (sbox->pos[Z] > 0) ? ((fxm((INV_CELL_SIZE), sbox->pos[Z])>>16)) + 1 : ((fxm((INV_CELL_SIZE), sbox->pos[Z])>>16)); // ? +1 : - 1

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

if(nyToTri2 >= 8192 && ny_Dist1 >= ny_Dist2 && you.hitSurface == false){
	you.hitMap = true;
	line_hit_plane_here(realCFs.yp1, realCFs.yp0, nyNearTriCF2, nyTriNorm2, alwaysLow, lowPoint);
	sort_angle_to_domain(nyTriNorm2, alwaysLow, you.renderRot);

	you.floorPos[X] = ((lowPoint[X]) - (sbox->Yneg[X]));
	you.floorPos[Y] = ((lowPoint[Y]) - (sbox->Yneg[Y]));
	you.floorPos[Z] = ((lowPoint[Z]) - (sbox->Yneg[Z]));
	you.shadowPos[X] = lowPoint[X];
	you.shadowPos[Y] = lowPoint[Y];
	you.shadowPos[Z] = lowPoint[Z];
	you.floorNorm[X] = nyTriNorm2[X];
	you.floorNorm[Y] = nyTriNorm2[Y];
	you.floorNorm[Z] = nyTriNorm2[Z];
} else if(nyToTri1 >= 8192 && ny_Dist1 < ny_Dist2 && you.hitSurface == false){
	you.hitMap = true;
	line_hit_plane_here(realCFs.yp1, realCFs.yp0, nyNearTriCF1, nyTriNorm1, alwaysLow, lowPoint);
	sort_angle_to_domain(nyTriNorm1, alwaysLow, you.renderRot);

	you.floorPos[X] = ((lowPoint[X]) - (sbox->Yneg[X]));
	you.floorPos[Y] = ((lowPoint[Y]) - (sbox->Yneg[Y]));
	you.floorPos[Z] = ((lowPoint[Z]) - (sbox->Yneg[Z]));
	you.shadowPos[X] = lowPoint[X];
	you.shadowPos[Y] = lowPoint[Y];
	you.shadowPos[Z] = lowPoint[Z];
	you.floorNorm[X] = nyTriNorm1[X];
	you.floorNorm[Y] = nyTriNorm1[Y];
	you.floorNorm[Z] = nyTriNorm1[Z];
	
} else {
	//Find Floor on Map, if not above an object
			if(you.aboveObject != true){
	len_A = unfix_length(you.pos, nyNearTriCF1);
	len_B = unfix_length(you.pos, nyNearTriCF2);
		if(len_A < len_B)
		{
		line_hit_plane_here(you.pos, below_player, nyNearTriCF1, nyTriNorm1, alwaysLow, you.shadowPos);
		} else {
		line_hit_plane_here(you.pos, below_player, nyNearTriCF2, nyTriNorm2, alwaysLow, you.shadowPos);
		}
			}
	//
	you.hitMap = false;
}

	if(you.hitSurface == true || you.hitMap == true){
		
		you.onSurface = true;
		you.surfFriction = (45875);
		you.pos[X] = (you.floorPos[X]);
		you.pos[Y] = (you.floorPos[Y]);
		you.pos[Z] = (you.floorPos[Z]);
		
		you.Velocity[Y] -= fxm(you.Velocity[Y], you.floorNorm[Y]); //Don't get Y velocity against the floor

		//if(you.sanics > 2<<16 && firstSurfHit == false){

 			//d - 2 * dot(d, n) * n
			//Surface Deflection at oblique angles is desired.
				//Somewhat Functional. Disabled for now.

		//VECTOR nOfn = {0, 0, 0};
		//cross_fixed(you.DirUV, you.floorNorm, nOfn);
		// jo_printf(0, 10, "(%i)", nOfn[X]/182);
		// jo_printf(0, 11, "(%i)", nOfn[Y]/182);
		// jo_printf(0, 12, "(%i)", nOfn[Z]/182);
		
		// slPrintFX(you.Velocity[X], slLocate(0, 10));
		// slPrintFX(you.Velocity[Y], slLocate(0, 11));
		// slPrintFX(you.Velocity[Z], slLocate(0, 12));

			// FIXED deflectionFactor = fxdot(you.Velocity, you.floorNorm);

			// you.Velocity[X] = (you.Velocity[X] - fxm(fxm(fxm(2<<16, deflectionFactor), you.floorNorm[X]), (you.floorNorm[X])));
			// you.Velocity[Y] = (you.Velocity[Y] - fxm(fxm(fxm(2<<16, deflectionFactor), you.floorNorm[Y]), (you.floorNorm[Y])));		
			// you.Velocity[Z] = (you.Velocity[Z] - fxm(fxm(fxm(2<<16, deflectionFactor), you.floorNorm[Z]), (you.floorNorm[Z])));
			

			// firstSurfHit = true;
			// you.hitSurface = false;
			// you.hitMap = false;
		//}
		
	} else {
		you.onSurface = false;
		firstSurfHit = false;
	}

	// jo_printf(0, 11, "(%i)", cellCenterPos[0]);
	// jo_printf(5, 11, "(%i)", cellCenterPos[1]);	

}


