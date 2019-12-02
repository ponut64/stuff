//Uses Jo Engine, copyright Johannes Fetz
//THIS SOFTWARE, ASIDE FROM THE ASSOCIATED LIBRARIES AND CONTRIBUTIONS USED TO CREATE IT, IS PUBLIC DOMAIN.
/**
In Loving Memory of Jeffrey Neil Lindamood,
my father. 1962 - 2019.
And Bonnie K Caulder Lindamood,
my grandmother. 1937 - 2019.
I am sorry for the pain you had to go through.
**/
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
//
#include <sddrvs.dat>
const char map[] = {0xff, 0xff, 0xff, 0xff}; //For sound driver

//
// Game data //
//Be very careful with uninitialized pointers. [In other words, INITIALIZE POINTERS!]
//Discovery: the MC68EC00 in the Saturn is a .. 68k. I tell people its 32-bit, but.. is it really? no, dont think so
//
FIXED outTime = 0;
int lasttime = 0;
Sint32 framerate;
Sint8 SynchConst;
//A zero vector to be used when you want zero.
POINT zPt = {0, 0, 0};

volatile Uint32 * scuireg = (Uint32*)0x25FE00A4;
volatile Uint32 * scuimask = (Uint32*)0x25FE00A0;
volatile Uint32 * scudmareg =  (Uint32*)0x25FE007C;

void	dpinit(void)
{
	init_vdp2();
	initCamera(&you.rcam);
}

//borrowed/given by XL2 -- Frame limiter to 30 FPS. EXTREMELY USEFUL.
void	update_gamespeed()
{
//	static int curtime = 0;
    outTime = jo_get_ticks(); //outTime is global variable

	int frmrt = (outTime-lasttime);
	
	static Uint8 lastTimes[100];
	static Uint8 time_selector = 0;
	static Uint8 worst_frame = 0;
	static Uint16 bad_times = 0;
	lastTimes[time_selector] = frmrt;
	time_selector++;
	if(time_selector > 100){
		time_selector = 0;
		worst_frame = 0;
		for(Uint8 c =0; c < 100; c++) lastTimes[c] = 0;
	}
	for(Uint8 n = 0; n < 100; n++){
		worst_frame = (lastTimes[n] > worst_frame) ? lastTimes[n] : worst_frame;
	}
	if(frmrt > 40) bad_times++;
	
    framerate = (frmrt)>>4;
    lasttime = outTime;
	
    if (framerate <= 0) framerate=1;
    else if (framerate > 5) framerate=5;
	jo_printf(0, 2, "x(%i)x", frmrt);
	jo_printf(8, 3, "1/100 Low:(%i)", worst_frame);
	jo_printf(8, 2, "Bad Frames:(%i)", bad_times);
	slPrintFX(outTime, slLocate(26, 2));
}

//Loading. Check msfs.c and ZT_LOADMODEL.h for more information.
void	load_test(void)
{
	//Next up: TGA file system handler?
	WRAP_NewPalette((Sint8*)"TADA.TGA", (void*)currentAddress);
	WRAP_NewTable((Sint8*)"DIR0.TGA", (void*)currentAddress, 0);
	WRAP_NewTable((Sint8*)"DIR1.TGA", (void*)currentAddress, 0);
	WRAP_NewTable((Sint8*)"DIR2.TGA", (void*)currentAddress, 0);
	WRAP_NewTable((Sint8*)"DIR3.TGA", (void*)currentAddress, 0);
	WRAP_NewTable((Sint8*)"DIR4.TGA", (void*)currentAddress, 0);
	//End tex 35
	WRAP_NewTable((Sint8*)"PONTX.TGA", (void*)currentAddress, 0);
	WRAP_NewTexture((Sint8*)"LEYE.TGA", (void*)currentAddress);
	WRAP_NewTexture((Sint8*)"REYE.TGA", (void*)currentAddress);
	//End Tex 42
	WRAP_NewTexture((Sint8*)"SHADOW.TGA", (void*)currentAddress);
	
ztModelRequest((Sint8*)"BPONY.ZTP", &pl_model, true, SORT_CEN, 35);
ztModelRequest((Sint8*)"SHADOW.ZTP", &shadow, false, SORT_CEN, 42);
ztModelRequest((Sint8*)"BOX.ZTP",  &entities[0], false, SORT_MAX, 42);
p64SoundRequest((Sint8*)"LSTEP.PCM", S1536KHZ, 2);
p64SoundRequest((Sint8*)"WND.PCM", S1536KHZ, 3);
p64SoundRequest((Sint8*)"BSTEP.PCM", S1536KHZ, 1);

p64MapRequest((Sint8*)"MUPART.PGM", 0);

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
	player_phys_affect();
		mypad();
	player_collision_test_loop();
	collide_with_heightmap(&pl_RBB);
	object_control_loop(you.dispPos);
	
	slSynch();
}

void	my_vlank(void){
	vblank_requirements();
///Watch out for SCSP command overflow.
///Changing the order of these functions may cause that.
	sound_on_channel(CH_SND_NUM[1], 1);
	sound_on_channel(CH_SND_NUM[2], 2);
	sound_on_channel(CH_SND_NUM[3], 3);
	sound_on_channel(CH_SND_NUM[4], 4);
	sound_on_channel(CH_SND_NUM[5], 5);
	sound_on_channel(CH_SND_NUM[6], 6);
	sound_on_channel(CH_SND_NUM[7], 7);
	music_vblIn(6);
}

void	load_in_frame(void){
	do{
	master_file_system(game_frame);
	} while (true);
//	master_file_system(game_frame);
}

void	jo_main(void)
{
	jo_core_init(0xE726); //Teal Color is the value
	//XL2
	slDynamicFrame(ON); //Dynamic framerate
    SynchConst=(Sint8)2;  //Framerate control. 1/60 = 60 FPS, 2/60 = 30 FPS, etc.
    framerate=2;
	//
	//Loading Area
	active_HWRAM_ptr = (void*)jo_malloc(50 * 2048); //High Work Ram Entity Area [100KB]
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
	load_test();
	declarations();
	set_camera();
	reset_player();
	load_in_frame();
}

