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
// Compilation now requires a _very specifically set_ GCC 9.2 that should be included alongside the source code.
//
//free music source: https://patrickdearteaga.com/arcade-music/
//
#include <jo/jo.h>
#include "def.h"
#include <SEGA_INT.H>

//Borrowed/given from XL2
//vdp2.c was here
//
#include "anidefs.h"
//
#include "draw.h"
#include "bounder.h"
#include "collision.h"
#include "control.h"
#include "ZT/ZT_COMMON.h"
#include "hmap.h"
#include "msfs.h"
#include "vdp2.h"
#include "physobjet.h"
#include "render.h"
#include "tga.h"
#include "ldata.h"
#include "dspm.h"
//
#include "lwram.c"
//
#include <sddrvs.dat>
const char map[] = {0xff, 0xff, 0xff, 0xff}; //For sound driver

//
// Game data //
//Be very careful with uninitialized pointers. [In other words, INITIALIZE POINTERS!]
//Discovery: the MC68EC00 in the Saturn is a .. 68k. I tell people its 32-bit, but.. is it really? no, dont think so
//

//A zero vector to be used when you want zero.
POINT zPt = {0, 0, 0};
extern Sint8 SynchConst; //SGL System Variable
Sint32 framerate;

unsigned char * dirty_buf;

volatile Uint32 * scuireg = (Uint32*)0x25FE00A4;
volatile Uint32 * scuimask = (Uint32*)0x25FE00A0;
volatile Uint32 * scudmareg =  (Uint32*)0x25FE007C;

void	dpinit(void)
{
	init_vdp2();
	initCamera();
}

#define GRAPH_X_OFFSET (12)

//borrowed/given by XL2 -- Frame limiter to 30 FPS. EXTREMELY USEFUL.
void	update_gamespeed(void)
{
	int frmrt = dt>>6;
	timer();
	
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
	
	jo_draw_background_line(time_selector+GRAPH_X_OFFSET, 22, time_selector+GRAPH_X_OFFSET, 8, 0xC210);
	jo_draw_background_line(time_selector+GRAPH_X_OFFSET, 22, time_selector+GRAPH_X_OFFSET, (curLine>>2)+6, 0x8200);
		if(time_selector > 1){
	jo_draw_background_line((time_selector-1)+GRAPH_X_OFFSET, 22, (time_selector-1)+GRAPH_X_OFFSET, (prevLine>>2)+6, 0xC000);
		}
		if(time_selector > 2){
	jo_draw_background_line((time_selector-2)+GRAPH_X_OFFSET, 22, (time_selector-2)+GRAPH_X_OFFSET, (nthLine>>2)+6, 0x8010);
		} 
		//
}

//Loading. Check msfs.c and ZT_LOADMODEL.h for more information.
void	load_test(void)
{
	//Next up: TGA file system handler?
	WRAP_NewPalette((Sint8*)"TADA.TGA", (void*)dirty_buf);
	WRAP_NewTable((Sint8*)"DIR0.TGA", (void*)dirty_buf, 0);
	WRAP_NewTable((Sint8*)"DIR1.TGA", (void*)dirty_buf, 0);
	WRAP_NewTable((Sint8*)"DIR2.TGA", (void*)dirty_buf, 0);
	WRAP_NewTable((Sint8*)"DIR3.TGA", (void*)dirty_buf, 0);
	WRAP_NewTable((Sint8*)"DIR4.TGA", (void*)dirty_buf, 0);
	//End tex 35
ztModelRequest((Sint8*)"BPONY.ZTP", &pl_model, true, SORT_CEN, numTex);
	WRAP_NewTable((Sint8*)"PONTX.TGA", (void*)dirty_buf, 0);
	WRAP_NewTexture((Sint8*)"LEYE.TGA", (void*)dirty_buf);
	WRAP_NewTexture((Sint8*)"REYE.TGA", (void*)dirty_buf);
ztModelRequest((Sint8*)"TRE.ZTP",  &entities[2], true, SORT_CEN, numTex);
	WRAP_NewTable((Sint8*)"TRE.TGA", (void*)dirty_buf, 0);
ztModelRequest((Sint8*)"BRING.ZTP",  &entities[0], true, SORT_CEN, numTex);
	WRAP_NewTable((Sint8*)"GTEXT.TGA", (void*)dirty_buf, 0);
ztModelRequest((Sint8*)"JOOSE.ZTP",  &entities[1], true, SORT_CEN, numTex);
	WRAP_NewTable((Sint8*)"JOSTEX.TGA", (void*)dirty_buf, 0);
ztModelRequest((Sint8*)"PILLAR.ZTP",  &entities[3], true, SORT_CEN, numTex);
	WRAP_NewTable((Sint8*)"PILTEX.TGA", (void*)dirty_buf, 0);
ztModelRequest((Sint8*)"SHADOW.ZTP", &shadow, true, SORT_CEN, numTex);
	WRAP_NewTexture((Sint8*)"SHADOW.TGA", (void*)dirty_buf);

//Don't change the order, it affects the PCM number. Not a good system ATM.
p64SoundRequest((Sint8*)"LSTEP.PCM", S1536KHZ, 0);
p64SoundRequest((Sint8*)"WND.PCM", S1536KHZ, 1);
p64SoundRequest((Sint8*)"BSTEP.PCM", S1536KHZ, 5);
p64SoundRequest((Sint8*)"CLCK1.PCM", S1536KHZ, 6);
p64SoundRequest((Sint8*)"BTN1.PCM", S1536KHZ, 7);
p64SoundRequest((Sint8*)"CRONCH.PCM", S1536KHZ, 9);
p64SoundRequest((Sint8*)"ALARM.PCM", S1536KHZ, 11);
p64SoundRequest((Sint8*)"WIN.PCM", S1536KHZ, 13);
//Sound RAM is full.
//A different way to load stuff in sound RAM is needed!

p64MapRequest((Sint8*)"00", 0);

}

void	game_frame(void)
{
	slCashPurge();
	update_gamespeed();
	master_draw_stats();
	frame_render_prep();
	
	master_draw(); 

	file_request_loop();
	//No Touch Order -- Affects animations/mechanics
	//player_phys_affect();
	//	mypad();
	//player_collision_test_loop();
	//collide_with_heightmap(&pl_RBB);
	//object_control_loop(you.dispPos);
	
	slSynch();
}

void	my_vlank(void){
	vblank_requirements();
///Watch out for SCSP command overflow.
///Changing the order of these functions may cause that.
	sound_on_channel(pcm_ctrl[1].CH_SND_NUM, 1);
	sound_on_channel(pcm_ctrl[2].CH_SND_NUM, 2);
	sound_on_channel(pcm_ctrl[3].CH_SND_NUM, 3);
	sound_on_channel(pcm_ctrl[4].CH_SND_NUM, 4);
	sound_on_channel(pcm_ctrl[5].CH_SND_NUM, 5);
	sound_on_channel(pcm_ctrl[6].CH_SND_NUM, 6);
	sound_on_channel(pcm_ctrl[7].CH_SND_NUM, 7);
/*
Channel Use Disambiguation
0: Music
1: Step, Jump
2: Gate pass, Gate success, gate fail
3: Item collection, victory
4:
5:
6:
7: Wind
*/
	music_vblIn(6);
}

void	load_in_frame(void){
	do{
	master_file_system(game_frame);
	} while (true);
//	master_file_system(game_frame);
}

void	attributions(void)
{
	load_dsp_prog();
	run_dsp();
	while(dsp_output_addr[0] != -1){} //No purpose.
	fadeOut(0);
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
	slPrint("also the sound driver sucks", slLocate(3, 21));
	slPrint("also 9.2 is slightly slower than 4.0 ;-;", slLocate(0, 22));
	slPrint("PRESS START", slLocate(3, 24));
	fadeIn();
	do{slSynch();}while(!jo_is_pad1_key_pressed(JO_KEY_START));
	jo_clear_screen();
	slBack1ColSet((void*)VDP2_RAMBASE, 0xE726);
}

void	jo_main(void)
{
	jo_core_init(0xE726); //Teal Colour is the value
	//XL2
	slDynamicFrame(ON); //Dynamic framerate
	SynchConst = 2;
//	attributions();
	//
	//Loading Area
	init_lwram();
	active_HWRAM_ptr = (void*)jo_malloc(150 * 1024); //High Work Ram Entity Area 
	dpinit();
	init_render_area();
	initPhys();
	anim_defs();
	init_heightmap();
	slInitSound((unsigned char *)sddrvstsk, sizeof(sddrvstsk), (unsigned char *)map, sizeof(map));
	//The one interrupt that SGL has you register
	slIntFunction(my_vlank);
	//SYS_SETSCUIM(SYS_GETSCUIM & 8192);
	//INT_SetScuFunc(0x4D, rt_fail); //Example of interrupt processing. Check SCU manual. Uses sprite draw end vector.
	//
	fill_obj_list();
	load_test();
	set_camera();
	reset_player();
	load_in_frame();
}

