//menu.c
//This file compiled separately.

#include <SL_DEF.H>
#include <SGL.H>

#include "def.h"
#include "mymath.h"
#include "pcmsys.h"
#include "input.h"
#include "control.h"
#include "render.h"
#include "player_phy.h"
#include "physobjet.h"
#include "bounder.h"

#include "menu.h"

int viewInfoTxt = 1;

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
	mnu.backColor = 16;
	mnu.optionColor = 5;
	char * option_list[] = {"Return to Game", "Return to Start", "Next Object", "Prev Object", "Tgl Info Txt"};
	mnu.option_text = option_list;
	
	if(is_key_struck(DIGI_RIGHT))	mnu.selection++;
	if(is_key_struck(DIGI_LEFT))	mnu.selection--;
	if(is_key_struck(DIGI_DOWN))	mnu.selection+= (mnu.option_grid[X]);
	if(is_key_struck(DIGI_UP))		mnu.selection-= (mnu.option_grid[X]);
	if(is_key_struck(DIGI_RIGHT) | is_key_struck(DIGI_LEFT)
	| is_key_struck(DIGI_DOWN) | is_key_struck(DIGI_UP)) pcm_play(snd_click, PCM_SEMI, 6);
	
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
	spr_sprintf(8,		0  + (72),		"Object:(%i)", dPreview);
	spr_sprintf(8,		12 + (72),		"X:(%i)", rdobj->pix[X]);
	spr_sprintf(72,		12 + (72),		"Y:(%i)", rdobj->pix[Y]);
	spr_sprintf(8,		24 + (72),		"eID:(%i)", rdobj->type.entity_ID);
	spr_sprintf(8,		36 + (72),		"Rad:x(%i)", rdobj->type.radius[X]);
	spr_sprintf(112,	36 + (72),		",y(%i)", rdobj->type.radius[Y]);
	spr_sprintf(184,	36 + (72),		",z(%i)", rdobj->type.radius[Z]);
	spr_sprintf(8,		48 + (72),		"Rot:x(%i)", rdobj->rot[X] / 182);
	spr_sprintf(112,	48 + (72),		",y(%i)", rdobj->rot[Y] / 182);
	spr_sprintf(184,	48 + (72),		",z(%i)", rdobj->rot[Z] / 182);
	spr_sprintf(8,		60 + (72),		"edat:(%x)", rdobj->type.ext_dat);
	spr_sprintf(120,	60 + (72),		"MDAT:(%x)", rdobj->more_data);
	spr_sprintf(8,		72 + (72),		"luma:(%i)", rdobj->type.light_bright);
	spr_sprintf(8,		90 + (72),		"yofs:(%i)", rdobj->type.light_y_offset);
	spr_sprintf(48,		102 + (72),		"dist:(%i)", rdobj->dist);
	} else if(viewInfoTxt == 2)
	{
		if(dPreview >= MAX_PHYS_PROXY) dPreview = MAX_PHYS_PROXY-1;
	_boundBox * rdbox = &RBBs[dPreview];
	spr_sprintf(8,		0  + (72),		"RBB:(%i)", dPreview);
	spr_sprintf(8,		12 + (72),		"st0:(%c)", rdbox->status[0]);
	spr_sprintf(8,		24 + (72),		"st1:(%c)", rdbox->status[1]);
	spr_sprintf(8,		36 + (72),		"st2:(%c)", rdbox->status[2]);
	spr_sprintf(8,		48 + (72),		"pX:(%i)", rdbox->pos[X]);
	spr_sprintf(8,		60 + (72),		"pY:(%i)", rdbox->pos[Y]);
	spr_sprintf(8,		72 + (72),		"pZ:(%i)", rdbox->pos[Z]);
	spr_sprintf(8,		90 + (72),		"Object:(%i)", activeObjects[dPreview]);
	}
	
	static int fuckinghatesynchingkeysvblankbullshit_timer = 0;
	fuckinghatesynchingkeysvblankbullshit_timer++;
	if(is_key_release(DIGI_START) && fuckinghatesynchingkeysvblankbullshit_timer > 30) you.inMenu = false;
	if(you.inMenu == false)
	{
		mnu.selection = 0;
		fuckinghatesynchingkeysvblankbullshit_timer = 0;
	}
	menu_with_options(&mnu);
}



