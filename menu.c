//menu.c
//This file compiled separately.

#include <SL_DEF.H>
#include <SEGA_GFS.H>
#include <SGL.H>

#include "def.h"
#include "mymath.h"
#include "pcmsys.h"
#include "pcmstm.h"
#include "input.h"
#include "control.h"
#include "render.h"
#include "player_phy.h"
#include "physobjet.h"
#include "bounder.h"
#include "minimap.h"

#include "menu.h"

int viewInfoTxt = 1;
int baseRingMenuTexno = 0;

_hudEvent hudEvents[HUD_EVENT_TYPES];

void	start_menu(void)
{

	static __basic_menu mnu;
	
	static short dPreview = 0;
	
	mnu.topLeft[X] = 120;
	mnu.topLeft[Y] = 40;
	mnu.scale[X] = 120;
	mnu.scale[Y] = 24;
	mnu.option_grid[X] = 1;
	mnu.option_grid[Y] = 5;
	mnu.num_option = 5;
	mnu.backColor = 79;
	mnu.optionColor = 5;
	char * option_list[] = {"Return to Game", "Return to Start", "Next Object", "Prev Object", "Tgl Info Txt"};
	mnu.option_text = option_list;
	
	if(is_key_struck(DIGI_RIGHT))	mnu.selection++;
	if(is_key_struck(DIGI_LEFT))	mnu.selection--;
	if(is_key_struck(DIGI_DOWN))	mnu.selection+= (mnu.option_grid[X]);
	if(is_key_struck(DIGI_UP))		mnu.selection-= (mnu.option_grid[X]);
	if(is_key_struck(DIGI_RIGHT) | is_key_struck(DIGI_LEFT)
	| is_key_struck(DIGI_DOWN) | is_key_struck(DIGI_UP)) pcm_play(snd_button2, PCM_SEMI, 6);
	
	if(is_key_release(DIGI_A))
	{
		pcm_play(snd_button, PCM_SEMI, 6);
		switch(mnu.selection)
		{
			case(0):
			you.inMenu = false;
			break;
			case(1):
			reset_player();
			you.inMenu = false;
			break;
			case(2):
			dPreview++;
			break;
			case(3):
			dPreview--;
			break;
			case(4):
			viewInfoTxt = (viewInfoTxt == 1) ? 2 : (viewInfoTxt == 2) ? 0 : 1;
			nbg_clear_text();
			break; 
			default:
			break;
		}
	}
	dPreview = (dPreview < 0) ? 0 : (dPreview >= MAX_WOBJS) ? MAX_WOBJS : dPreview;
	_declaredObject * rdobj = &dWorldObjects[dPreview];
	if(viewInfoTxt == 1)
	{
	spr_sprintf(8,		0  + (60),		"Object:(%i)", dPreview);
	spr_sprintf(8,		12 + (60),		"X:(%i)", rdobj->pix[X]);
	spr_sprintf(72,		12 + (60),		"Y:(%i)", rdobj->pix[Y]);
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
	draw2dSquare(gdat_bg_tlpt, gdat_bg_brpt, 76, 0);
	
	_declaredObject * someLDATA = get_first_in_object_list(LDATA);
	while(someLDATA != &dWorldObjects[objNEW])
	{
		if((someLDATA->type.ext_dat & LDATA_TYPE) == TRACK_DATA)
		{
			if(!(someLDATA->type.ext_dat & TRACK_COMPLETE))
			{
				spr_sprintf(100, 188, "Gates in Track:(%i)", someLDATA->pix[Y]);
				if(someLDATA->type.ext_dat & TRACK_DISCOVERED)
				{
				spr_sprintf(100, 200, "Track Discovered!");
				} else {
				spr_sprintf(100, 200, "Gates Discovered:(%i)", someLDATA->pix[X]);
				}
			} else {
				spr_sprintf(100, 200, "Track is Complete!");
			}
		} else if((someLDATA->type.ext_dat & LDATA_TYPE) == ITEM_MANAGER && (someLDATA->type.ext_dat & ITEM_CONDITION_TYPES) == MANAGER_7RINGS)
		{
			const int rix = 120;
			if(someLDATA->more_data & 0x1)
			{
				draw_normal_sprite(rix, 172, baseRingMenuTexno, 0);
			} else {
				draw_normal_sprite(rix, 172, baseRingMenuTexno, 0x8003);	
			}
			if(someLDATA->more_data & 0x2)
			{
				draw_normal_sprite(rix + 16, 172, baseRingMenuTexno+1, 0);	
			} else {
				draw_normal_sprite(rix + 16, 172, baseRingMenuTexno+1, 0x8003);
			}
			if(someLDATA->more_data & 0x4)
			{
				draw_normal_sprite(rix + 32, 172, baseRingMenuTexno+2, 0);
			} else {
				draw_normal_sprite(rix + 32, 172, baseRingMenuTexno+2, 0x8003);
			}
			if(someLDATA->more_data & 0x8)
			{
				draw_normal_sprite(rix + 48, 172, baseRingMenuTexno+3, 0);	
			} else {
				draw_normal_sprite(rix + 48, 172, baseRingMenuTexno+3, 0x8003);	
			}
			if(someLDATA->more_data & 0x10)
			{
				draw_normal_sprite(rix + 64, 172, baseRingMenuTexno+4, 0);
			} else {
				draw_normal_sprite(rix + 64, 172, baseRingMenuTexno+4, 0x8003);
			}
			if(someLDATA->more_data & 0x20)
			{
				draw_normal_sprite(rix + 80, 172, baseRingMenuTexno+5, 0);
			} else {
				draw_normal_sprite(rix + 80, 172, baseRingMenuTexno+5, 0x8003);
			}
			if(someLDATA->more_data & 0x40)
			{
				draw_normal_sprite(rix + 96, 172, baseRingMenuTexno+6, 0);
			} else {
				draw_normal_sprite(rix + 96, 172, baseRingMenuTexno+6, 0x8003);
			}
		} else if((someLDATA->type.ext_dat & LDATA_TYPE) == ITEM_MANAGER && (someLDATA->type.ext_dat & ITEM_CONDITION_TYPES) == MANAGER_CTF)
		{
			if(someLDATA->type.ext_dat & CTF_FLAG_CAPTURED)
			{
				spr_sprintf(100, 212, "Flag Captured!");
			} else if(someLDATA->type.ext_dat & CTF_FLAG_TAKEN)
			{
				spr_sprintf(100, 212, "Flag Taken!");
			} else if(someLDATA->type.ext_dat & CTF_FLAG_OPEN)
			{
				spr_sprintf(100, 212, "Flag is open!");
			} else {
				spr_sprintf(100, 212, "Flag stand is shielded!");
			}
		}
		
		someLDATA = step_linked_object_list(someLDATA);
	}
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
		char type = 'S';
		if(event->strobe_type != EVENT_NO_STROBE) type = event->strobe_type;
		
		short prx[3] = {0, 0, 0};
		
		prx[Y] = event->strobe_interval;
		
		short tsp = add_to_sprite_list(event->startPos, prx, event->texno, event->colorBank, 0, type, 0, event->spriteTime);
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
	
	event->soundType = ADX_STREAM;
	event->soundNum = stm_orchit0;
	event->volume = 6;
	
	event->texno = 5;
	event->colorBank = 1<<6;
	
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
			
			if(hudEvents[i].spr->lifetime < hudEvents[i].eventTime && hudEvents[i].status == HUD_EVENT_RUN)
			{
				//Sound to play on event time
				if(hudEvents[i].soundType == ADX_STREAM)
				{
					start_adx_stream(stmsnd[hudEvents[i].soundNum], hudEvents[i].volume);
				} else {
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



