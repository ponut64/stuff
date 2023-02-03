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

#include "render.h"
#include "collision.h"
#include "control.h"
#include "hmap.h"
#include "vdp2.h"
#include "physobjet.h"
#include "tga.h"
#include "ldata.h"
#include "input.h"
#include "object_col.h"
#include "pcmsys.h"
#include "pcmstm.h"
#include "draw.h"
#include "anidefs.h"
#include "player_phy.h"
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
unsigned char hwram_model_data[HWRAM_MODEL_DATA_HEAP_SIZE];
void * active_HWRAM_ptr;
//

//
short * division_table;

//short * sine_table;

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
 int snd_smack;
//////////////////////////////////////////////////////////////////////////////
//Animation Structs
//Why are these here?
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
 
 animationControl flap;
//////////////////////////////////////////////////////////////////////////////
// Player data struct
//////////////////////////////////////////////////////////////////////////////
	_player you;

void	dpinit(void)
{
	init_vdp2(0xE726);
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
		
		jo_printf(16, 4, "(%i) fmrt", frmrt);
		
		if(is_key_down(DIGI_START) && is_key_down(DIGI_A) && is_key_down(DIGI_B) && is_key_down(DIGI_C))
		{
			SYS_Exit(0);
		}
}

//Loading. Check msfs.c and mloader c/h
void	load_test(void)
{
	//
	active_HWRAM_ptr = (void *)(&hwram_model_data[0]);
	////////////////////////////////////////////////
	// REMINDER: All file names must comply with the 8.3 standard.
	// File extensions can be no longer than 3 letters.
	// File names can be no longer than 8 letters.
	////////////////////////////////////////////////
	snd_bwee = load_8bit_pcm((Sint8*)"BWEE.PCM", 15360);
	snd_lstep = load_8bit_pcm((Sint8*)"LSTEP.PCM", 15360);
	snd_wind = load_8bit_pcm((Sint8*)"WND.PCM", 15360);
	snd_bstep = load_8bit_pcm((Sint8*)"STEP.PCM", 15360);
	snd_click = load_8bit_pcm((Sint8*)"CLCK1.PCM", 15360);
	snd_button = load_8bit_pcm((Sint8*)"BTN1.PCM", 15360);
	snd_cronch = load_8bit_pcm((Sint8*)"CRONCH.PCM", 15360);
	snd_alarm = load_8bit_pcm((Sint8*)"ALARM.PCM", 15360);
	snd_win = load_8bit_pcm((Sint8*)"WIN.PCM", 15360);
	snd_smack = load_8bit_pcm((Sint8*)"MSMACK.PCM", 15360);
	//Next up: TGA file system handler?
	int map_tex_start = numTex;
	WRAP_NewPalette((Sint8*)"TADA.TGA", (void*)dirty_buf);
	map_texture_table_numbers[0] = numTex;
	WRAP_NewTable((Sint8*)"DIR0.TGA", (void*)dirty_buf, 0);
	map_texture_table_numbers[1] = numTex;
	WRAP_NewTable((Sint8*)"DIR1.TGA", (void*)dirty_buf, 0);
	map_texture_table_numbers[2] = numTex;
	WRAP_NewTable((Sint8*)"DIR2.TGA", (void*)dirty_buf, 0);
	map_texture_table_numbers[3] = numTex;
	WRAP_NewTable((Sint8*)"DIR3.TGA", (void*)dirty_buf, 0);
	map_texture_table_numbers[4] = numTex;
	WRAP_NewTable((Sint8*)"DIR4.TGA", (void*)dirty_buf, 0);
	map_tex_amt = (numTex);
	make_4way_combined_textures(map_tex_start, map_tex_amt);
	map_last_combined_texno = numTex;
	make_dithered_textures_for_map();
	
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"PONY.GVP", 		active_HWRAM_ptr, &pl_model,    GV_SORT_CEN, MODEL_TYPE_PLAYER);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"WINGS.GVP", 		active_HWRAM_ptr, &wings,	    GV_SORT_CEN, MODEL_TYPE_PLAYER);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"SHADOW.GVP", 		active_HWRAM_ptr, &shadow,	    GV_SORT_CEN, MODEL_TYPE_NORMAL);
	
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"KYOOB.GVP",		active_HWRAM_ptr, &entities[9], GV_SORT_CEN, MODEL_TYPE_NORMAL); 
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"PLATF00.GVP",		active_HWRAM_ptr, &entities[10], GV_SORT_CEN, MODEL_TYPE_NORMAL); 
	
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"BRIDGE1.GVP",		active_HWRAM_ptr, &entities[11], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"GREECE01.GVP",	active_HWRAM_ptr, &entities[12], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"GREECE02.GVP",	active_HWRAM_ptr, &entities[13], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"GREECE03.GVP",	active_HWRAM_ptr, &entities[14], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"GREECE04.GVP",	active_HWRAM_ptr, &entities[15], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"OVRHNG.GVP",		active_HWRAM_ptr, &entities[16], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"PIER1.GVP",		active_HWRAM_ptr, &entities[17], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"TUNNEL1.GVP",		active_HWRAM_ptr, &entities[18], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"TUNNEL2.GVP",		active_HWRAM_ptr, &entities[19], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"TUNNEL3.GVP",		active_HWRAM_ptr, &entities[20], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"WALL1.GVP",		active_HWRAM_ptr, &entities[21], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"BUILD00.GVP",		active_HWRAM_ptr, &entities[22], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"BRIDGE2.GVP",		active_HWRAM_ptr, &entities[23], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"OBSTCL1.GVP",		active_HWRAM_ptr, &entities[24], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"FLOAT01.GVP",		active_HWRAM_ptr, &entities[25], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"FLOAT02.GVP",		active_HWRAM_ptr, &entities[26], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	
	// active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"BB00.GVP",		active_HWRAM_ptr, &entities[0], GV_SORT_CEN, MODEL_TYPE_NORMAL);
	// active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"BB01.GVP",		active_HWRAM_ptr, &entities[1], GV_SORT_CEN, MODEL_TYPE_NORMAL);
	// active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"BB02.GVP",		active_HWRAM_ptr, &entities[2], GV_SORT_CEN, MODEL_TYPE_NORMAL);
	// active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"BB03.GVP",		active_HWRAM_ptr, &entities[3], GV_SORT_CEN, MODEL_TYPE_NORMAL);
	// active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"BB04.GVP",		active_HWRAM_ptr, &entities[4], GV_SORT_CEN, MODEL_TYPE_NORMAL);
	
	// active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"MEME00.GVP",		active_HWRAM_ptr, &entities[5], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	// active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"MEME01.GVP",		active_HWRAM_ptr, &entities[6], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	// active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"MEME02.GVP",		active_HWRAM_ptr, &entities[7], GV_SORT_CEN, MODEL_TYPE_BUILDING); 
	
	// active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"RING00.GVP",		active_HWRAM_ptr, &entities[8], GV_SORT_CEN, MODEL_TYPE_NORMAL); 
	// active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"POST00.GVP",		active_HWRAM_ptr, &entities[9], GV_SORT_CEN, MODEL_TYPE_NORMAL); 
	
	// active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"RTUNNEL.GVP",		active_HWRAM_ptr, &entities[11], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	// active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"TRACKOB2.GVP",	active_HWRAM_ptr, &entities[12], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	// active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"WALKWAL.GVP",		active_HWRAM_ptr, &entities[13], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	// active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"HTUNNEL.GVP",		active_HWRAM_ptr, &entities[14], GV_SORT_CEN, MODEL_TYPE_BUILDING);
	// active_HWRAM_ptr = gvLoad3Dmodel((Sint8*)"SLTUNNL.GVP",		active_HWRAM_ptr, &entities[15], GV_SORT_CEN, MODEL_TYPE_BUILDING);

	start_pcm_stream((Sint8*)"TRSC202.MUS", 3);
	stm.times_to_loop = 255;


	p64MapRequest(05);
	//
	
}

void	game_frame(void)
{
	slCashPurge();
	update_gamespeed();
	master_draw_stats();
	frame_render_prep();
	reset_pad(&pad1);
	master_draw(); 
}

void	my_vlank(void){
	vblank_requirements();
	operate_digital_pad1();
	//Sound Driver Stuff
	sdrv_stm_vblank_rq();
	sdrv_vblank_rq();
	//
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

	slPrint("Sound Driver by Ponut64 [dat me]", slLocate(3, 21));
	slPrint("Give it a second", slLocate(3, 24));
	jo_printf(3, 25, "(%i)md", hi_res_switch);
	
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
	dpinit();
	init_render_area(90 * 182);
	initPhys();
	anim_defs();
	init_heightmap();
	//Sound Driver
	load_drv(ADX_MASTER_2304); 
	//
	load_dsp_prog();
	//
	//The one interrupt that SGL has you register
	slIntFunction(my_vlank);
	//
	
	fill_obj_list();
	init_entity_list();
	
	//load_test();
	attributions();
	
	set_camera();
	reset_player();

	run_dsp(); //Dry-run the DSP to get it to flag done
	add_adx_front_buffer(11520);
	add_adx_back_buffer(dirty_buf);
	pcm_stream_init(30720, PCM_TYPE_8BIT);
	pcm_stream_host(game_frame);
}

