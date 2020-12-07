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
//Discovery: the MC68EC00 in the Saturn is a .. 68k. I tell people its 32-bit, but.. is it really? no, dont think so
//

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
	bad_frames += (frmrt > 40) ? 1 : 0;
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
	WRAP_NewPalette((Sint8*)"TADA.TGA", (void*)dirty_buf);
	WRAP_NewTable((Sint8*)"DIR0.TGA", (void*)dirty_buf, 0);
	WRAP_NewTable((Sint8*)"DIR1.TGA", (void*)dirty_buf, 0);
	WRAP_NewTable((Sint8*)"DIR2.TGA", (void*)dirty_buf, 0);
	WRAP_NewTable((Sint8*)"DIR3.TGA", (void*)dirty_buf, 0);
	WRAP_NewTable((Sint8*)"DIR4.TGA", (void*)dirty_buf, 0);
	//End tex 35
gvModelRequest((Sint8*)"DPONY.GVP", &pl_model, true, SORT_CEN);

// gvModelRequest((Sint8*)"TRE.GVP",  &entities[2], false, SORT_CEN);

// gvModelRequest((Sint8*)"BRING.GVP",  &entities[0], false, SORT_CEN);

// gvModelRequest((Sint8*)"JOOSE.GVP",  &entities[1], false, SORT_CEN);

// gvModelRequest((Sint8*)"PILLAR.GVP",  &entities[3], false, SORT_CEN);

gvModelRequest((Sint8*)"SLANT.GVP",  &entities[4], false, SORT_CEN);

gvModelRequest((Sint8*)"SHADOW.GVP", &shadow, true, SORT_CEN);


p64MapRequest((Sint8*)"00");

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
	slPrint("XL2 - Binary model file process", slLocate(3, 7));
	slPrint("XL2 - Animation data (de)compression", slLocate(3, 8));
	slPrint("XL2 - Renderpath Prototype", slLocate(3, 9));
	slPrint("XL2 - Teaching", slLocate(3, 10));
	slPrint("Emerald Nova - fixed-point timer", slLocate(3, 11));
	slPrint("vbt - GCC 9.2 & SGL fixes", slLocate(3, 12));
	slPrint("fafling - actually read VDP2 manual", slLocate(3, 13));
	slPrint("mrkotftw - DMA & RISC education", slLocate(3, 14));
	slPrint("Johannez Fetz - dev environment", slLocate(3, 15));
	slPrint("& the few parts of jo engine", slLocate(3, 16));
	slPrint("that actually work [Kappa]", slLocate(3, 17));
	slPrint("music from Freedom Planet", slLocate(3, 19));
	slPrint("[ u h  o h ]", slLocate(3, 20));
	slPrint("Sound Driver by Ponut64 [dat me]", slLocate(3, 21));
	slPrint("PRESS START", slLocate(3, 24));
	do{slSynch();}while(!is_key_pressed(DIGI_START));
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
	active_HWRAM_ptr = (void*)jo_malloc(150 * 1024); //High Work Ram Entity Area 
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
	//attributions();
	//
	fill_obj_list();
	load_test();
	set_camera();
	reset_player();
	load_in_frame();
}

