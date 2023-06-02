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
#include <sl_def.h>
#include "def.h"
#include <SEGA_INT.H>
#include <SEGA_GFS.H>

//Outstanding code contributions from XL2 
//

#include "mymath.h"
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
#include "gamespeed.h"
#include "menu.h"
#include "sound.h"
//
#include "lwram.c"
//
//
#include "dspm.h"
//
// Game data //
//Be very careful with uninitialized pointers. [In other words, INITIALIZE POINTERS!]
// SGL Work Area is using the last 200KB of High Work RAM. The game binary is using about 200KB.
unsigned char hwram_model_data[HWRAM_MODEL_DATA_HEAP_SIZE];
void * HWRAM_ldptr;
void * HWRAM_hldptr;
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

int game_set_res = TV_320x240;

//////////////////////////////////////////////////////////////////////////////
int flagIconTexno = 0;
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
animationControl climbIdle;
animationControl climbing;
 
 animationControl flap;
//////////////////////////////////////////////////////////////////////////////
// Player data struct
//////////////////////////////////////////////////////////////////////////////
	_player you;

void	dpinit(void)
{
	init_vdp2(0xE726);
}

//Loading. Check msfs.c and mloader c/h
void	load_test(void)
{

	WRAP_NewPalette((Sint8*)"TADA.TGA", (void*)dirty_buf);
	baseAsciiTexno = numTex;
	sprAsciiHeight = 12;
	sprAsciiWidth = WRAP_NewTable((Sint8*)"FONT.TGA", dirty_buf, sprAsciiHeight); //last argument, tex height
	//
	HWRAM_ldptr = (void *)(&hwram_model_data[0]);
	////////////////////////////////////////////////
	// REMINDER: All file names must comply with the 8.3 standard.
	// File extensions can be no longer than 3 letters.
	// File names can be no longer than 8 letters.
	// The total length is thusly 12 characters (as there is a period).
	////////////////////////////////////////////////
	stmsnd[stm_win] = (Sint8*)"WIN.ADX";
	stmsnd[stm_freturn] = (Sint8*)"FRETURN.ADX";
	stmsnd[stm_orchit0] = (Sint8*)"ORCHIT0.ADX";
	snd_bwee = load_8bit_pcm((Sint8*)"BWEE.PCM", 15360);
	snd_lstep = load_8bit_pcm((Sint8*)"LSTEP.PCM", 15360);
	snd_wind = load_8bit_pcm((Sint8*)"WND.PCM", 15360);
	snd_bstep = load_8bit_pcm((Sint8*)"STEP.PCM", 15360);
	snd_cronch = load_8bit_pcm((Sint8*)"CRONCH.PCM", 15360);
	snd_alarm = load_8bit_pcm((Sint8*)"ALARM.PCM", 15360);
	snd_ftake = load_8bit_pcm((Sint8*)"FLAG.PCM", 15360);
	snd_smack = load_8bit_pcm((Sint8*)"MSMACK.PCM", 15360);
	snd_khit = load_8bit_pcm((Sint8*)"KICKHIT.PCM", 7680);
	snd_clack = load_8bit_pcm((Sint8*)"CLACK.PCM", 7680);
	snd_click = load_8bit_pcm((Sint8*)"CLICK.PCM", 7680);
	snd_close = load_8bit_pcm((Sint8*)"CLOSE.PCM", 7680);
	snd_button = load_8bit_pcm((Sint8*)"BUTTON1.PCM", 7680);
	snd_button2 = load_8bit_pcm((Sint8*)"BUTTON2.PCM", 7680);
	snd_ffield1 = load_8bit_pcm((Sint8*)"FOPEN.PCM", 7680);
	snd_ffield2 = load_8bit_pcm((Sint8*)"FCLOSE.PCM", 7680);
	snd_ring1 = load_8bit_pcm((Sint8*)"CRING1.PCM", 7680);
	snd_ring2 = load_8bit_pcm((Sint8*)"CRING2.PCM", 7680);
	snd_ring3 = load_8bit_pcm((Sint8*)"CRING3.PCM", 7680);
	snd_ring4 = load_8bit_pcm((Sint8*)"CRING4.PCM", 7680);
	snd_ring5 = load_8bit_pcm((Sint8*)"CRING5.PCM", 7680);
	snd_ring6 = load_8bit_pcm((Sint8*)"CRING6.PCM", 7680);
	snd_ring7 = load_8bit_pcm((Sint8*)"CRING7.PCM", 7680);
	baseRingMenuTexno = numTex;
	WRAP_NewTable((Sint8*)"RINGNUM.TGA", (void*)dirty_buf, 0);
	flagIconTexno = numTex;
	WRAP_NewTexture((Sint8*)"FLAGICON.TGA", (void*)dirty_buf);
	/////////////////////////////////////
	// Floor / heightmap textures
	/////////////////////////////////////
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
	map_end_of_original_textures = numTex;
	map_tex_amt = (numTex - map_texture_table_numbers[0]);
	make_4way_combined_textures(map_texture_table_numbers[0], map_end_of_original_textures, 0);
	map_last_combined_texno = numTex;
	make_dithered_textures_for_map(0);

/* 	WRAP_ReplaceTable((Sint8*)"DIRTEST.TGA", (void*)dirty_buf, 0, map_texture_table_numbers[2]); 
	make_4way_combined_textures(map_texture_table_numbers[0], map_end_of_original_textures-1, map_end_of_original_textures);
	make_dithered_textures_for_map(1); */

	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"PONY.GVP", 		HWRAM_ldptr, &pl_model,    GV_SORT_CEN, MODEL_TYPE_PLAYER, NULL);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"WINGS.GVP", 		HWRAM_ldptr, &wings,	    GV_SORT_CEN, MODEL_TYPE_PLAYER, NULL);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"SHADOW.GVP", 		HWRAM_ldptr, &shadow,	    GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL);
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RING1.GVP",		HWRAM_ldptr, &entities[1], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RING2.GVP",		HWRAM_ldptr, &entities[2], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RING3.GVP",		HWRAM_ldptr, &entities[3], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RING4.GVP",		HWRAM_ldptr, &entities[4], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RING5.GVP",		HWRAM_ldptr, &entities[5], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RING6.GVP",		HWRAM_ldptr, &entities[6], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RING7.GVP",		HWRAM_ldptr, &entities[7], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"KYOOB.GVP",		HWRAM_ldptr, &entities[9], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"PLATF00.GVP",		HWRAM_ldptr, &entities[10], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"FLAG.GVP",			HWRAM_ldptr, &entities[57], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"FFIELD.GVP",		HWRAM_ldptr, &entities[55], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 

	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"TEST00.GVP",		HWRAM_ldptr, &entities[0], GV_SORT_CEN, MODEL_TYPE_BUILDING, NULL);
		
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"BRIDGE1.GVP",		HWRAM_ldptr, &entities[11], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"GREECE01.GVP",		HWRAM_ldptr, &entities[12], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"GREECE02.GVP",		HWRAM_ldptr, &entities[13], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"GREECE03.GVP",		HWRAM_ldptr, &entities[14], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"GREECE04.GVP",		HWRAM_ldptr, &entities[15], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"OVRHNG.GVP",		HWRAM_ldptr, &entities[16], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"PIER1.GVP",		HWRAM_ldptr, &entities[17], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);

	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"POST00.GVP",		HWRAM_ldptr, &entities[18], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"TUNNEL2.GVP",		HWRAM_ldptr, &entities[19], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"TUNNEL3.GVP",		HWRAM_ldptr, &entities[20], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"WALL1.GVP",		HWRAM_ldptr, &entities[21], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"FLOAT03.GVP",		HWRAM_ldptr, &entities[22], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"BRIDGE2.GVP",		HWRAM_ldptr, &entities[23], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"OBSTCL1.GVP",		HWRAM_ldptr, &entities[24], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"FLOAT01.GVP",		HWRAM_ldptr, &entities[25], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"FLOAT02.GVP",		HWRAM_ldptr, &entities[26], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"HIWAY01.GVP",		HWRAM_ldptr, &entities[27], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"HIWAY02.GVP",		HWRAM_ldptr, &entities[28], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"HIWAY03.GVP",		HWRAM_ldptr, &entities[29], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"HIWAY04.GVP",		HWRAM_ldptr, &entities[30], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"HIWAY05.GVP",		HWRAM_ldptr, &entities[31], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"HIWAY06.GVP",		HWRAM_ldptr, &entities[32], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RAMP01.GVP",		HWRAM_ldptr, &entities[33], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"HIWAY07.GVP",		HWRAM_ldptr, &entities[34], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"TOWER01.GVP",		HWRAM_ldptr, &entities[35], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"STARSTAN.GVP",		HWRAM_ldptr, &entities[51], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"FLAGSTAN.GVP",		HWRAM_ldptr, &entities[53], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"GOALSTAN.GVP",		HWRAM_ldptr, &entities[54], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_hldptr = HWRAM_ldptr;
	
	p64MapRequest(0);
	//
	
}

void	game_frame(void)
{
	slCashPurge();
	prep_map_mtx();
	//ABC+Start Exit Condition
	if(is_key_down(DIGI_START) && is_key_down(DIGI_A) && is_key_down(DIGI_B) && is_key_down(DIGI_C))
	{
		SYS_Exit(0);
	}
	
	update_gamespeed();
	maintRand();
	master_draw_stats();
	frame_render_prep();
	master_draw(); 
	operate_stage_music();
	reset_pad(&pad1);
}

void	my_vlank(void)
{
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
	slPrint("dannyduarte - fixed workarea.c", slLocate(3, 8));
	slPrint("ReyeMe - for his czechnology", slLocate(3, 9));
	slPrint("Emerald Nova - fixed-point timer", slLocate(3, 11));
	slPrint("fafling - actually read VDP2 manual", slLocate(3, 13));
	slPrint("mrkotftw - formal programmer guy", slLocate(3, 14));
	slPrint("Johannez Fetz - good example code", slLocate(3, 15));

	slPrint("Sound Driver by Ponut64", slLocate(3, 21));
	
	//testing_level_data((Sint8*)"LIST0.REM", (void*)dirty_buf);
	
	load_test();
	
	nbg_clear_text();
}

int	main(void)
{
	//jo_core_init(0xE726); 
	game_set_res = (hi_res_switch) ? TV_704x448 : TV_352x224;
	slInitSystem(game_set_res, NULL, 2);
	slDynamicFrame(ON); //Dynamic framerate
    SynchConst = (Sint8)2;  //Framerate control. 1/60 = 60 FPS, 2/60 = 30 FPS, etc.
	//
	//Loading Area
	//
	init_lwram();
	dpinit();
	init_render_area(90 * 182);
	initPhys();
	init_box_handling();
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
	init_hud_events();
	
	set_camera();
	reset_player();

	run_dsp(); //Dry-run the DSP to get it to flag done
	add_adx_front_buffer(23040);
	add_adx_back_buffer(dirty_buf);
	pcm_stream_init(30720, PCM_TYPE_8BIT);
	pcm_stream_host(game_frame);
	return 1;
}

