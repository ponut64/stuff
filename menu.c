//menu.c
//This file compiled separately.

#include <SL_DEF.H>
#include <SEGA_GFS.H>
#include <SGL.H>

#include "def.h"
#include "mymath.h"
#include "pcmsys.h"
#include "pcmstm.h"
#include "tga.h"
#include "input.h"
#include "control.h"
#include "render.h"
#include "physobjet.h"

#include "minimap.h"
#include "sound.h"
#include "collision.h"
#include "draw.h"

#include <string.h>

#include "menu.h"
extern Sint8 SynchConst; //SGL System Variable
int viewInfoTxt = 0;
int baseRingMenuTexno = 0;
int controlImgTexno = 0;
int slowImgTexno = 0;
int parImgTexno = 0;
int goldImgTexno = 0;
int menuLayer = 0;

_hudEvent hudEvents[HUD_EVENT_TYPES];

void	debug_menu_layer(__basic_menu * mnu)
{
	
	static short dPreview = 0;
	
	mnu->topLeft[X] = 120;
	mnu->topLeft[Y] = 20;
	mnu->scale[X] = 120;
	mnu->scale[Y] = 24;
	mnu->option_grid[X] = 1;
	mnu->option_grid[Y] = 7;
	mnu->num_option = 7;
	mnu->backColor = 6 + (64 * 2);
	mnu->optionColor = 5;
	static char * option_list[] = {"<- Back", "Reset Player", "Toggle Frmt", "Next Object", "Prev Object", "Tgl Info Txt", "Tex Viewer"};
	mnu->option_text = option_list;
	
	if(is_key_release(DIGI_A))
	{
		pcm_play(snd_button, PCM_SEMI, 6);
		switch(mnu->selection)
		{
			case(0):
			menuLayer = HUD_LAYER_START;
			break;
			case(1):
			reset_player();
			you.inMenu = false;
			break;
			case(2):
			SynchConst = (SynchConst == 1) ? 2 : (SynchConst == 2) ? 3 : 1;
			break;
			case(3):
			dPreview++;
			break;
			case(4):
			dPreview--;
			break;
			case(5):
			viewInfoTxt = (viewInfoTxt == 1) ? 2 : (viewInfoTxt == 2) ? 0 : 1;
			nbg_clear_text();
			break; 
			case(6):
			menuLayer = HUD_LAYER_TEXVIEWER;
			mnu->selection = 0;
			default:
			break;
		}
	}
	dPreview = (dPreview < 0) ? 0 : (dPreview >= MAX_WOBJS) ? MAX_WOBJS : dPreview;
	_declaredObject * rdobj = &dWorldObjects[dPreview];
	if(viewInfoTxt == 1)
	{
	spr_sprintf(8,		(-12) + (60),	"Height:(%i)", rdobj->pos[Y]>>16);
	spr_sprintf(8,		0  + (60),		"Object:(%i)", dPreview);
	spr_sprintf(8,		12 + (60),		"Sctr:(%i)", rdobj->curSector);
	spr_sprintf(72,		12 + (60),		"PrSctr:(%i)", rdobj->prevSector);
	spr_sprintf(8,		24 + (60),		"eID:(%i)", rdobj->type.entity_ID);
	spr_sprintf(8,		36 + (60),		"Rad:x(%i)", rdobj->type.radius[X]);
	spr_sprintf(112,	36 + (60),		",y(%i)", rdobj->type.radius[Y]);
	spr_sprintf(184,	36 + (60),		",z(%i)", rdobj->type.radius[Z]);
	spr_sprintf(8,		48 + (60),		"Rot:x(%i)", rdobj->rot[X]);
	spr_sprintf(112,	48 + (60),		",y(%i)", rdobj->rot[Y]);
	spr_sprintf(184,	48 + (60),		",z(%i)", rdobj->rot[Z]);
	spr_sprintf(8,		60 + (60),		"edat:(%x)", rdobj->type.ext_dat);
	spr_sprintf(120,	60 + (60),		"MDAT:(%x)", rdobj->more_data);
	spr_sprintf(8,		72 + (60),		"luma:(%i)", rdobj->type.light_bright);
	spr_sprintf(8,		90 + (60),		"yofs:(%i)", rdobj->type.light_y_offset);
	spr_sprintf(8,		102 + (60),		"dist:(%i)", rdobj->dist);
	} else if(viewInfoTxt == 2)
	{
		if(dPreview >= MAX_PHYS_PROXY) dPreview = MAX_PHYS_PROXY-1;
	_boundBox * rdbox = &RBBs[dPreview];
	spr_sprintf(8,		0  + (60),		"RBB:(%i)", dPreview);
	spr_sprintf(8,		12 + (60),		"Player Hit:(%i)", pl_RBB.collisionID);
	spr_sprintf(8,		24 + (60),		"st0:(%c)", rdbox->status[0]);
	spr_sprintf(8,		36 + (60),		"st1:(%c)", rdbox->status[1]);
	spr_sprintf(8,		48 + (60),		"st2:(%c)", rdbox->status[2]);
	spr_sprintf(8,		60 + (60),		"pX:(%i)", rdbox->pos[X]);
	spr_sprintf(8,		72 + (60),		"pY:(%i)", rdbox->pos[Y]);
	spr_sprintf(8,		84 + (60),		"pZ:(%i)", rdbox->pos[Z]);
	spr_sprintf(8,		96 + (60),		"Obj:(%i)", rdbox->boxID);
	spr_sprintf(100,	96 + (60),		"Hit:(%i)", rdbox->collisionID);
	}
	
}

void	control_menu_layer(__basic_menu * mnu)
{
	mnu->topLeft[X] = 25;
	mnu->topLeft[Y] = 20;
	mnu->scale[X] = 100;
	mnu->scale[Y] = 20;
	mnu->option_grid[X] = 1;
	mnu->option_grid[Y] = 1;
	mnu->num_option = 1;
	mnu->backColor = 6 + (64 * 2);
	mnu->optionColor = 5;
	static char * option_list[] = {"<- Back"};
	mnu->option_text = option_list;
	
	if(is_key_release(DIGI_A))
	{
		pcm_play(snd_button, PCM_SEMI, 6);
		switch(mnu->selection)
		{
			case(0):
			menuLayer = HUD_LAYER_START;
			break;
			default:
			break;
		}
	}
	
	//draw_normal_sprite(96, 60, controlImgTexno, 1);
	int point0[3] = {50 - TV_HALF_WIDTH, 48 - TV_HALF_HEIGHT, 0};
	int point1[3] = {300 - TV_HALF_WIDTH,48 - TV_HALF_HEIGHT, 0};
	int point2[3] = {300 - TV_HALF_WIDTH,173 - TV_HALF_HEIGHT, 0};
	int point3[3] = {50 - TV_HALF_WIDTH, 173 - TV_HALF_HEIGHT, 0};
	msh2SetCommand(point0, point1, point2, point3,
	VDP1_BASE_CMDCTRL, (VDP1_BASE_PMODE),
	pcoTexDefs[controlImgTexno].SRCA, 1<<6, pcoTexDefs[controlImgTexno].SIZE, 0, 2<<16);
}

void	items_menu_later(__basic_menu * mnu)
{
	mnu->topLeft[X] = 35;
	mnu->topLeft[Y] = 40;
	mnu->scale[X] = 140;
	mnu->scale[Y] = 24;
	mnu->option_grid[X] = 2;
	mnu->option_grid[Y] = 2;
	mnu->num_option = 2;
	mnu->backColor = 6 + (64 * 2);
	mnu->optionColor = 5;
	static char * option_list[] = {"Shorty Shotgun", "Lever Pistol"};
	mnu->option_text = option_list;
	
	if(is_key_release(DIGI_A))
	{
		pcm_play(snd_button, PCM_SEMI, 6);
		switch(mnu->selection)
		{
			case(0):
			you.inMenu = false;
			set_viewmodel_from_slot(0);
			break;
			case(1):
			you.inMenu = false;
			set_viewmodel_from_slot(1);
			break;
			default:
			break;
		}
	}
	
	if(is_key_release(DIGI_C))
	{
		pcm_play(snd_clack, PCM_SEMI, 6);
		menuLayer = HUD_LAYER_START;
		mnu->selection = 0;
	}
	
}

void	start_menu_layer(__basic_menu * mnu)
{
	mnu->topLeft[X] = 35;
	mnu->topLeft[Y] = 40;
	mnu->scale[X] = 140;
	mnu->scale[Y] = 24;
	mnu->option_grid[X] = 2;
	mnu->option_grid[Y] = 4;
	mnu->num_option = 5;
	mnu->backColor = 6 + (64 * 2);
	mnu->optionColor = 5;
	static char * option_list[] = {"Weapon Select", "Level Select", "Debug Menu", "Options Menu",
									"View Controls"};
	mnu->option_text = option_list;
	
	if(is_key_release(DIGI_A))
	{
		pcm_play(snd_button, PCM_SEMI, 6);
		switch(mnu->selection)
		{
			case(0):
			menuLayer = HUD_LAYER_ITEMS;
			mnu->selection = 0;
			break;
			case(1):
			menuLayer = HUD_LAYER_LEVEL;
			mnu->selection = 0;
			break;
			case(2):
			menuLayer = HUD_LAYER_DEBUG;
			mnu->selection = 0;
			break; 
			case(3):
			menuLayer = HUD_LAYER_OPTION_1;
			mnu->selection = 0;
			break;
			case(4):
			menuLayer = HUD_LAYER_CONTROLS;
			mnu->selection = 0;
			break;
			default:
			break;
		}
	}
	
}

void	levelselect_menu_layer(__basic_menu * mnu)
{
	
	mnu->topLeft[X] = 120;
	mnu->topLeft[Y] = 40;
	mnu->scale[X] = 120;
	mnu->scale[Y] = 24;
	mnu->option_grid[X] = 1;
	mnu->option_grid[Y] = 5;
	mnu->num_option = 5;
	mnu->backColor = 6 + (64 * 2);
	mnu->optionColor = 5;
	static char * option_list[] = {"Return to Game", "<- Back", "Next Level", "Prev Level", "Go to level!"};
	mnu->option_text = option_list;
	
	static int levelSelect = 0;
	
	if(is_key_release(DIGI_A))
	{
		pcm_play(snd_button, PCM_SEMI, 6);
		switch(mnu->selection)
		{
			case(0):
			you.inMenu = false;
			break;
			case(1):
			menuLayer = HUD_LAYER_START;
			break;
			case(2):
			levelSelect++;
			break;
			case(3):
			levelSelect--;
			break;
			case(4):
			p64MapRequest(levelSelect);
			you.inMenu = false;
			break;
			default:
			break;
		}
	}
	
	levelSelect = (levelSelect < 0) ? 0 : levelSelect;
	
	levelSelect = (levelSelect > NUM_LEVELS) ? NUM_LEVELS : levelSelect;
	
	//Some stuff about the level, but for now:
	//nbg_sprintf(2, 10, "Level:(%i)", levelSelect);
	spr_sprintf(16, 120, "Level:(%i)", levelSelect);
	
}

void	options_menu_layer(__basic_menu * mnu)
{
	
	mnu->topLeft[X] = 35;
	mnu->topLeft[Y] = 40;
	mnu->scale[X] = 140;
	mnu->scale[Y] = 24;
	mnu->option_grid[X] = 1;
	mnu->option_grid[Y] = 5;
	mnu->num_option = 5;
	mnu->backColor = 58 + (64 * 2);
	mnu->optionColor = 5;
	static char * option_list[] = {"<- Back", "Tgl Movement Cam", "Tgl Facing Cam",
									"Auto Cam Spd", "Next->"};
	mnu->option_text = option_list;
	
	if(is_key_release(DIGI_A))
	{
		pcm_play(snd_button, PCM_SEMI, 6);
		switch(mnu->selection)
		{
			case(0):
			menuLayer = HUD_LAYER_START;
			break;
			case(1):
			usrCntrlOption.movementCam = (usrCntrlOption.movementCam == 1) ? 0 : 1;
			break;
			case(2):
			usrCntrlOption.facingCam = (usrCntrlOption.facingCam == 1) ? 0 : 1;
			break;
			case(3):
			usrCntrlOption.followForce = 1<<16;
			break;
			case(4):
			menuLayer = HUD_LAYER_OPTION_2;
			mnu->selection = 0;
			break; 
			default:
			break;
		}
	}
	
	if(is_key_release(DIGI_Y) && mnu->selection == 3)
	{
		pcm_play(snd_click, PCM_SEMI, 6);
		usrCntrlOption.followForce += 1024;
	}

	if(is_key_release(DIGI_B) && mnu->selection == 3)
	{
		pcm_play(snd_click, PCM_SEMI, 6);
		usrCntrlOption.followForce -= 1024;
	}
	
	if(usrCntrlOption.followForce < 0) usrCntrlOption.followForce = 0;
	
	static int option_bg_tlp[XY];
	static int option_bg_brpt[XY];
	option_bg_tlp[X] = 180;
	option_bg_tlp[Y] = 40;
	option_bg_brpt[X] = 350;
	option_bg_brpt[Y] = 170;
	draw2dSquare(option_bg_tlp, option_bg_brpt, 26 + (64 * 2), 0, 5, 0);
	
	if(usrCntrlOption.movementCam)
	{
		spr_sprintf(200, 76, "On");
	} else {
		spr_sprintf(200, 76, "Off");
	}
	
	if(usrCntrlOption.facingCam)
	{
		spr_sprintf(200, 100, "On");
	} else {
		spr_sprintf(200, 100, "Off");
	}
	
	if(usrCntrlOption.followForce > 0)
	{
	spr_sprintf(200, 124, "%i", usrCntrlOption.followForce);
	} else {
	spr_sprintf(200, 124, "Auto Cam OFF");
	}
	
	spr_sprintf(180, 146, "Y inc, B dec, A reset");
	
}


void	options_menu_layer_2(__basic_menu * mnu)
{
	
	mnu->topLeft[X] = 35;
	mnu->topLeft[Y] = 40;
	mnu->scale[X] = 140;
	mnu->scale[Y] = 24;
	mnu->option_grid[X] = 1;
	mnu->option_grid[Y] = 5;
	mnu->num_option = 5;
	mnu->backColor = 42 + (64 * 2);
	mnu->optionColor = 5;
	static char * option_list[] = {"<- Back", "Camera Accel", "Cam Spd Cap",
									"Lockout Time", "<emp>"};
	mnu->option_text = option_list;
	
	if(is_key_release(DIGI_A))
	{
		pcm_play(snd_button, PCM_SEMI, 6);
		switch(mnu->selection)
		{
			case(0):
			menuLayer = HUD_LAYER_OPTION_1;
			break;
			case(1):
			usrCntrlOption.cameraAccel = 45;
			break;
			case(2):
			usrCntrlOption.cameraCap = 0;
			break;
			case(3):
			usrCntrlOption.lockoutTime = 1<<16;
			break;
			case(4):

			break; 
			default:
			break;
		}
	}
	
	if(is_key_down(DIGI_Y))
	{
		switch(mnu->selection)
		{
			case(1):
			pcm_play(snd_click, PCM_SEMI, 6);
			usrCntrlOption.cameraAccel += 2;
			break;
			case(2):
			pcm_play(snd_click, PCM_SEMI, 6);
			usrCntrlOption.cameraCap += 90;
			break;
			case(3):
			pcm_play(snd_click, PCM_SEMI, 6);
			usrCntrlOption.lockoutTime += 1024;
			break;
			default:
			break;
		}
	}

	if(is_key_down(DIGI_B))
	{
		switch(mnu->selection)
		{
			case(1):
			pcm_play(snd_click, PCM_SEMI, 6);
			usrCntrlOption.cameraAccel -= 2;
			break;
			case(2):
			pcm_play(snd_click, PCM_SEMI, 6);
			usrCntrlOption.cameraCap -= 90;
			break;
			case(3):
			pcm_play(snd_click, PCM_SEMI, 6);
			usrCntrlOption.lockoutTime -= 1024;
			break;
			default:
			break;
		}
	}
	
	if(usrCntrlOption.lockoutTime < 0) usrCntrlOption.lockoutTime = 0;
	if(usrCntrlOption.cameraAccel < 0) usrCntrlOption.cameraAccel = 0;
	if(usrCntrlOption.cameraCap < 0) usrCntrlOption.cameraCap = 0;
	
	static int option_bg_tlp[XY];
	static int option_bg_brpt[XY];
	option_bg_tlp[X] = 180;
	option_bg_tlp[Y] = 40;
	option_bg_brpt[X] = 350;
	option_bg_brpt[Y] = 170;
	draw2dSquare(option_bg_tlp, option_bg_brpt, 50 + (64 * 2), 0, 5, 0);
	

	spr_sprintf(200, 76, "%i", usrCntrlOption.cameraAccel);
	
	if(usrCntrlOption.cameraCap == 0)
	{
		spr_sprintf(200, 100, "Uncapped");
	} else {
		spr_sprintf(200, 100, "%i", usrCntrlOption.cameraCap);
	}
	
	spr_sprintf(200, 124, "%i", usrCntrlOption.lockoutTime);
	
	spr_sprintf(180, 146, "Y inc, B dec, A reset");
	
}


void	texviewer_menu_layer(__basic_menu * mnu)
{
	
	mnu->topLeft[X] = 120;
	mnu->topLeft[Y] = 40;
	mnu->scale[X] = 120;
	mnu->scale[Y] = 24;
	mnu->option_grid[X] = 1;
	mnu->option_grid[Y] = 5;
	mnu->num_option = 2;
	mnu->backColor = 6 + (64 * 2);
	mnu->optionColor = 5;
	static char * option_list[] = {"<- Back", "Select Texture"};
	mnu->option_text = option_list;
	
	static int texSelect = 0;
	
	if(is_key_release(DIGI_A))
	{
		pcm_play(snd_button, PCM_SEMI, 6);
		switch(mnu->selection)
		{
			case(0):
			menuLayer = HUD_LAYER_DEBUG;
			break;
			case(1):
			texSelect++;
			break;
			default:
			break;
		}
	}

	if(is_key_release(DIGI_B))
	{
		pcm_play(snd_button, PCM_SEMI, 6);
		switch(mnu->selection)
		{
			case(0):
			break;
			case(1):
			texSelect--;
			break;
			default:
			break;
		}
	}
	
	if(is_key_down(DIGI_X) && mnu->selection == 1)
	{
		texSelect++;
	}

	if(is_key_down(DIGI_Y) && mnu->selection == 1)
	{
		texSelect--;
	}

	spr_sprintf(16, 50, "Texno:%i", texSelect);
	spr_sprintf(16, 64, "Addr:%i", (pcoTexDefs[texSelect].SRCA<<3));
	spr_sprintf(16, 78, "End:%i", (pcoTexDefs[numTex-1].SRCA<<3));
	draw_normal_sprite(32, 96, texSelect, 1);

	
}

void	start_menu(void)
{

	static __basic_menu mnu;
	
	bad_frames = 0; //Reset the bad-frame ct on menu'ing
	
	if(is_key_struck(DIGI_RIGHT))	mnu.selection++;
	if(is_key_struck(DIGI_LEFT))	mnu.selection--;
	if(is_key_struck(DIGI_DOWN))	mnu.selection+= (mnu.option_grid[X]);
	if(is_key_struck(DIGI_UP))		mnu.selection-= (mnu.option_grid[X]);
	if(is_key_struck(DIGI_RIGHT) | is_key_struck(DIGI_LEFT)
	| is_key_struck(DIGI_DOWN) | is_key_struck(DIGI_UP)) pcm_play(snd_button2, PCM_SEMI, 6);
	
		if(menuLayer == HUD_LAYER_START)
		{
			start_menu_layer(&mnu);
		} else if(menuLayer == HUD_LAYER_DEBUG)
		{
			debug_menu_layer(&mnu);
		} else if(menuLayer == HUD_LAYER_LEVEL)
		{
			levelselect_menu_layer(&mnu);
		} else if(menuLayer == HUD_LAYER_OPTION_1)
		{
			options_menu_layer(&mnu);
		} else if(menuLayer == HUD_LAYER_OPTION_2)
		{
			options_menu_layer_2(&mnu);
		} else if(menuLayer == HUD_LAYER_TEXVIEWER)
		{
			texviewer_menu_layer(&mnu);
		} else if(menuLayer == HUD_LAYER_CONTROLS)
		{
			control_menu_layer(&mnu);
		} else if(menuLayer == HUD_LAYER_ITEMS)
		{
			items_menu_later(&mnu);
		}
	static int fuckinghatesynchingkeysvblankbullshit_timer = 0;
	fuckinghatesynchingkeysvblankbullshit_timer++;
	if(is_key_release(DIGI_START) && fuckinghatesynchingkeysvblankbullshit_timer > 30)
	{
		you.inMenu = false;
	}
	if(you.inMenu == false)
	{
		mnu.selection = 0;
		fuckinghatesynchingkeysvblankbullshit_timer = 0;
	}
	menu_with_options(&mnu);
	
	static int gdat_bg_tlpt[XY];
	static int gdat_bg_brpt[XY];
	gdat_bg_tlpt[X] = 70;
	gdat_bg_tlpt[Y] = 188;
	gdat_bg_brpt[X] = 350;
	gdat_bg_brpt[Y] = 224;
	draw2dSquare(gdat_bg_tlpt, gdat_bg_brpt, 18 + (64 * 2), 0, 5, 0);

	//spr_sprintf(244, 44,
	
}

void	iterate_sprite_to_position(_sprite * spr, int step, int * endPos)
{
	if(spr->lifetime > 0)
	{
		int difX = spr->pos[X] - endPos[X];
		int difY = spr->pos[Y] - endPos[Y];
		int difZ = spr->pos[Z] - endPos[Z];
		
		if(difX < 0 && difX < (-step))
		{
			spr->pos[X] += step;
		} else if(difX > 0 && difX > step)
		{
			spr->pos[X] -= step;
		}
		
		if(difY < 0 && difY < (-step))
		{
			spr->pos[Y] += step;
		} else if(difY > 0 && difY > step)
		{
			spr->pos[Y] -= step;
		}
		
		if(difZ < 0 && difZ < (-step))
		{
			spr->pos[Z] += step;
		} else if(difZ > 0 && difZ > step)
		{
			spr->pos[Z] -= step;
		}
	}
	
}
	
void	start_hud_event(short eventNum)
{
	_hudEvent * event = &hudEvents[eventNum];
	if(event->status == HUD_EVENT_CLOSE)
	{
		short prx[3] = {0, 0, 0};
		
		prx[Y] = event->strobe_interval;
		
		sprite_prep.info.drawMode = (event->strobe_type == EVENT_NO_STROBE) ? SPRITE_TYPE_NORMAL : event->strobe_type;
		sprite_prep.info.drawOnce = 0;
		sprite_prep.info.mesh = 0;
		sprite_prep.info.sorted = 0;
		
		short tsp = add_to_sprite_list(event->startPos, prx, event->texno, event->colorBank, sprite_prep, 0, event->spriteTime);
		if(tsp != -1)
		{
			event->spr = &sprWorkList[tsp];
			event->status = HUD_EVENT_RUN;
		} else {
			event->status = HUD_EVENT_CLOSE;
		}
	}
	
}

void	init_hud_events(void)
{
	_hudEvent * event;
	event = &hudEvents[RINGS_ALL_EVENT];
	
	event->startPos[X] = 176;
	event->startPos[Y] = 0;
	event->endPos[X] = 176;
	event->endPos[Y] = 140;
	event->eventTime = 40536; 
	event->spriteTime = 1<<16; //One second
	event->screenStep = 10;
	
	event->soundType = PCM_PROTECTED;//ADX_STREAM;
	event->soundNum = snd_orchit0;//stm_orchit0;
	event->volume = 6;
	
	event->texno = 0;
	static char ringgettxt[] = "All rings get!";
	event->text = &ringgettxt[0];
	event->colorBank = 3<<6;
	event->text_lines = 1;
	event->text_width = strlen(event->text);
	
	event = &hudEvents[RING1_EVENT];
	
	event->startPos[X] = 0;
	event->startPos[Y] = 175;
	event->endPos[X] = 176;
	event->endPos[Y] = 150;
	event->eventTime = 0;
	event->spriteTime = 1<<16;
	event->screenStep = 15;
	
	event->strobe_type = EVENT_STROBE_MESH;
	event->strobe_interval = 4000;
	
	// event->soundNum = snd_khit;
	// event->volume = 4;
	
	event->texno = baseRingMenuTexno;
	event->colorBank = 1<<6;
	
	event = &hudEvents[RING2_EVENT];
	*event = hudEvents[RING1_EVENT];
	event->texno = baseRingMenuTexno+1;
	
	event = &hudEvents[RING3_EVENT];
	*event = hudEvents[RING1_EVENT];
	event->texno = baseRingMenuTexno+2;

	event = &hudEvents[RING4_EVENT];
	*event = hudEvents[RING1_EVENT];
	event->texno = baseRingMenuTexno+3;

	event = &hudEvents[RING5_EVENT];
	*event = hudEvents[RING1_EVENT];
	event->texno = baseRingMenuTexno+4;
	
	event = &hudEvents[RING6_EVENT];
	*event = hudEvents[RING1_EVENT];
	event->texno = baseRingMenuTexno+5;
	
	event = &hudEvents[RING7_EVENT];
	*event = hudEvents[RING1_EVENT];
	event->texno = baseRingMenuTexno+6;
//////////////////////////////////////////////////
	event = &hudEvents[GATE_DISCOVERY_EVENT];
	
	event->startPos[X] = 176;
	event->startPos[Y] = 0;
	event->endPos[X] = 176;
	event->endPos[Y] = 140;
	event->eventTime = 1<<16; 
	event->spriteTime = 1<<16; //One second
	event->screenStep = 10;
	
	event->soundType = PCM_SEMI;
	event->soundNum = snd_khit;
	event->volume = 6;
	
	event->texno = EVENT_SHOW_TEXT;
	static char gatediscovertxt[] = "Gate discovered!";
	event->text = &gatediscovertxt[0];
	event->colorBank = 1<<6;
	event->text_lines = 1;
	event->text_width = strlen(event->text);
//////////////////////////////////////////////////
	event = &hudEvents[TRACK_DISCOVERED_EVENT];
	
	event->startPos[X] = 176;
	event->startPos[Y] = 0;
	event->endPos[X] = 176;
	event->endPos[Y] = 140;
	event->eventTime = 1<<16; 
	event->spriteTime = 1<<16; //One second
	event->screenStep = 10;
	
	event->soundType = PCM_PROTECTED;
	event->soundNum = snd_cronch;
	event->volume = 6;
	
	event->texno = EVENT_SHOW_TEXT;
	static char trackdiscoveredtxt[] = "All gates found. Ready to go?";
	event->text = &trackdiscoveredtxt[0];
	event->colorBank = 1<<6;
	event->text_lines = 1;
	event->text_width = strlen(event->text);
//////////////////////////////////////////////////
	event = &hudEvents[GATE_PASSED_EVENT];
	
	event->startPos[X] = 176;
	event->startPos[Y] = 0;
	event->endPos[X] = 176;
	event->endPos[Y] = 140;
	event->eventTime = 1<<16; 
	event->spriteTime = 1<<16; //One second
	event->screenStep = 10;
	
	event->soundType = PCM_PROTECTED;
	event->soundNum = snd_button;
	event->volume = 6;
	
	event->texno = EVENT_SHOW_TEXT;
	static char gatepasstxt[] = "^^^^^";
	event->text = &gatepasstxt[0];
	event->colorBank = 1<<6;
	event->text_lines = 1;
	event->text_width = strlen(event->text);
//////////////////////////////////////////////////
	event = &hudEvents[TRACK_START_EVENT];
	
	event->startPos[X] = 176;
	event->startPos[Y] = 0;
	event->endPos[X] = 176;
	event->endPos[Y] = 140;
	event->eventTime = 1<<16; 
	event->spriteTime = 1<<16; //One second
	event->screenStep = 10;
	
	event->soundType = PCM_PROTECTED;
	event->soundNum = snd_gpass;
	event->volume = 6;
	
	event->texno = EVENT_SHOW_TEXT;
	static char trackstarttxt[] = "Track start!";
	event->text = &trackstarttxt[0];
	event->colorBank = 1<<6;
	event->text_lines = 1;
	event->text_width = strlen(event->text);
//////////////////////////////////////////////////
	event = &hudEvents[RESET_TIMERS_EVENT];
	
	event->startPos[X] = 176;
	event->startPos[Y] = 0;
	event->endPos[X] = 176;
	event->endPos[Y] = 140;
	event->eventTime = 1<<16; 
	event->spriteTime = 1<<16; //One second
	event->screenStep = 10;
	
	event->soundType = PCM_PROTECTED;
	event->soundNum = snd_rlap;
	event->volume = 6;
	
	event->texno = EVENT_SHOW_TEXT;
	static char timeresetevent[] = "Track & Flag Reset!";
	event->text = &timeresetevent[0];
	event->colorBank = 1<<6;
	event->text_lines = 1;
	event->text_width = strlen(event->text);
//////////////////////////////////////////////////
	event = &hudEvents[TRACK_FAILED_EVENT];
	
	event->startPos[X] = 176;
	event->startPos[Y] = 0;
	event->endPos[X] = 176;
	event->endPos[Y] = 140;
	event->eventTime = 1<<16; 
	event->spriteTime = 1<<16; //One second
	event->screenStep = 10;
	
	event->soundType = PCM_PROTECTED;
	event->soundNum = snd_alarm;
	event->volume = 6;
	
	event->texno = EVENT_SHOW_TEXT;
	static char trackfailtxt[] = "Track Reset!";
	event->text = &trackfailtxt[0];
	event->colorBank = 1<<6;
	event->text_lines = 1;
	event->text_width = strlen(event->text);
//////////////////////////////////////////////////
	event = &hudEvents[TRACK_SLOW_EVENT];
	
	event->startPos[X] = 80;
	event->startPos[Y] = 0;
	event->endPos[X] = 80;
	event->endPos[Y] = 140;
	event->eventTime = 5<<16; 
	event->spriteTime = 5<<16; //One second
	event->screenStep = 10;
	
	event->soundType = PCM_PROTECTED;//ADX_STREAM;
	event->soundNum = snd_tslow;//stm_win;
	event->volume = 6;
	
	event->texno = slowImgTexno;
	static char trackbadtxt[] = "Slow! Par Time: %i";
	event->text = &trackbadtxt[0];
	event->printedData = &you.parTime;
	
	event->colorBank = 1<<6;
	
	event = &hudEvents[TRACK_PAR_EVENT];
	
	event->startPos[X] = 80;
	event->startPos[Y] = 0;
	event->endPos[X] = 80;
	event->endPos[Y] = 140;
	event->eventTime = 5<<16; 
	event->spriteTime = 5<<16; //One second
	event->screenStep = 10;
	
	event->soundType = parImgTexno;//ADX_STREAM;
	event->soundNum = snd_yeah;//stm_win;
	event->volume = 6;
	
	event->texno = parImgTexno;
	static char trackpartxt[] = "Complete! Avg Sanics: %i";
	event->text = &trackpartxt[0];
	event->printedData = &you.end_average;
	
	event->colorBank = 1<<6;
	
	event = &hudEvents[TRACK_GOLD_EVENT];
	
	event->startPos[X] = 80;
	event->startPos[Y] = 0;
	event->endPos[X] = 80;
	event->endPos[Y] = 140;
	event->eventTime = 5<<16; 
	event->spriteTime = 5<<16; //One second
	event->screenStep = 10;
	
	event->soundType = PCM_PROTECTED;//ADX_STREAM;
	event->soundNum = snd_win;//stm_win;
	event->volume = 6;
	
	event->texno = goldImgTexno;
	static char trackwintxt[] = "Gold! Avg Sanics: %i";
	event->text = &trackwintxt[0];
	event->printedData = &you.end_average;
	
	event->colorBank = 1<<6;
/////////////////////////////////////////////////
	event = &hudEvents[FLAG_RETURNED_EVENT];
	
	event->startPos[X] = 176;
	event->startPos[Y] = 0;
	event->endPos[X] = 176;
	event->endPos[Y] = 140;
	event->eventTime = 1<<16; 
	event->spriteTime = 1<<16; //One second
	event->screenStep = 10;
	
	event->soundType = PCM_PROTECTED;//ADX_STREAM;
	event->soundNum = snd_freturn;//stm_freturn;
	event->volume = 5;
	
	event->texno = EVENT_SHOW_TEXT;
	static char flagreturntxt[] = "Flag returned!";
	event->text = &flagreturntxt[0];
	
	event->colorBank = 1<<6;
	event->text_lines = 1;
	event->text_width = strlen(event->text);
//////////////////////////////////////////////////
	event = &hudEvents[FLAG_SLOW_EVENT];
	
	event->startPos[X] = 80;
	event->startPos[Y] = 0;
	event->endPos[X] = 80;
	event->endPos[Y] = 140;
	event->eventTime = 5<<16; 
	event->spriteTime = 5<<16;
	event->screenStep = 10;
	
	event->soundType = PCM_PROTECTED;//ADX_STREAM;
	event->soundNum = snd_tslow;//stm_win;
	event->volume = 5;
	
	event->texno = slowImgTexno;
	static char flagfailtxt[] = "Slow! Par Time: %i";
	event->text = &flagfailtxt[0];
	event->printedData = &you.parTime;
	
	event->colorBank = 1<<6;
	event->text_lines = 1;
	event->text_width = strlen(event->text);
	
	event = &hudEvents[FLAG_PAR_EVENT];
	
	event->startPos[X] = 80;
	event->startPos[Y] = 0;
	event->endPos[X] = 80;
	event->endPos[Y] = 140;
	event->eventTime = 5<<16; 
	event->spriteTime = 5<<16;
	event->screenStep = 10;
	
	event->soundType = PCM_PROTECTED;//ADX_STREAM;
	event->soundNum = snd_yeah;//stm_win;
	event->volume = 5;
	
	event->texno = parImgTexno;
	static char flagpartxt[] = "Success! Avg Sanics: %i";
	event->text = &flagpartxt[0];
	event->printedData = &you.end_average;
	
	event->colorBank = 1<<6;
	event->text_lines = 1;
	event->text_width = strlen(event->text);
	
	event = &hudEvents[FLAG_GOLD_EVENT];
	
	event->startPos[X] = 80;
	event->startPos[Y] = 0;
	event->endPos[X] = 80;
	event->endPos[Y] = 140;
	event->eventTime = 5<<16; 
	event->spriteTime = 5<<16;
	event->screenStep = 10;
	
	event->soundType = PCM_PROTECTED;//ADX_STREAM;
	event->soundNum = snd_win;//stm_win;
	event->volume = 5;
	
	event->texno = goldImgTexno;
	static char flagwintxt[] = "Gold! Avg Sanics: %i";
	event->text = &flagwintxt[0];
	event->printedData = &you.end_average;
	
	event->colorBank = 1<<6;
	event->text_lines = 1;
	event->text_width = strlen(event->text);
//////////////////////////////////////////////////
	
	event = &hudEvents[UsePrompt];
	
	event->startPos[X] = 352>>1;
	event->startPos[Y] = 224>>1;
	event->endPos[X] = 352>>1;
	event->endPos[Y] = 224>>1;
	event->eventTime = 1<<16; 
	event->spriteTime = 1<<16; //One second
	event->screenStep = 10;
	
	event->soundType = EVENT_NO_SOUND;
	event->soundNum = 0;
	event->volume = 0;
	
	event->texno = EVENT_SHOW_TEXT;
	static char useText[] = "[Use]";
	event->text = &useText[0];
	event->colorBank = 1<<6;
	event->text_lines = 1;
	event->text_width = 5;
	
}

void	hud_menu(void)
{
	//HUD Menu
	//Host of drawing on-screen sprite events, etc
	
	//List of events I want:
	// -> Flag open
	// -> Flag taken
	// -> Flag returned
	// -> x/7 Ring Collected
	// -> Gate Discovered
	// -> All Gates Discovered
	// -> Gate Timer Start
	// -> Gate Timer Reset
	// -> Gate Win
	for(int i = 0; i < HUD_EVENT_TYPES; i++)
	{
		if(hudEvents[i].status == HUD_EVENT_CLOSE)
		{
			continue;
		} else if(hudEvents[i].status == HUD_EVENT_START)
		{
			start_hud_event(i);
		} else if(hudEvents[i].status == HUD_EVENT_RUN || hudEvents[i].status == HUD_EVENT_DONE)
		{
			iterate_sprite_to_position(hudEvents[i].spr, hudEvents[i].screenStep, hudEvents[i].endPos);
			if(hudEvents[i].texno == 0)
			{
				int offset_x = hudEvents[i].text_width * 4;
				spr_sprintf(hudEvents[i].spr->pos[X] - offset_x, hudEvents[i].spr->pos[Y], hudEvents[i].text, *hudEvents[i].printedData);
				int back_p1[2] = {hudEvents[i].spr->pos[X] - offset_x, hudEvents[i].spr->pos[Y]};
				int back_p2[2] = {hudEvents[i].spr->pos[X] + offset_x, hudEvents[i].spr->pos[Y] + (12 * hudEvents[i].text_lines)};
				draw2dSquare(back_p1, back_p2, 63, 0, 5, 1);
			}
			
			if(hudEvents[i].spr->lifetime < hudEvents[i].eventTime && hudEvents[i].status == HUD_EVENT_RUN)
			{
				//Sound to play on event time
				if(hudEvents[i].soundType == ADX_STREAM)
				{
					start_adx_stream(stmsnd[hudEvents[i].soundNum], hudEvents[i].volume);
				} else if(hudEvents[i].soundType != EVENT_NO_SOUND)
				{
					pcm_play(hudEvents[i].soundNum, PCM_PROTECTED, hudEvents[i].volume);
				}
				hudEvents[i].status = HUD_EVENT_DONE;
			}
			if(hudEvents[i].spr->lifetime < 0)
			{
				hudEvents[i].status = HUD_EVENT_CLOSE;
			} else if(hudEvents[i].spr->extra == 1 && hudEvents[i].volume != 0)
			{
				//Sound to play on strobe
				pcm_play(hudEvents[i].soundNum, PCM_PROTECTED, hudEvents[i].volume);
				hudEvents[i].spr->extra = 0;
			}
		}	
	}
	
	
	draw_minimap();
	update_mmap_1pass();
}



