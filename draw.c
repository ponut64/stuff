
#include <sl_def.h>
#include <SGL.H>
#include <SEGA_GFS.H>

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
#include "pcmstm.h"
#include "menu.h"
#include "particle.h"
#include "gamespeed.h"
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

//////////////////////////////////////////////////////////////////////////////
//Animation Structs
//////////////////////////////////////////////////////////////////////////////
animationControl idle;
animationControl idleB;
animationControl stop;
animationControl fall;
animationControl slideIdle;
animationControl slideLln;
animationControl slideRln;
animationControl airIdle;
animationControl airLeft;
animationControl airRight;
animationControl jump;
animationControl hop;
animationControl walk;
animationControl run;
animationControl dbound;
animationControl climbIdle;
animationControl climbing;
 
animationControl flap;

spriteAnimation qmark;
spriteAnimation arrow;
spriteAnimation check;
spriteAnimation goal;

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
		if(viewInfoTxt == 1)
		{
	slPrintFX(you.pos[X], slLocate(9, 1));
	slPrintFX(you.pos[Y], slLocate(19, 1));
	slPrintFX(you.pos[Z], slLocate(29, 1));

	//nbg_sprintf(18, 0, "(File System Status)");
	nbg_sprintf(27, 2, "Pts :%x:", you.points);
	//nbg_sprintf(10, 2, "throttle:(%i)", you.IPaccel);
	//slPrintFX(you.sanics, slLocate(26, 3));
//		if(delta_time>>6 > 35)
//		{
	nbg_sprintf(8, 25, "TRPLY:                ");
	nbg_sprintf(8, 25, "TRPLY:%i", transPolys[0]);
	nbg_sprintf(8, 26, "SNTPL:                ");
	nbg_sprintf(8, 26, "SNTPL:%i", ssh2SentPolys[0] + msh2SentPolys[0]);
	nbg_sprintf(8, 27, "VERTS:                ");
	nbg_sprintf(8, 27, "VERTS:%i", transVerts[0]);
//		}
	nbg_sprintf(37, 26, "cX(%i)", you.cellPos[X]);
	nbg_sprintf(37, 27, "cY(%i)", you.cellPos[Y]);    
	
	nbg_sprintf(1, 4, "Fuel:(%i), Rate:(%i)", you.power, you.IPaccel);
	
	nbg_sprintf(16, 2, "Stream:(%i)", file_system_status_reporting);
	nbg_sprintf(17, 3, "Sanics:(%i)", you.sanics);
	
	
		} else if(viewInfoTxt == 2)
		{
			
			slPrintFX(you.pos[X], slLocate(9, 1));
			slPrintFX(you.pos[Y], slLocate(19, 1));
			slPrintFX(you.pos[Z], slLocate(29, 1));
			
			slPrintFX(you.velocity[X], slLocate(9, 2));
			slPrintFX(you.velocity[Y], slLocate(19, 2));
			slPrintFX(you.velocity[Z], slLocate(29, 2));
			
			slPrintFX(you.dV[X], slLocate(9, 3));
			slPrintFX(you.dV[Y], slLocate(19, 3));
			slPrintFX(you.dV[Z], slLocate(29, 3));
			
			slPrintFX(you.ControlUV[X], slLocate(9, 4));
			slPrintFX(you.ControlUV[Y], slLocate(19, 4));
			slPrintFX(you.ControlUV[Z], slLocate(29, 4));
			
			nbg_sprintf(2, 5, "rX:(%i)", you.rot[X]);
			nbg_sprintf(15, 5, "rY:(%i)", you.rot[Y]);
			nbg_sprintf(29, 5, "rZ:(%i)", you.rot[Z]);
			
			nbg_sprintf(2, 6, "rRX:(%i)", you.renderRot[X]);
			nbg_sprintf(15, 6,"rRY:(%i)", you.renderRot[Y]);
			nbg_sprintf(29, 6, "rRZ:(%i)", you.renderRot[Z]);
			
			nbg_sprintf(2, 7, "hitWall:(%i)", you.hitWall);
			nbg_sprintf(2, 8, "hitSurf:(%i)", you.hitSurface);
			nbg_sprintf(20, 8,"hitItm:(%i)", pl_RBB.collisionID);
			
		}
}

void	player_animation(void)
{
	
			//Animation Chains
					static int airTimer = 0;
			if(pl_model.file_done == true)
			{
				if(you.hitSurface == true)
				{
				airTimer = 0;
					if(you.setSlide != true && you.climbing != true && airTimer == 0)
					{
						if(you.velocity[X] == 0 && you.velocity[Y] == 0 && you.velocity[Z] == 0)
						{
							meshAnimProcessing(&idle, &pl_model,  false);
						} else if( (you.velocity[X] != 0 || you.velocity[Z] != 0) && you.dirInp)
						{
						if(you.IPaccel <= 0){
								meshAnimProcessing(&stop, &pl_model,  false);
							}
						if(you.sanics < 2<<16 && you.IPaccel > 0){
								meshAnimProcessing(&walk, &pl_model,  true);
							}
						if(you.sanics < 3<<16 && you.sanics > 2<<16){
								meshAnimProcessing(&run, &pl_model,  true);
							}
						if(you.sanics >= 3<<16){
								meshAnimProcessing(&dbound, &pl_model,  true);
							}
						} else if((you.velocity[X] != 0 || you.velocity[Z] != 0) && !you.dirInp)
						{
							meshAnimProcessing(&stop, &pl_model,  false);
						} else {
							meshAnimProcessing(&idle, &pl_model,  false);
						}	
						//IF NOT SLIDE ENDIF
					} else if(you.setSlide == true && you.climbing != true){
						if(you.rot2[Y] > (30 * 182) && you.rot2[Y]  < (150 * 182) && you.dirInp)
						{
							meshAnimProcessing(&slideRln, &pl_model,  false);
						} else if(you.rot2[Y] < (330 * 182) && you.rot2[Y] > (210 * 182) && you.dirInp)
						{
							meshAnimProcessing(&slideLln, &pl_model,  false);
						} else {
							meshAnimProcessing(&slideIdle, &pl_model,  false);
						}
						//IF SLIDE ENDIF
					} else if(you.climbing == true)
					{
						if(you.sanics == 0)
						{
							meshAnimProcessing(&climbIdle, &pl_model,  false);
						} else {
							meshAnimProcessing(&climbing, &pl_model,  false);
						}
						//IF CLIMB ENDIF
					}
					//IF SURFACE ENDIF	
				} else {
						airTimer++;
						if(airTimer < 8 && airTimer != 0 && you.velocity[Y] != 0)
						{
							if(!you.setJet){
								meshAnimProcessing(&jump, &pl_model,  false);
							} else {
								meshAnimProcessing(&hop, &pl_model,  false);
							}
						} else if(you.rot2[Y] > (30 * 182) && you.rot2[Y]  < (150 * 182) && you.dirInp)
						{
							meshAnimProcessing(&airRight, &pl_model,  false);
						} else if(you.rot2[Y] < (330 * 182) && you.rot2[Y] > (210 * 182) && you.dirInp)
						{
							meshAnimProcessing(&airLeft, &pl_model,  false);
						} else {
							meshAnimProcessing(&airIdle, &pl_model,  false);
						}
				}//IF AIR ENDIF
			} //IF MODEL LOADED ENDIF
	
			//This plays a wing flap animation when you start 'jetting'.
			//Due to the configuration of the animation, the wings stop after one flap.
			//This code manually resets the flap animation when you're done jetting, so they will flap again.
			if(you.setJet)
			{
				meshAnimProcessing(&flap, &wings,  false);
			} else {
				flap.curFrm = flap.startFrm * 8;
				flap.curKeyFrm = flap.startFrm;
				flap.reset_enable = 'Y';
			}
	
}

void	player_draw(void)
{
	//slPushMatrix();
	{
		//Note that "sl_RBB" is used as a slave-only copy of pl_RBB to be manipulated.
		sl_RBB = pl_RBB;
		sl_RBB.pos[X] = 0;//-sl_RBB.pos[X]; //Negate, because coordinate madness
		sl_RBB.pos[Y] = 0;//-sl_RBB.pos[Y]; //Negate, because coordinate madness
		sl_RBB.pos[Z] = 0;//-sl_RBB.pos[Z]; //Negate, because coordinate madness
	
		pl_model.prematrix = (FIXED*)&sl_RBB;
		wings.prematrix = (FIXED*)&sl_RBB;
		
		msh2DrawModel(&pl_model, perspective_root);

	}
	//slPopMatrix();
	
	
	if(you.setJet)
	{
		//slPushMatrix();
		msh2DrawModel(&wings, perspective_root);
		//slPopMatrix();
	}

		// pl_RBB.pos[X] = -pl_RBB.pos[X]; //Safety un-negation
		// pl_RBB.pos[Y] = -pl_RBB.pos[Y]; //Safety un-negation
		// pl_RBB.pos[Z] = -pl_RBB.pos[Z]; //Safety un-negation

}

void	obj_draw_queue(void)
{
		
	for( unsigned char i = 0; i < MAX_PHYS_PROXY; i++)
	{
		//This conditions covers if somehow a non-renderable object (like level data) got put into the render stack.
		//Assuming the rest of the game code made sense up to this point. Else the game's gonna crash here.
		if(RBBs[i].status[0] != 'R') continue;
		if(RBBs[i].status[4] == 'S') apply_box_scale(&RBBs[i]);
		//unsigned short objType = (dWorldObjects[activeObjects[i]].type.ext_dat & ETYPE);
		
	slPushMatrix();
	
		entities[objDRAW[i]].prematrix = (FIXED *)&RBBs[i];
	
		if(entities[objDRAW[i]].type == MODEL_TYPE_BUILDING)
		{ 
			plane_rendering_with_subdivision(&entities[objDRAW[i]]);
		} else {
			
			ssh2DrawModel(&entities[objDRAW[i]]);
		}
	slPopMatrix();
	
	}
	

	for(int s = 0; s < MAX_SPRITES; s++)
	{
		if(sprWorkList[s].lifetime >= 0)
		{
			sprWorkList[s].lifetime -= delta_time;
			if(sprWorkList[s].type == SPRITE_TYPE_BILLBOARD || sprWorkList[s].type == SPRITE_TYPE_UNSCALED_BILLBOARD)
			{
				ssh2BillboardScaledSprite(&sprWorkList[s]);
			} else if(sprWorkList[s].type == SPRITE_TYPE_3DLINE || sprWorkList[s].type == SPRITE_TYPE_UNSORTED_LINE)
			{
				ssh2Line(&sprWorkList[s]);
			} else if(sprWorkList[s].type == SPRITE_TYPE_NORMAL || sprWorkList[s].type == SPRITE_MESH_STROBE
			|| sprWorkList[s].type == SPRITE_FLASH_STROBE || sprWorkList[s].type == SPRITE_BLINK_STROBE)
			{
				ssh2NormalSprite(&sprWorkList[s]);
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
		//Note that "sl_RBB" is used as a slave-only copy of pl_RBB to be manipulated.
	sl_RBB = pl_RBB;
	sl_RBB.pos[X] = -you.shadowPos[X];
	sl_RBB.pos[Y] = -(you.shadowPos[Y] - 8192);
	sl_RBB.pos[Z] = -you.shadowPos[Z]; 
	
	shadow.prematrix = (FIXED*)&sl_RBB;

	ssh2DrawModel(&shadow);
	
	slPopMatrix();
		}
}

 //Uses SGL to prepare the matrix for the map, so it doesn't mess up the matrix stack when the map draws
 //Be aware the location of this function is important:
 //The player's position/rotation cannot change from between when it runs and when the matrix is used.
void	prep_map_mtx(void)
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

	//volatile int times[8];

void	object_draw(void)
{
	*timeComm = 0;
	//times[2] = get_time_in_frame();
	computeLight();
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
	//player_draw();
	shadow_draw();
	//times[3] = get_time_in_frame();
	obj_draw_queue();
	//times[4] = get_time_in_frame();
	}
	slPopMatrix();
	
	//if(viewInfoTxt == 1)
	//{
	//slPrintFX(times[2], slLocate(7, 12));
	//slPrintFX(times[3], slLocate(7, 13));
	//slPrintFX(times[4], slLocate(7, 14));
	//nbg_sprintf(2, 11, "SSH2:");
	//nbg_sprintf(2, 12, "S.Beg:");
	//nbg_sprintf(2, 13, "Plyr:");
	//nbg_sprintf(2, 14, "Ents:");
	//}
	
}

void	map_draw_prep(void)
{
	hmap_cluster();
	
	//Loads the DSP pepperbox
	you.prevCellPos[X] = you.cellPos[X];
	you.prevCellPos[Y] = you.cellPos[Y];
	you.prevDispPos[X] = you.dispPos[X];
	you.prevDispPos[Y] = you.dispPos[Y];
	//View Distance Extention -- Makes turning view cause performance issue, beware?
	you.cellPos[X] = (fxm((INV_CELL_SIZE), you.pos[X])>>16);
	you.cellPos[Y] = (fxm((INV_CELL_SIZE), you.pos[Z])>>16);
	int center_distance = (CELL_SIZE_INT * ((LCL_MAP_PLY>>1)-1))<<16;
	int sineY = fxm(slSin(-you.viewRot[Y]), center_distance);
	int sineX = fxm(slCos(-you.viewRot[Y]), center_distance);
	you.dispPos[X] = (fxm((INV_CELL_SIZE), you.pos[X] +  sineY)>>16);
	you.dispPos[Y] = (fxm((INV_CELL_SIZE), you.pos[Z] +  sineX)>>16);
	//
	hmap_matrix_pos[X] = (you.pos[X] + you.velocity[X]) - ((you.dispPos[X] * CELL_SIZE_INT)<<16);
	hmap_matrix_pos[Z] = (you.pos[Z] + you.velocity[Z]) - ((you.dispPos[Y] * CELL_SIZE_INT)<<16);
	
	hmap_actual_pos[X] = hmap_matrix_pos[X] - (you.pos[X] + you.velocity[X]);
	hmap_actual_pos[Y] = 0;
	hmap_actual_pos[Z] = hmap_matrix_pos[Z] - (you.pos[Z] + you.velocity[Z]);
	
	load_hmap_prog();
	run_hmap_prog();

}

void	map_draw(void)
{	

	while(dsp_noti_addr[0] == 0){}; //"DSP Wait"
	update_hmap(hmap_mtx);

}

void	master_draw(void)
{
	static int time_at_start;
	static int time_of_master_draw;
	static int time_of_object_management;
	static int time_at_end;
	static int time_at_ssh2_end;
	static int interim_time;
	static int extra_time;

	
	time_at_start = get_time_in_frame();
	
	if(!you.inMenu)
	{
	slSlaveFunc(object_draw, 0); //Get SSH2 busy with its drawing stack ASAP
	slCashPurge();

	interim_time = get_time_in_frame();
	
	player_animation();
	player_draw();

	//
	if(!(you.distanceToMapFloor > 768<<16))
	{
	map_draw();
	}
	map_draw_prep();
	//
	time_of_master_draw = get_time_in_frame() - interim_time;
	interim_time = get_time_in_frame();
	//
	operate_particles();
	hud_menu();
	slSlaveFunc(sort_master_polys, 0);
	//
	
		//No Touch Order -- Affects animations/mechanics
		controls();
		player_phys_affect();
		player_collision_test_loop();
		collide_with_heightmap(&pl_RBB);
		//
	extra_time = get_time_in_frame() - interim_time;
	
	interim_time = get_time_in_frame();
	flush_boxes(0);
	light_control_loop(); //lit
	object_control_loop(you.dispPos);
	time_of_object_management = get_time_in_frame() - interim_time;
	
		//
	} else if(you.inMenu)
	{
		start_menu();
		//
		slSlaveFunc(sort_master_polys, 0);
		//
	}
	clean_sprite_animations();
	start_texture_animation(&check, &entities[18]);
	start_texture_animation(&qmark, &entities[36]);
	start_texture_animation(&arrow, &entities[37]);
	start_texture_animation(&arrow, &entities[38]);
	start_texture_animation(&arrow, &entities[39]);
	start_texture_animation(&goal, &entities[40]);
	operate_texture_animations();

	time_at_end = get_time_in_frame();
	//Debug stuff. Important!
	
	if(viewInfoTxt == 1)
	{
	slPrintFX(time_at_start, slLocate(7, 7));
	slPrintFX(time_of_master_draw, slLocate(7, 8));
	slPrintFX(time_of_object_management, slLocate(7, 9));
	slPrintFX(extra_time, slLocate(7, 10));
	slPrintFX(time_at_end, slLocate(7, 11));
	nbg_sprintf(2, 6, "MSH2:");
	nbg_sprintf(2, 7, "Strt:");
	nbg_sprintf(2, 8, "Map:");
	nbg_sprintf(2, 9, "Objs:");
	nbg_sprintf(2, 10, "Ext:");
	nbg_sprintf(2, 11, "End:");
	
	while(!*timeComm){
		if(get_time_in_frame() >= (50<<16)) break;
	};
	time_at_ssh2_end = get_time_in_frame();
	slPrintFX(time_at_ssh2_end, slLocate(7, 12));
	nbg_sprintf(2, 12, "SSH2:");
	}
}

