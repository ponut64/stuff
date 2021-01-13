//Uses Jo Engine, copyright Johannes Fetz
//MIT
/**
In Loving Memory of Jeffrey Neil Lindamood,
my father. 1962 - 2019.
And Bonnie K Caulder Lindamood,
my grandmother. 1937 - 2019.
I am sorry for the pain you had to go through.
**/
//
// Compilation updated to use latest version of Jo Engine standard compiler.
//
//
#include <jo/jo.h>
#include "def.h"
#include <SEGA_INT.H>

//Outstanding code contributions from XL2 
//
#include "anidefs.h"

#include "draw.h"
#include "bounder.h"
#include "collision.h"
#include "control.h"
#include "mloader.h"
#include "hmap.h"
#include "msfs.h"
#include "vdp2.h"
#include "physobjet.h"
#include "render.h"
#include "tga.h"
#include "ldata.h"
#include "input.h"
#include "object_col.h"
//
#include "lwram.c"
//
//
#include "dspm.h"
//
// Game data //
//Be very careful with uninitialized pointers. [In other words, INITIALIZE POINTERS!]
// SGL Work Area is using the last 200KB of High Work RAM. The game binary is using about 200KB.
// Jo Engine is using at least 100KB. 
// My heightmap polygon model is using about 32KB.
// And then there's just some... "raff" being used, here or there.
// Let's say then your game code can use about 400KB of HWRAM.
unsigned char hwram_model_data[256 * 1024];
//

//
short * division_table;

//A zero vector to be used when you want zero.
POINT zPt = {0, 0, 0};
extern Sint8 SynchConst; //SGL System Variable
int framerate;
int frmul;

unsigned char * dirty_buf;
void * currentAddress;

volatile Uint32 * scuireg = (Uint32*)0x25FE00A4;
volatile Uint32 * scuimask = (Uint32*)0x25FE00A0;
volatile Uint32 * scudmareg =  (Uint32*)0x25FE007C;

//////////////////////////////////////////////////////////////////////////////
//Sound Numbers
//////////////////////////////////////////////////////////////////////////////
 int snd_dash;
 int snd_lstep;
 int snd_wind;
 int snd_bstep;
 int snd_click;
 int snd_button;
 int snd_cronch;
 int snd_alarm;
 int snd_win;
 int snd_bwee;
//////////////////////////////////////////////////////////////////////////////
//Animation Structs
//////////////////////////////////////////////////////////////////////////////
 animationControl walk;
 animationControl run;
 animationControl dbound;

 animationControl runshoot;
 animationControl runmelee;

 animationControl melee;
 animationControl shoot;

 animationControl idle;

 animationControl jump;
 animationControl stop;

 animationControl airShoot;
 animationControl airMelee;

 animationControl airIdle;
 animationControl airRight;
 animationControl airLeft;
 animationControl slideIdle;
 animationControl slideRln;
 animationControl slideLln;
//////////////////////////////////////////////////////////////////////////////
// Player data struct
//////////////////////////////////////////////////////////////////////////////
	_player you;

void	dpinit(void)
{
	init_vdp2();
	initCamera();
}

#define GRAPH_X_OFFSET (12)

//borrowed/given by XL2 -- Frame limiter to 30 FPS. EXTREMELY USEFUL.
void	update_gamespeed(void)
{
	int frmrt = delta_time>>6;
	jo_fixed_point_time();
	
 	static int lastTimes[66];
	static int time_selector = 0;
	static int bad_frames = 0;
	
	lastTimes[time_selector] = frmrt;
	//If the frame-time is too fast or too slow, mark it as a bad frame.
	bad_frames += (frmrt < 30 || frmrt > 35) ? 1 : 0;
	time_selector = (time_selector > 66) ? 0 : time_selector+1;
	
    framerate = (frmrt)>>4;
	jo_printf(1, 3, "(%i) Bad Frames)", bad_frames);
	
    if (framerate <= 0) framerate=1;
    else if (framerate > 5) framerate=5;

		//Framegraph
	char curLine = frmrt;
	char prevLine = (time_selector < 1) ? lastTimes[65] : lastTimes[time_selector-1];
	char nthLine = (time_selector < 2) ? lastTimes[65] : lastTimes[time_selector-2];
	
	jo_draw_background_line(time_selector+GRAPH_X_OFFSET, 22, time_selector+GRAPH_X_OFFSET, 8, 0xC210); //(last argument is color)
	jo_draw_background_line(time_selector+GRAPH_X_OFFSET, 22, time_selector+GRAPH_X_OFFSET, (curLine>>2)+6, 0x8200);
		if(time_selector > 1){
	jo_draw_background_line((time_selector-1)+GRAPH_X_OFFSET, 22, (time_selector-1)+GRAPH_X_OFFSET, (prevLine>>2)+6, 0xC000);
		}
		if(time_selector > 2){
	jo_draw_background_line((time_selector-2)+GRAPH_X_OFFSET, 22, (time_selector-2)+GRAPH_X_OFFSET, (nthLine>>2)+6, 0x8010);
		} 
		//
		frmul = framerate<<16;
		
		
		if(is_key_down(DIGI_START) && is_key_down(DIGI_A) && is_key_down(DIGI_B) && is_key_down(DIGI_C))
		{
			SYS_Exit(0);
		}
}

//Loading. Check msfs.c and mloader c/h
void	load_test(void)
{
	//
	snd_bwee = load_8bit_pcm((Sint8*)"BWEE.PCM", 15360);
	snd_lstep = load_8bit_pcm((Sint8*)"LSTEP.PCM", 15360);
	snd_wind = load_8bit_pcm((Sint8*)"WND.PCM", 15360);
	snd_bstep = load_8bit_pcm((Sint8*)"STEP.PCM", 15360);
	snd_click = load_8bit_pcm((Sint8*)"CLCK1.PCM", 15360);
	snd_button = load_8bit_pcm((Sint8*)"BTN1.PCM", 15360);
	snd_cronch = load_8bit_pcm((Sint8*)"CRONCH.PCM", 15360);
	snd_alarm = load_8bit_pcm((Sint8*)"ALARM.PCM", 15360);
	snd_win = load_8bit_pcm((Sint8*)"WIN.PCM", 15360);
	//Next up: TGA file system handler?
	int map_tex_start = numTex;
	WRAP_NewPalette((Sint8*)"TADA.TGA", (void*)dirty_buf);
	WRAP_NewTable((Sint8*)"DIR0.TGA", (void*)dirty_buf, 0);
	WRAP_NewTable((Sint8*)"DIR1.TGA", (void*)dirty_buf, 0);
	WRAP_NewTable((Sint8*)"DIR2.TGA", (void*)dirty_buf, 0);
	WRAP_NewTable((Sint8*)"DIR3.TGA", (void*)dirty_buf, 0);
	WRAP_NewTable((Sint8*)"DIR4.TGA", (void*)dirty_buf, 0);
	map_tex_amt = (numTex);
	make_4way_combined_textures(map_tex_start, map_tex_amt);
	//End tex 35
// gvModelRequest((Sint8*)"DPONY.GVP", &pl_model, true, SORT_CEN, 'P');
// gvModelRequest((Sint8*)"SHADOW.GVP", &shadow, true, SORT_CEN, 'N');

active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"DPONY.GVP", 		active_HWRAM_ptr, &pl_model,    SORT_CEN, 'P');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"SHADOW.GVP", 		active_HWRAM_ptr, &shadow,	    SORT_CEN, 'N');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"PINE.GVP",		active_HWRAM_ptr, &entities[2], SORT_CEN, 'N');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"LOG.GVP",			active_HWRAM_ptr, &entities[3], SORT_CEN, 'N');

active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"BENCH.GVP",		active_HWRAM_ptr, &entities[4], SORT_CEN, 'N');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"TABLE.GVP",		active_HWRAM_ptr, &entities[5], SORT_CEN, 'N');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"COUCH.GVP",		active_HWRAM_ptr, &entities[6], SORT_CEN, 'N');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"CRATE.GVP",		active_HWRAM_ptr, &entities[7], SORT_CEN, 'N');

active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"FIRE.GVP",		active_HWRAM_ptr, &entities[8], SORT_CEN, 'N');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"FURNACE.GVP",		active_HWRAM_ptr, &entities[9], SORT_CEN, 'N');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"LAMP.GVP",		active_HWRAM_ptr, &entities[10], SORT_CEN, 'N');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"LANTERN.GVP",		active_HWRAM_ptr, &entities[11], SORT_CEN, 'N');

active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"CANE.GVP",		active_HWRAM_ptr, &entities[12], SORT_CEN, 'N');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"SNOWMAN.GVP",		active_HWRAM_ptr, &entities[13], SORT_CEN, 'N');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"WREATH.GVP",		active_HWRAM_ptr, &entities[14], SORT_CEN, 'N');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"GIFT.GVP",		active_HWRAM_ptr, &entities[15], SORT_CEN, 'N');

active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"CART.GVP",		active_HWRAM_ptr, &entities[16], SORT_CEN, 'N');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"PILE.GVP",		active_HWRAM_ptr, &entities[17], SORT_CEN, 'N');

active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"HOUSE.GVP",		active_HWRAM_ptr, &entities[18], SORT_CEN, 'B');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"IGLOO.GVP",		active_HWRAM_ptr, &entities[19], SORT_CEN, 'B');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"AIRPLAT.GVP",		active_HWRAM_ptr, &entities[20], SORT_CEN, 'B');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"TRAILER.GVP",		active_HWRAM_ptr, &entities[21], SORT_CEN, 'B');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"CAMP.GVP",		active_HWRAM_ptr, &entities[22], SORT_CEN, 'B');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"ISLE.GVP",		active_HWRAM_ptr, &entities[23], SORT_CEN, 'B');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"TOWER.GVP",		active_HWRAM_ptr, &entities[24], SORT_CEN, 'B');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"GATE0.GVP",		active_HWRAM_ptr, &entities[25], SORT_CEN, 'B');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"GATE1.GVP",		active_HWRAM_ptr, &entities[26], SORT_CEN, 'B');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"GATE2.GVP",		active_HWRAM_ptr, &entities[27], SORT_CEN, 'B');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"GATE3.GVP",		active_HWRAM_ptr, &entities[28], SORT_CEN, 'B');
active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"GATE4.GVP",		active_HWRAM_ptr, &entities[29], SORT_CEN, 'B');


// gvModelRequest((Sint8*)"PINE.GVP",		&entities[2], true, SORT_CEN, 'N');
// gvModelRequest((Sint8*)"LOG.GVP",		&entities[3], true, SORT_CEN, 'N');

// gvModelRequest((Sint8*)"BENCH.GVP",		&entities[4], true, SORT_CEN, 'N');
// gvModelRequest((Sint8*)"TABLE.GVP",		&entities[5], true, SORT_CEN, 'N');
// gvModelRequest((Sint8*)"COUCH.GVP",		&entities[6], true, SORT_CEN, 'N');
// gvModelRequest((Sint8*)"CRATE.GVP",		&entities[7], true, SORT_CEN, 'N');

// gvModelRequest((Sint8*)"FIRE.GVP",		&entities[8], true, SORT_CEN, 'N');
// gvModelRequest((Sint8*)"FURNACE.GVP",	&entities[9], true, SORT_CEN, 'N');
// gvModelRequest((Sint8*)"LAMP.GVP",		&entities[10], true, SORT_CEN, 'N');
// gvModelRequest((Sint8*)"LANTERN.GVP",	&entities[11], true, SORT_CEN, 'N');

// gvModelRequest((Sint8*)"CANE.GVP",		&entities[12], true, SORT_CEN, 'N');
// gvModelRequest((Sint8*)"SNOWMAN.GVP",	&entities[13], true, SORT_CEN, 'N');
// gvModelRequest((Sint8*)"WREATH.GVP",	&entities[14], true, SORT_CEN, 'N');
// gvModelRequest((Sint8*)"GIFT.GVP",		&entities[15], true, SORT_CEN, 'N');

// gvModelRequest((Sint8*)"CART.GVP",		&entities[16], true, SORT_CEN, 'N');
// gvModelRequest((Sint8*)"PILE.GVP",		&entities[17], true, SORT_CEN, 'N');

// gvModelRequest((Sint8*)"HOUSE.GVP",		&entities[18], true, SORT_CEN, 'B');
// gvModelRequest((Sint8*)"IGLOO.GVP",		&entities[19], true, SORT_CEN, 'B');
// gvModelRequest((Sint8*)"AIRPLAT.GVP",	&entities[20], true, SORT_CEN, 'B');
// gvModelRequest((Sint8*)"TRAILER.GVP",	&entities[21], true, SORT_CEN, 'B');
// gvModelRequest((Sint8*)"CAMP.GVP",		&entities[22], true, SORT_CEN, 'B');
// gvModelRequest((Sint8*)"ISLE.GVP",		&entities[23], true, SORT_CEN, 'B');
// gvModelRequest((Sint8*)"TOWER.GVP",		&entities[24], true, SORT_CEN, 'B');
// gvModelRequest((Sint8*)"GATE0.GVP",		&entities[25], true, SORT_CEN, 'B');
// gvModelRequest((Sint8*)"GATE1.GVP",		&entities[26], true, SORT_CEN, 'B');
// gvModelRequest((Sint8*)"GATE2.GVP",		&entities[27], true, SORT_CEN, 'B');
// gvModelRequest((Sint8*)"GATE3.GVP",		&entities[28], true, SORT_CEN, 'B');
// gvModelRequest((Sint8*)"GATE4.GVP",		&entities[29], true, SORT_CEN, 'B');

p64MapRequest(0);

}

void	game_frame(void)
{
	slCashPurge();
	update_gamespeed();
	master_draw_stats();
	frame_render_prep();
	
	master_draw(); 

	file_request_loop();
	
	slSynch();
}

void	my_vlank(void){
	vblank_requirements();
	operate_digital_pad1();
	//Sound Driver Stuff
	m68k_com->start = 1;
	m68k_com->dT_ms = delta_time>>6;
	music_vblIn(7);
	//
}

void	load_in_frame(void){
	do{
	master_file_system(game_frame);
	} while (true);
}

void	attributions(void)
{
	slPrint("Created by Ponut64", slLocate(3, 4));
	slPrint("Contributions:", slLocate(3, 6));
	slPrint("XL2 - Essential knowledge & tools", slLocate(3, 7));
	slPrint("Emerald Nova - fixed-point timer", slLocate(3, 11));
	slPrint("fafling - actually read VDP2 manual", slLocate(3, 13));
	slPrint("mrkotftw - formal programmer guy", slLocate(3, 14));
	slPrint("Johannez Fetz - good example code", slLocate(3, 15));
	slPrint("music from The Horde [TfB]", slLocate(3, 19));

	slPrint("Sound Driver by Ponut64 [dat me]", slLocate(3, 21));
	slPrint("Give it a second", slLocate(3, 24));
	
	load_test();
	
	jo_clear_screen();
}

void	jo_main(void)
{
	jo_core_init(0xE726); 
	//XL2
	slDynamicFrame(ON); //Dynamic framerate
	SynchConst = 2;

	//
	//Loading Area
	//
	init_lwram();
	active_HWRAM_ptr = &hwram_model_data[0];
	dpinit();
	init_render_area();
	initPhys();
	anim_defs();
	init_heightmap();
	//Sound Driver
	load_drv(); 
	//
	load_dsp_prog();
	//
	//The one interrupt that SGL has you register
	slIntFunction(my_vlank);

	//
	fill_obj_list();
	
	//load_test();
	attributions();
	
	set_camera();
	reset_player();
	load_in_frame();
}

