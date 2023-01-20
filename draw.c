
#include <jo/jo.h>


#include "def.h"
#include "mymath.h"
#include "input.h"
#include "control.h"
#include "render.h"
#include "tga.h"
#include "physobjet.h"
#include "hmap.h"
#include "player_phy.h"
#include "object_col.h"
#include "collision.h"
//
#include "dspm.h"
//
#include "height.h"

#include "draw.h"

FIXED sun_light[3] = {5000, -20000, 0};
//Player Model
entity_t pl_model;
//Player's Shadow
entity_t shadow;
//Player Wings
entity_t wings;
//Heightmap Matrix
MATRIX hmap_mtx;
//Root matrix after perspective transform
MATRIX perspective_root;
// ???
MATRIX unit;
//Billboard sprite work list (world-space positions)
//
// Mental note: Try Rotation-16 framebuffer and alternate screen coordinate from 0,0 and 1,1 every vblank
// Should make mesh transparencies work better

FIXED hmap_matrix_pos[XYZ] = {0, 0, 0};
FIXED hmap_actual_pos[XYZ] = {0, 0, 0};

//Note: global light is in order of BLUE, GREEN, RED. [BGR]
int globalColorOffset;
int glblLightApply = true;
unsigned char * backScrn = (unsigned char *)VDP2_RAMBASE;

void	computeLight(void)
{

	if(glblLightApply == true)
	{
	
	color_offset_vdp1_palette(globalColorOffset, &glblLightApply);
	//Next, set the sun light.
	active_lights[0].pop = 1;
	active_lights[0].ambient_light = &sun_light[0];
	active_lights[0].min_bright = 10000;
	active_lights[0].bright = 0;
	//////////////////////////////////////////////////////////////////////////////////////

	glblLightApply = false;
	}
}

void	set_camera(void)
{
 POINT viewPnt = {0, 0, 1<<16};	

	slLookAt( viewPnt , zPt , 0 );
}

void	master_draw_stats(void)
{
	slPrintFX(you.pos[X], slLocate(9, 1));
	slPrintFX(you.pos[Y], slLocate(19, 1));
	slPrintFX(you.pos[Z], slLocate(29, 1));

	//jo_printf(18, 0, "(File System Status)");
	jo_printf(27, 2, "Pts :%x:", you.points);
	//jo_printf(10, 2, "throttle:(%i)", you.IPaccel);
	//slPrintFX(you.sanics, slLocate(26, 3));
//		if(delta_time>>6 > 35)
//		{
	jo_printf(8, 25, "TRPLY:                ");
	jo_printf(8, 25, "TRPLY:%i", transPolys[0]);
	jo_printf(8, 26, "SNTPL:                ");
	jo_printf(8, 26, "SNTPL:%i", ssh2SentPolys[0] + msh2SentPolys[0]);
	jo_printf(8, 27, "VERTS:                ");
	jo_printf(8, 27, "VERTS:%i", transVerts[0]);
//		}
	jo_printf(37, 26, "cX(%i)", you.cellPos[X]);
	jo_printf(37, 27, "cY(%i)", you.cellPos[Y]);    
}

void	player_draw(void)
{
	slPushMatrix();
	{

		pl_RBB.pos[X] = -pl_RBB.pos[X]; //Negate, because coordinate madness
		pl_RBB.pos[Y] = -pl_RBB.pos[Y]; //Negate, because coordinate madness
		pl_RBB.pos[Z] = -pl_RBB.pos[Z]; //Negate, because coordinate madness
	
		pl_model.prematrix = (FIXED*)&pl_RBB;
		wings.prematrix = (FIXED*)&pl_RBB;
		
//Animation Chains
					static int airTimer = 0;
			if(pl_model.file_done == true){
				if(you.hitSurface == true){
				airTimer = 0;
					if(you.setSlide != true && airTimer == 0){
						if(you.velocity[X] == 0 && you.velocity[Y] == 0 && you.velocity[Z] == 0){
							ssh2DrawAnimation(&idle, &pl_model,  false);
						} else if( (you.velocity[X] != 0 || you.velocity[Z] != 0) && you.dirInp){
						if(you.IPaccel < 0){
							ssh2DrawAnimation(&stop, &pl_model,  false);
							}
						if(you.sanics < 2<<16 && you.IPaccel > 0){
							ssh2DrawAnimation(&walk, &pl_model,  true);
							}
						if(you.sanics < 3<<16 && you.sanics > 2<<16){
							ssh2DrawAnimation(&run, &pl_model,  true);
							}
						if(you.sanics >= 3<<16){
							ssh2DrawAnimation(&dbound, &pl_model,  true);
							}
						} else if((you.velocity[X] != 0 || you.velocity[Z] != 0) && !you.dirInp){
						ssh2DrawAnimation(&stop, &pl_model,  false);
						} else {
						ssh2DrawAnimation(&idle, &pl_model,  false);
						}	
					} else {//IF NOT SLIDE ENDIF
						if(is_key_pressed(DIGI_RIGHT)){
						ssh2DrawAnimation(&slideRln, &pl_model,  false);
						} else if(is_key_pressed(DIGI_LEFT)){
						ssh2DrawAnimation(&slideLln, &pl_model,  false);
						} else if(is_key_pressed(DIGI_UP)){
						ssh2DrawAnimation(&slideIdle, &pl_model,  false);
						} else if(is_key_pressed(DIGI_DOWN)){
						ssh2DrawAnimation(&slideIdle, &pl_model,  false);
						} else {
						ssh2DrawAnimation(&slideIdle, &pl_model,  false);
						}
						}//IF SLIDE ENDIF
						
					
				} else {//IF SURFACE ENDIF
						airTimer++;
						if(airTimer < 8 && airTimer != 0 && you.velocity[Y] != 0){
							if(!you.setJet){
							ssh2DrawAnimation(&jump, &pl_model,  false);
							} else {
							ssh2DrawAnimation(&hop, &pl_model,  false);
							}
						} else if(is_key_pressed(DIGI_RIGHT)){
						ssh2DrawAnimation(&airRight, &pl_model,  false);
						} else if(is_key_pressed(DIGI_LEFT)){
						ssh2DrawAnimation(&airLeft, &pl_model,  false);
						} else if(is_key_pressed(DIGI_DOWN)){
						ssh2DrawAnimation(&airIdle, &pl_model,  false);
						} else {
						ssh2DrawAnimation(&airIdle, &pl_model,  false);
						}
				}//IF AIR ENDIF
			} //IF MODEL LOADED ENDIF
			
			
	}
	slPopMatrix();
	
	slPushMatrix();
			//This plays a wing flap animation when you start 'jetting'.
			//Due to the configuration of the animation, the wings stop after one flap.
			//This code manually resets the flap animation when you're done jetting, so they will flap again.
			if(you.setJet)
			{
				ssh2DrawAnimation(&flap, &wings,  false);
			} else {
				flap.currentFrm = flap.startFrm * 8;
				flap.currentKeyFrm = flap.startFrm;
				flap.reset_enable = 'Y';
			}
	slPopMatrix();

		pl_RBB.pos[X] = -pl_RBB.pos[X]; //Safety un-negation
		pl_RBB.pos[Y] = -pl_RBB.pos[Y]; //Safety un-negation
		pl_RBB.pos[Z] = -pl_RBB.pos[Z]; //Safety un-negation

}

void	obj_draw_queue(void)
{
		
	for( unsigned char i = 0; i < MAX_PHYS_PROXY; i++)
	{
		if(RBBs[i].status[0] != 'R') continue;
		
		unsigned short objType = (dWorldObjects[activeObjects[i]].type.ext_dat & OTYPE);
		
	slPushMatrix();
	
		entities[objDRAW[i]].prematrix = (FIXED *)&RBBs[i];
	
			if( objType != ITEM && objType != LDATA && objType != BUILD )
			{ //Check if entity is NOT ITEM or LDATA
		ssh2DrawModel(&entities[objDRAW[i]]);
			} else if( objType == ITEM )
			{ //if entity IS ITEM
				if( !(dWorldObjects[activeObjects[i]].type.ext_dat & 8) ) //Check if root entity still exists
				{
					ssh2DrawModel(&entities[objDRAW[i]]);
				}
			} else if( objType == BUILD)
			{
		plane_rendering_with_subdivision(&entities[objDRAW[i]]);
			}
	slPopMatrix();
	
	}
	

	for(int s = 0; s < 64; s++)
	{
		if(sprWorkList[s].lifetime >= 0)
		{
			sprWorkList[s].lifetime -= delta_time;
			if(sprWorkList[s].type == 'B')
			{
				ssh2BillboardScaledSprite(&sprWorkList[s]);
			} else if(sprWorkList[s].type == 'L')
			{
				ssh2Line(&sprWorkList[s]);
			}
		} else {
			//Mark expired sprites as unused.
			sprWorkList[s].type = 'N'; 
		}
	}
}

void	shadow_draw(void)
{
 	//static char first_run;
		if(shadow.file_done == true)
		{
			
	slPushMatrix();

		//Make shadow match player rotation. I mean, it's not a perfect solution, but it mostly works.

	pl_RBB.pos[X] = -you.shadowPos[X];
	pl_RBB.pos[Y] = -(you.shadowPos[Y] - 8192);
	pl_RBB.pos[Z] = -you.shadowPos[Z]; 
	
	shadow.prematrix = (FIXED*)&pl_RBB;

	ssh2DrawModel(&shadow);
		
		//Just casually undo that
		
	pl_RBB.pos[X] = you.pos[X];
	pl_RBB.pos[Y] = you.pos[Y];
	pl_RBB.pos[Z] = you.pos[Z];
	
	slPopMatrix();
		}
	// Reset the shadow position controller
	you.aboveObject = false;
}

void	object_draw(void)
{
	slPushMatrix();
	{	
	slTranslate((VIEW_OFFSET_X), (VIEW_OFFSET_Y), (VIEW_OFFSET_Z) );
	
	//Take care about the order of the matrix transformations!
	slRotX((you.viewRot[X]));
	slRotY((you.viewRot[Y]));
	slGetMatrix(perspective_root);
	//////////////////////////////////////////////////////////////
	// "viewpoint" is the point from which the perspective will originate (contains view translation/rotation).
	//////////////////////////////////////////////////////////////
	you.viewpoint[X] = fxm(perspective_root[X][X], perspective_root[3][X]) +
						fxm(perspective_root[Y][X], perspective_root[3][Y]) +
						fxm(perspective_root[Z][X], perspective_root[3][Z]);
						
	you.viewpoint[Y] = fxm(perspective_root[X][Y], perspective_root[3][X]) +
						fxm(perspective_root[Y][Y], perspective_root[3][Y]) +
						fxm(perspective_root[Z][Y], perspective_root[3][Z]);
						
	you.viewpoint[Z] = fxm(perspective_root[X][Z], perspective_root[3][X]) +
						fxm(perspective_root[Y][Z], perspective_root[3][Y]) +
						fxm(perspective_root[Z][Z], perspective_root[3][Z]);
		//
	slTranslate(you.pos[X], you.pos[Y], you.pos[Z]);
	player_draw();
	shadow_draw();
		
	obj_draw_queue();
	}
	slPopMatrix();
	
}

void	map_draw(void){	

	while(dsp_noti_addr[0] == 0){}; //"DSP Wait"
update_hmap(hmap_mtx);

}

void	prep_map_mtx(void) //Uses SGL to prepare the matrix for the map, so it doesn't mess up the matrix stack when the map draws
{
	slInitMatrix();
	set_camera();
	slPushMatrix();
	{
	slTranslate((VIEW_OFFSET_X), (VIEW_OFFSET_Y), (VIEW_OFFSET_Z) );
	slRotX((you.viewRot[X]));
	slRotY((you.viewRot[Y]));
	slTranslate(hmap_matrix_pos[X], you.pos[Y], hmap_matrix_pos[Z]);
	slGetMatrix(hmap_mtx);
	}
	slPopMatrix();
}

void	master_draw(void)
{
	prep_map_mtx();
	computeLight();
	slSlaveFunc(object_draw, 0); //Get SSH2 busy with its drawing stack ASAP
	slCashPurge();

	hmap_cluster();
	map_draw();
	
	you.prevCellPos[X] = you.cellPos[X];
	you.prevCellPos[Y] = you.cellPos[Y];
	you.prevDispPos[X] = you.dispPos[X];
	you.prevDispPos[Y] = you.dispPos[Y];
	//View Distance Extention -- Makes turning view cause performance issue, beware?
	you.cellPos[X] = (fxm((INV_CELL_SIZE), you.pos[X])>>16);
	you.cellPos[Y] = (fxm((INV_CELL_SIZE), you.pos[Z])>>16);
	int sineY = fxm(slSin(-you.viewRot[Y]), 275<<16);
	int sineX = fxm(slCos(-you.viewRot[Y]), 275<<16);
	you.dispPos[X] = (fxm((INV_CELL_SIZE), you.pos[X] +  sineY)>>16);
	you.dispPos[Y] = (fxm((INV_CELL_SIZE), you.pos[Z] +  sineX)>>16);
	//
	hmap_matrix_pos[X] = (you.pos[X] + you.velocity[X]) - ((you.dispPos[X] * CELL_SIZE_INT)<<16);
	hmap_matrix_pos[Z] = (you.pos[Z] + you.velocity[Z]) - ((you.dispPos[Y] * CELL_SIZE_INT)<<16);
	
	hmap_actual_pos[X] = hmap_matrix_pos[X] - (you.pos[X] + you.velocity[X]);
	hmap_actual_pos[Y] = 0;
	hmap_actual_pos[Z] = hmap_matrix_pos[Z] - (you.pos[Z] + you.velocity[Z]);
	
	run_dsp();
	
	//No Touch Order -- Affects animations/mechanics
		mypad();
	player_phys_affect();
	player_collision_test_loop();
	collide_with_heightmap(&pl_RBB);
	light_control_loop(); //lit
	object_control_loop(you.dispPos);
	//
	
	slSlaveFunc(sort_master_polys, 0);	
}

