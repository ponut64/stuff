
#include <sl_def.h>
#include <SGL.H>
#include <SEGA_GFS.H>

#include "def.h"
#include "mymath.h"
#include "input.h"
#include "control.h"
#include "vdp2.h"
#include "render.h"
#include "tga.h"
#include "physobjet.h"
#include "collision.h"
#include "pcmstm.h"
#include "menu.h"
#include "particle.h"
#include "gamespeed.h"
//
#include "dspm.h"

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
//Root matrix for the screen
MATRIX perspective_root;
//Root matrix for the world
_boundBox world_box;
// Forward Vector. For convenience.
int scrn_z_fwd[3] = {0, 0, 0};

// Mental note: Try Rotation-16 framebuffer and alternate screen coordinate from 0,0 and 1,1 every vblank
// Should make mesh transparencies work better

//Note: global light is in order of BLUE, GREEN, RED. [BGR]
int globalColorOffset;
int glblLightApply = true;
int drawModeSwitch = DRAW_MASTER;
unsigned char * backScrn = (unsigned char *)VDP2_RAMBASE;

//////////////////////////////////////////////////////////////////////////////
//Animation Structs
//////////////////////////////////////////////////////////////////////////////
backgroundAnimation shorty_idle;
backgroundAnimation shorty_fire;

spriteAnimation qmark;

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
	nbg_sprintf(27, 2, "Scr :%x:", you.score);
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
			
			slPrintFX(you.wpos[X], slLocate(9, 1));
			slPrintFX(you.wpos[Y], slLocate(19, 1));
			slPrintFX(you.wpos[Z], slLocate(29, 1));
			
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
			nbg_sprintf(14, 7, "hitSurf:(%i)", you.hitSurface);
			nbg_sprintf(24, 7,"hitItm:(%i)", pl_RBB.collisionID);
			
		}
}

void	player_animation(void)
{
	//Empty
}

void	player_draw(int draw_mode)
{
	
	//Drawing first-person model....

	//Note that "sl_RBB" is used as a safe copy of pl_RBB to be manipulated.
	bound_box_starter.modified_box = &sl_RBB;
	bound_box_starter.x_location = 0;
	bound_box_starter.y_location = 0;
	bound_box_starter.z_location = 0;
	
	bound_box_starter.x_rotation = -you.viewRot[X];
	bound_box_starter.y_rotation = -you.viewRot[Y];
	bound_box_starter.z_rotation = 0;
	
	bound_box_starter.x_radius = entities[0].radius[X]<<16;
	bound_box_starter.y_radius = entities[0].radius[Y]<<16;
	bound_box_starter.z_radius = entities[0].radius[Z]<<16;

	makeBoundBox(&bound_box_starter, EULER_OPTION_XZY);
	
	//step-in code for handling nbg1 weapon sprite
	
	
	//so if test idle frame is 330x75, and its the starting keyframe in the top left, we need to:
	//1. set window from top left (352-330),(224-75) to bottom right of (352,224)
	//2. then, scroll the screen right (352-330) pixels, and down (75-224) pixels	

	//okay, let's display the animation "shorty_idle"
	
	backgroundAnimation * curAnim = &shorty_fire;
	
	static int keyTimer = 0;
	static int curKey = 0;
	
	keyTimer+= delta_time;
	
	if(keyTimer > curAnim->lifetimes[curKey])
	{
		curKey += 1;
		keyTimer = 0;
	}
	if(curKey >= curAnim->length)
	{
		curKey = 0;
	}
	
	bg_key * keyfrm = curAnim->keyframes[curKey];
	
	int finWindow[2] = {keyfrm->wpos[X]-you.viewmodel_offset[X], keyfrm->wpos[Y]-you.viewmodel_offset[Y]};
	
	int finPos[2] = {keyfrm->spos[X]-finWindow[X], keyfrm->spos[Y]-finWindow[Y]};
	
	slScrWindow0(finWindow[X], finWindow[Y], finWindow[X]+keyfrm->size[X], finWindow[Y]+keyfrm->size[Y]);
	slScrPosNbg1((finPos[X])<<16, (finPos[Y])<<16);
	
	// meshAnimProcessing(&reload, &entities[1], false);

	// entities[1].prematrix = (FIXED*)&sl_RBB;
	// entities[1].z_plane = 1;
	// if(draw_mode == DRAW_MASTER)
	// {
		// msh2DrawModel(&entities[1], perspective_root);
	// } else if(draw_mode == DRAW_SLAVE)
	// {
		// slPushMatrix();
		// ssh2DrawModel(&entities[1]);
		// slPopMatrix();
	// }
}

void	shadow_draw(int draw_mode)
{
	
	//Make shadow match player rotation. I mean, it's not a perfect solution, but it mostly works.
	//Note that "sl_RBB" is used as a slave-only copy of pl_RBB to be manipulated.
	sl_RBB = pl_RBB;
	sl_RBB.pos[X] = you.pos[X] - you.shadowPos[X];
	sl_RBB.pos[Y] = you.pos[Y] - (you.shadowPos[Y] - 8192);
	sl_RBB.pos[Z] = you.pos[Z] - you.shadowPos[Z]; 
	
	shadow.prematrix = (FIXED*)&sl_RBB;

	if(draw_mode == DRAW_MASTER)
	{
		msh2DrawModel(&shadow, perspective_root);
	} else if(draw_mode == DRAW_SLAVE)
	{
		slPushMatrix();
		ssh2DrawModel(&shadow);
		slPopMatrix();
	}
	
}


void	obj_draw_queue(void)
{
	
	for( unsigned char i = 0; i < MAX_PHYS_PROXY; i++)
	{
		//This conditions covers if somehow a non-renderable object (like level data) got put into the render stack.
		//Assuming the rest of the game code made sense up to this point. Else the game's gonna crash here.
		if(DBBs[i].status[0] != 'R') continue;
		if(DBBs[i].status[4] == 'S') apply_box_scale(&DBBs[i]);
		//unsigned short objType = (dWorldObjects[activeObjects[i]].type.ext_dat & ETYPE);
		DBBs[i].status[5] = 'r';
	slPushMatrix();
	
		entities[objDRAW[i]].prematrix = (FIXED *)&DBBs[i];

		switch(entities[objDRAW[i]].type)
		{
			case(MODEL_TYPE_BUILDING):
			plane_rendering_with_subdivision(&entities[objDRAW[i]]);
			break;
			case(MODEL_TYPE_SECTORED):
			break;
			default:
			ssh2DrawModel(&entities[objDRAW[i]]);
			break;
		}

	slPopMatrix();
	
	}
	
	//Warning: Reverse-count loops must not start equal to the max count; this results in memory corruption.
	//Ex: Array[63] has valid members 0-62.
	for(int s = (MAX_SPRITES-1); s >= 0; s--)
	{
		if(sprWorkList[s].lifetime >= 0 || sprWorkList[s].type.info.drawOnce)
		{
			sprWorkList[s].lifetime -= delta_time;
			switch(sprWorkList[s].type.info.drawMode)
			{
				case(SPRITE_TYPE_BILLBOARD):
				case(SPRITE_TYPE_UNSCALED_BILLBOARD):
				ssh2BillboardScaledSprite(&sprWorkList[s]);
				break;
				case(SPRITE_TYPE_3DLINE):
				ssh2Line(&sprWorkList[s]);
				break;
				case(SPRITE_TYPE_NORMAL):
				case(SPRITE_MESH_STROBE):
				case(SPRITE_FLASH_STROBE):
				case(SPRITE_BLINK_STROBE):
				default:
				ssh2NormalSprite(&sprWorkList[s]);
				break;
			}
			sprWorkList[s].type.info.alive = (sprWorkList[s].type.info.drawOnce) ? 0 : 1;
			sprWorkList[s].type.info.drawOnce = 0;
		} else {
			//Mark expired sprites as unused.
			sprWorkList[s].type.info.alive = 0; 
			sprWorkList[s].type.info.drawOnce = 0;
		}
	}
	
	for(int s = 0; s < nearSectorCt; s++)
	{
		draw_sector(visibleSectors[s], *sectorToDrawFrom, (MATRIX*)&world_box);
	}
	
	//Random:
	//This is the correct time to set all sectors as "not ready for drawing".
	//Both the MSH2 and SSH2 are done with all sectors at this point.
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		sectors[i].ready_this_frame = 0;
	}
	
}

void	scene_draw(void)
{
	*timeComm = 0;
	/////////////////////////////////////////////
	// Important first-step
	// Sometimes the master will finish preparing the drawing list while the slave is working on them.
	// That would normally break the work flow and make some entities disappear.
	// To prevent that, we capture the to-draw lists at the start of the Slave's workflow to a list that the Master does not edit.
	// (i need to do the same with the sprite work list)
	/////////////////////////////////////////////
	for(int i = 0; i < objUP; i++)
	{
		objDRAW[i] = objPREP[i];
	}
	for(int i = 0; i < MAX_PHYS_PROXY; i++)
	{
		DBBs[i] = RBBs[i];
	}

	computeLight();
	slPushMatrix();
	{	
	//(for third-person camera only)
	//slTranslate((VIEW_OFFSET_X), (VIEW_OFFSET_Y), (VIEW_OFFSET_Z) );
	
	//Take care about the order of the matrix transformations!
	slRotX((you.viewRot[X]));
	slRotY((you.viewRot[Y]));

	//Adjustments are made on the Y to account for the height of the player (in first-person camera)
	slTranslate(you.pos[X], you.pos[Y] + ((PLAYER_Y_SIZE>>1) - (2<<16)), you.pos[Z]);


	obj_draw_queue();

	}
	slPopMatrix();
	
	///////////////////////////////////////////////////////////////////////////////////////////
	// Important Wait-Synch-Loop
	// If master SH2 is not done drawing, don't continue on to sorting its work.
	// If it however IS done drawing, beak the nop loop and sort its output in the Zbuffer.
	do
	{
		__asm__("nop;");
	} while(!(*masterIsDoneDrawing));
	
	sort_master_polys();
}

void	background_draw(void)
{
	
	//
	// What do I want to do now with the background layer?
	// It is 512x512. I want the screen to move to appear as if it is matching rotation on the Y axis.
	// I also want Y-0 rotation to display the center of the background image.
	// First: How much can the screen span in the X axis for this to be possible?
	// So with 512 pixels as the width, one degree is 1.4222 pixels. We could abstract that so 128 units is 1 pixel.
	// (128 being 1 pixel is achieved by 65536 (360 degrees) / 512 pixels)
	
	// The X rotation - Y screen axis is completely different however.
	// The Y axis of the image will NOT rollover. It would rollover in space, but we aren't in space.
	// It also is not a spherical projection or even a logical projection; I can't invert the image when it rolls over.
	// There are ways to manipulate the image data itself such that rollovers are masked, but I will instead limit the rotation.
	// The view rotation here will be limited to +/- 90 degrees.
	// 16384 is the integer value of 90 degrees, but we allow a range of +/- 90, so our total degree range is 32768.
	// That means the unit value of 1 pixel is (32768 units / 512 pixels) = 64 units per pixel
	// But we end up using 85 units per pixel. For... reasons, I guess. It's what worked. No clear idea why.
	int vrx = you.viewRot[X];
	
	int screen_y_to_x = 192 - ((you.viewRot[Y] / 128));
	int screen_x_to_y = 192 - ((vrx / 85));
	// Screen Rollover Control
	// When the Y rotation has caused the screen to rollover (e.g. 192 + y_to_x > 512), the image is offset.
	// So we need to get a position that corresponds to a position under 512.
	if(JO_ABS(screen_y_to_x) > 512)
	{
		screen_y_to_x = screen_y_to_x % 512;
	}

	slScrPosNbg0(screen_y_to_x<<16, screen_x_to_y<<16);
	
	
}

void	sector_vertex_remittance(void)
{
	static int viewport_pos[3];
	viewport_pos[X] = you.pos[X];
	viewport_pos[Y] = you.pos[Y] + ((PLAYER_Y_SIZE>>1) - (2<<16));
	viewport_pos[Z] = you.pos[Z] - (1<<16);
	
	/////////////////////////////////////////////////////////
	// World matrix processing 
	/////////////////////////////////////////////////////////
	bound_box_starter.modified_box = &world_box;

	bound_box_starter.x_location = viewport_pos[X];
	bound_box_starter.y_location = viewport_pos[Y];
	bound_box_starter.z_location = viewport_pos[Z];
	
	bound_box_starter.x_rotation = -you.viewRot[X];
	bound_box_starter.y_rotation = you.viewRot[Y] + (32768);
	bound_box_starter.z_rotation = 0;
	
	bound_box_starter.x_radius = 1;
	bound_box_starter.y_radius = 1;
	bound_box_starter.z_radius = 1;


	/////////////////////////////////////////////////////////////////////
	// world pos?
	/////////////////////////////////////////////////////////////////////
	makeBoundBox(&bound_box_starter, EULER_OPTION_XYZ);
	//we copy off perspective_root to have the world matrix before translations are applied
	copy_matrix((int*)&perspective_root, (int*)&world_box); 
	perspective_root[3][X] = 0;
	perspective_root[3][Y] = 0;
	perspective_root[3][Z] = 0;
	fxMatrixApplyTranslation((int*)&world_box);

	scrn_z_fwd[X] = world_box.UVZ[X];
	scrn_z_fwd[Y] = world_box.UVZ[Y];
	scrn_z_fwd[Z] = world_box.UVZ[Z];
	
	//Because the world geometry is drawn independent of the object system, we have to make a valid prematrix for it.
	static int world_prematrix[12] = {1<<16, 0, 0, 0, 1<<16, 0, 0, 0, 1<<16, 0,0,0};
	world_prematrix[9] = levelPos[X];
	world_prematrix[10] = levelPos[Y];
	world_prematrix[11] = levelPos[Z];
	
	/////////////////////////////////////////////////////////
	// Pre-loop portal processing
	////////////////////////////////////////////////////////
	_sector * sct = &sectors[*sectorToDrawFrom];
	sct->ent->prematrix = &world_prematrix[0];
	for(int s = 0; s < nearSectorCt; s++)
	{
		visibleSectors[s] = sct->pvs[s];
		collect_portals_from_sector(visibleSectors[s], (MATRIX*)&world_box, &viewport_pos[0]);
	}
	
	//////////////////////////////////////////////
	// Process should create:
	// sectorIsAdjacent as a boolean flag which states which sectors are and which are not adjacent.
	// Every frame, it is purged such that all sectors are not adjacent.
	// Then, the correct sectors from the PVS are written in as "1", for true, adjacent.
	// This information is used to short-hand operations when determining sector visibility based on the portal information.
	for(unsigned int s = 0; s < MAX_SECTORS; s++)
	{
		sectorIsAdjacent[s] = 0;
		sectorIsVisible[s] = 0;
	}
	for(unsigned int p = 0; p < sct->nbAdjacent; p++)
	{
		//+1 from the PVS list to bypass the sector self-identifier
		sectorIsAdjacent[sct->pvs[p+1]] = 1;
	}
	for(unsigned int p = 0; p < sct->nbVisible; p++)
	{
		sectorIsVisible[sct->pvs[p]] = 1;
	}
	/////////////////////////////////////////////////////////
	// Sector Vertex Transform Loop
	////////////////////////////////////////////////////////
	dsp_noti_addr[0] = 1;
	for(int s = 0; s < nearSectorCt; s++)
	{
		transform_verts_for_sector(visibleSectors[s], (MATRIX*)&world_box);
	}
	
	//vertex_t * screenspace_verts = (vertex_t*)sectors[1].scrnspace_tvtbl;
	//for(int i = 0; i < 4; i++)
	//{
	//	nbg_sprintf(2, 8+i, "clip(%x)", screenspace_verts[i].clipFlag);	
	//}
	
	

	//nbg_sprintf(2, 6, "prts:(%i)", *current_portal_count);
	//nbg_sprintf(2, 6, "drwSector:(%i)", *sectorToDrawFrom);
	//nbg_sprintf(2, 7, "curSector:(%i)", you.curSector);
	//nbg_sprintf(2, 8, "prvSector:(%i)", you.prevSector);
	
	// nbg_sprintf(16, 8, "ctrX:(%i)",  sectors[you.curSector].center_pos[X]>>16);
	// nbg_sprintf(16, 9, "ctrY:(%i)",  sectors[you.curSector].center_pos[Y]>>16);
	// nbg_sprintf(16, 10, "ctrZ:(%i)", sectors[you.curSector].center_pos[Z]>>16);
	
	// nbg_sprintf(16, 12, "radX:(%i)", sectors[you.curSector].radius[X]>>16);
	// nbg_sprintf(16, 13, "radY:(%i)", sectors[you.curSector].radius[Y]>>16);
	// nbg_sprintf(16, 14, "radZ:(%i)", sectors[you.curSector].radius[Z]>>16);
	
	int alltilect = 0;
	for(int i = 0; i < sectors[you.curSector].nbPolygon; i++)
	{
		alltilect += sectors[you.curSector].nbTile[i];
	}
	
	//nbg_sprintf(2, 10, "sctTile:(%i)", alltilect);

}

void	master_draw(void)
{
	static int time_of_sectors;
	static int time_of_master_draw;
	static int time_of_object_management;
	static int time_at_end;
	static int time_at_ssh2_end;
	static int interim_time;
	static int extra_time;

	time_of_sectors = get_time_in_frame();

	background_draw();
	
	interim_time = get_time_in_frame();
	//
	player_animation();
	player_draw(DRAW_MASTER);
	shadow_draw(DRAW_MASTER);
	//
	time_of_master_draw = get_time_in_frame() - interim_time;
	interim_time = get_time_in_frame();
	//
	operate_particles();
	hud_menu();

	// nbg_sprintf(2, 6, "prts:(%i)", *current_portal_count);
	// nbg_sprintf(2, 6, "drwSector:(%i)", *sectorToDrawFrom);
	// nbg_sprintf(2, 7, "curSector:(%i)", you.curSector);
	// nbg_sprintf(2, 8, "surface:(%i)", you.hitSurface);

	//
		//No Touch Order -- Affects animations/mechanics
		controls();
		player_phys_affect();
		player_collision_test_loop();
	//
	extra_time = get_time_in_frame() - interim_time;
	
	interim_time = get_time_in_frame();
	flush_boxes(0);
	light_control_loop(); //lit
	object_control_loop();
	time_of_object_management = get_time_in_frame() - interim_time;
	*masterIsDoneDrawing = 1;
		//
	
	//Oh, right, this...
	//I was intending on integrating some sort of process-based loop on these.
	//Oh well...
	clean_sprite_animations();
	operate_texture_animations();

	time_at_end = get_time_in_frame();
	//Debug stuff. Important!
	
	
	static int rolling_avg_msh2 = 0;
	static int rolling_avg_ssh2 = 0;
	static int avg_samples = 0;
	
	if(viewInfoTxt == 1)
	{
		nbg_sprintf_decimal(34, 7, time_of_sectors);
		nbg_sprintf_decimal(34, 8, time_of_master_draw);
		nbg_sprintf_decimal(34, 9, time_of_object_management);
		nbg_sprintf_decimal(34, 10, extra_time);
		nbg_sprintf_decimal(34, 11, time_at_end);
		nbg_sprintf(29, 6, "MSH2:");
		nbg_sprintf(29, 7, "Sctr:");
		nbg_sprintf(29, 8, "Map:");
		nbg_sprintf(29, 9, "Objs:");
		nbg_sprintf(29, 10, "Ext:");
		nbg_sprintf(29, 11, "End:");
		
		while(!*timeComm){
			if(get_time_in_frame() >= (50<<16)) break;
		};
		time_at_ssh2_end = get_time_in_frame();
		nbg_sprintf_decimal(34, 12, time_at_ssh2_end);
		nbg_sprintf(29, 12, "SSH2:");
	
		avg_samples++;
		int inversion = fxdiv(1<<16, avg_samples<<16);
		rolling_avg_msh2 -= fxm(rolling_avg_msh2, inversion);
		rolling_avg_msh2 += fxm(time_at_end, inversion);
		rolling_avg_ssh2 -= fxm(rolling_avg_ssh2, inversion);
		rolling_avg_ssh2 += fxm(time_at_ssh2_end, inversion);
		
		nbg_sprintf_decimal(34, 14, rolling_avg_msh2);
		nbg_sprintf_decimal(34, 15, rolling_avg_ssh2);
		nbg_sprintf(29, 14, "CPU0:");
		nbg_sprintf(29, 15, "CPU1:");

	} else {
		rolling_avg_msh2 = time_at_end;
		rolling_avg_ssh2 = time_at_ssh2_end;
		avg_samples = 0;
	}
}

