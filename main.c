
//
// It might be time to add a console / event-viewer to report goings-on of the game in text.
// Reason: One way to implement subtitles.
// Of course an easier alternative is just implement a subtitle system,
// and then use it for event reporting.
//
/**

jackie's birthday is july 1st

Sector-based engine has been implemented in its first steps.

I now want to move on and optimize the number of planes put in each sector.
What I want is to have a "First Pass Subdivision", where a polygon is subdivided by rules,
but WITHOUT a UV coordinate respected to it; the polygons subdivided from the first are all "tiles" of the main polygon.

I have the first-pass of extra large plane subdivision.

However, it's amounting to a pretty high vertex count.
I should investigate some way to optimize it.

I have decided: there is no way an FPS game is happening with 100% real-time subdivision.
What I need to do is precalculate the subdivision on a sector-by-sector basis.
In this way, the mesh loaded from RAM contains the plane data of the level, raw.
The game engine then compiles that raw plane data into sectors and then subdivides the planes of the sector
according to the number of subdivisions that it must receive.
What I will try first is subdividing down to tiles only first, then if that works well, try subdividing all the way down.
That will burn a lot of memory but I do have a lot of free LWRAM.
This also can be packed efficiently; many tiles and polygons share normals and attributes.

Well, what exactly does precalculating the subdivision save me?
Basically I would be using the same process as currently, and THEN have to matrix transform the subdivisions.
There would be a lot of set-up and duplicated work....
One thing I don't do in the subdivision routine is eliminate duplicate vertices in the output;
it's more work to do that than not.
In case of precalculation, you have the time to invest into removing duplicates; that can reduce the workload.

An Alternative Optimization:
Portals.
If you cannot see the portal that borders a particular sector, then you cannot see that sector;
that sector should not be drawn if you aren't in it.
This can help isolate the problem to the current sector, at least.

Primary issue:
Tile subdivision creates a tremendous amount of duplicate vertices.

I need to precalculate a table, per plane, which tags vertices created by each subdivision.
uhhh.... how do i do that
what follows from each plane is an ID for each tile of the plane
from each subdivision of each tile, an ID must be created.
in this situation, every possible polygon has a unique ID
How will I structure that?
Some rules that it must adhere to:
1. The IDs must be unique and consistent across the entire plane
2. The IDs must identify which IDs are created when that ID is subdivided, according to its rule.
Tag list is based on the tile ID and the subdivisions proceeding from the tile.
If +++, the tag list is:
tile+1
	tile+2
		tile+3
		tile+4
		tile+5
		tile+6
	tile+7
		tile+8
		tile+9
		..



And then, in the subdivision system, I need to check if that tag is already alive in a fixed-size list
before creating a vertex.

so, I need to check... exactly how many duplicates are there within a plane?
i'm so close to solving it but I would like to know...

when I finish a performant & functional version of the sector-based engine, I want to make a video explaining it.
I know that'll be a week time-sink to plan the script and shoot, but it might be helpful to Emerald.

I also need to implement point lights / dynamic lights back in.

I also need to test \ implement actors in the sector system.


i would like to note that with my new knowledge of bitfields, so much code can be more optimally rewritten


**/
//
// Compilation updated to use latest version of Jo Engine standard compiler.
//
//
#include <sl_def.h>
#include <SEGA_INT.H>
#include <SEGA_GFS.H>

//Outstanding code contributions from XL2 
//
#include "def.h"
#include "mymath.h"
#include "render.h"
#include "collision.h"
#include "control.h"
#include "vdp2.h"
#include "physobjet.h"
#include "tga.h"
#include "ldata.h"
#include "input.h"
#include "pcmsys.h"
#include "pcmstm.h"
#include "draw.h"
#include "anidefs.h"
#include "gamespeed.h"
#include "menu.h"
#include "sound.h"
#include "particle.h"
//
#include "lwram.c"
//
//
#include "dspm.h"
#include <stdio.h>
//
// Game data //
//Be very careful with uninitialized pointers. [In other words, INITIALIZE POINTERS!]
// SGL Work Area is using the last 200KB of High Work RAM. The game binary is using about 250KB.
unsigned char hwram_model_data[HWRAM_MODEL_DATA_HEAP_SIZE];
void * HWRAM_ldptr;
void * HWRAM_hldptr;
void * sectorPolygonStack;

//short * sine_table;

//A zero vector to be used when you want zero.
POINT zPt = {0, 0, 0};
extern Sint8 SynchConst; //SGL System Variable

unsigned char * dirty_buf;
unsigned char * dirtier_buf;
void * currentAddress;

volatile Uint32 * scuireg = (Uint32*)0x25FE00A4;
volatile Uint32 * scuimask = (Uint32*)0x25FE00A0;
volatile Uint32 * scudmareg =  (Uint32*)0x25FE007C;

int game_set_res = TV_320x240;

//////////////////////////////////////////////////////////////////////////////
int flagIconTexno = 0;
//////////////////////////////////////////////////////////////////////////////
// Player data struct
//////////////////////////////////////////////////////////////////////////////
	_player you;

void	p64MapRequest(short levelNo)
{
	Sint8 ldat_name[13];
	char the_number[3] = {'0', '0', '0'};
	//Why three?
	//Strings need a terminating character.
	//String "99" is actually binary #'9','9',0 (literal zero).
	int num_char = sprintf(the_number, "%i", levelNo);
	if(num_char == 1)
	{
		the_number[1] = the_number[0];
		the_number[0] = '0';
	}
///Fill out the request.
					
 	ldat_name[0] = 'L';
	ldat_name[1] = 'E';
	ldat_name[2] = 'V';
	ldat_name[3] = 'E';
	ldat_name[4] = 'L';
	ldat_name[5] = the_number[0];
	ldat_name[6] = the_number[1];
	ldat_name[7] = '.';
	ldat_name[8] = 'L';
	ldat_name[9] = 'D';
	ldat_name[10] = 'S';
	
	set_music_track = NO_STAGE_MUSIC; //Clear stage music, preparing to change music of course.
	new_file_request(ldat_name, dirty_buf, process_binary_ldata, 0);
	ldata_ready = false;


}


//Loading. Check msfs.c and mloader c/h
void	load_test(void)
{

	get_file_in_memory((Sint8*)"NBG_PAL.TGA", (void*)dirty_buf);
	set_tga_to_nbg1_palette((void*)dirty_buf);
	
	//Uint32 fid = GFS_NameToId((Sint8*)"SPLASH_4.TGA");
	//set_8bpp_tga_to_nbg0_image(fid, dirty_buf);
	
	WRAP_NewPalette((Sint8*)"TADA.TGA", (void*)dirty_buf);
	baseAsciiTexno = numTex;
	sprAsciiHeight = 12;
	sprAsciiWidth = WRAP_NewTable((Sint8*)"FONT2.TGA", dirty_buf, sprAsciiHeight); //last argument, tex height
	//
	HWRAM_ldptr = (void *)(&hwram_model_data[0]);
	////////////////////////////////////////////////
	// REMINDER: All file names must comply with the 8.3 standard.
	// File extensions can be no longer than 3 letters.
	// File names can be no longer than 8 letters.
	// The total length is thusly 12 characters (as there is a period).
	////////////////////////////////////////////////
	nbg_sprintf(0,0, "Loading ...");
	
	snd_win = load_adx((Sint8*)"WIN.ADX");
	snd_yeah = load_adx((Sint8*)"YEAH.ADX");
	snd_freturn = load_adx((Sint8*)"FRETURN.ADX");
	snd_orchit0 = load_adx((Sint8*)"ORCHIT0.ADX");
	snd_ftake = load_adx((Sint8*)"FLAG.ADX");
	snd_tslow = load_adx((Sint8*)"TSLOW.ADX");
	snd_gpass = load_adx((Sint8*)"GPASS.ADX");
	snd_rlap = load_adx((Sint8*)"RLAP.ADX");
	snd_bwee = load_8bit_pcm((Sint8*)"BWEE.PCM", 15360);
	snd_lstep = load_8bit_pcm((Sint8*)"LSTEP.PCM", 15360);
	snd_mstep = load_8bit_pcm((Sint8*)"MSTEP.PCM", 15360);
	snd_slideon = load_8bit_pcm((Sint8*)"SLIDEON.PCM", 15360);
	snd_wind = load_8bit_pcm((Sint8*)"WND.PCM", 15360);
	snd_bstep = load_8bit_pcm((Sint8*)"STEP.PCM", 15360);
	snd_cronch = load_8bit_pcm((Sint8*)"CRONCH.PCM", 15360);
	snd_alarm = load_8bit_pcm((Sint8*)"ALARM.PCM", 15360);
	snd_smack = load_8bit_pcm((Sint8*)"MSMACK.PCM", 15360);
	snd_boost = load_8bit_pcm((Sint8*)"BOOST.PCM", 15360);
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
	sparkTexno = numTex;
	WRAP_NewTexture((Sint8*)"SPARK.TGA", (void*)dirty_buf);
	auraTexno = numTex;
	WRAP_NewTexture((Sint8*)"AURA.TGA", (void*)dirty_buf);
	puffTexno = numTex;
	WRAP_NewTexture((Sint8*)"PUFF.TGA", (void*)dirty_buf);
	controlImgTexno = numTex;
	WRAP_NewTexture((Sint8*)"CONTROL2.TGA", (void*)dirty_buf);
	slowImgTexno = numTex;
	WRAP_NewTexture((Sint8*)"SLOW.TGA", (void*)dirty_buf);
	parImgTexno = numTex;
	WRAP_NewTexture((Sint8*)"PAR.TGA", (void*)dirty_buf);
	goldImgTexno = numTex;
	WRAP_NewTexture((Sint8*)"GOLD.TGA", (void*)dirty_buf);

	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"SHADOW.GVP", 		HWRAM_ldptr, &shadow,	    GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"TGUN.GVP",			HWRAM_ldptr, &entities[1], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"BOX.GVP",			HWRAM_ldptr, &entities[2], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"TEST00.GVP",		HWRAM_ldptr, &entities[0], GV_SORT_CEN, MODEL_TYPE_TPACK, NULL);
		
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"STARSTAN.GVP",		HWRAM_ldptr, &entities[11], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"TMAP2.GVP",		HWRAM_ldptr, &entities[12], GV_SORT_CEN, MODEL_TYPE_SECTORED, &entities[0]);
	HWRAM_ldptr = buildAdjacentSectorList(12, HWRAM_ldptr);
	
	int ptr_begin = (unsigned int)HWRAM_ldptr;
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		HWRAM_ldptr = preprocess_planes_to_tiles_for_sector(&sectors[i], HWRAM_ldptr);
	}
	ptr_begin -= (unsigned int)HWRAM_ldptr;
	
	nbg_sprintf(5, 10, "sz(%i)", ptr_begin);
	HWRAM_hldptr = HWRAM_ldptr;

	while(is_key_up(DIGI_START))
	{
		
	};

	init_pathing_system();
	p64MapRequest(0);
	//
}

void	game_frame(void)
{
	
	// nbg_sprintf(2, 9, "ax(%i)", apd1.ax);
	// nbg_sprintf(2, 10, "ay(%i)", apd1.ay);
	// nbg_sprintf(2, 11, "lt(%i)", apd1.lta);
	// nbg_sprintf(2, 12, "rt(%i)", apd1.rta);

	slCashPurge();
	
	update_gamespeed();
	maintRand();
	master_draw_stats();
	frame_render_prep();
	master_draw(); 
	operate_stage_music();
	reset_pad(&pad1);

	//ABC+Start Exit Condition
	if(is_key_down(DIGI_START) && is_key_down(DIGI_A) && is_key_down(DIGI_B) && is_key_down(DIGI_C))
	{
		SYS_Exit(0);
	}

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
	
	//testing_level_data((Sint8*)"LIST0.REM", (void*)dirty_buf);
	slZoomNbg0(65536, 65536);
	load_test();
	
	nbg_clear_text();

	//For NBG0, we zoom it.
	//This zooms the screen such that a 128x128 area is displayed over the 352x224 screen.
	//This is for th zoom on in-game backgrounds. Before this point, the zoom is set for the splash screen.
	slZoomNbg0(23831, 37449);
}

int		validation_escape(void)
{
	static int first_stroke = 0;
	static int escaped = 0;
	if(is_key_pressed(DIGI_L) && is_key_pressed(DIGI_R))
	{
		first_stroke = 1;
	}
	
	if(first_stroke == 1 && is_key_pressed(DIGI_DOWN) && is_key_pressed(DIGI_START) && is_key_up(DIGI_L) && is_key_up(DIGI_R))
	{
		escaped = 1;
	}
	return escaped;
}

void	hardware_validation(void)
{
	load_drv(ADX_MASTER_1536); 
	sdrv_vblank_rq();
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
	init_vdp2(0xE726);
	init_render_area(90 * 182);
	initPhys();
	init_box_handling();
	//The one interrupt that SGL has you register
	slIntFunction(my_vlank);
	//
	//
	init_dsp_programs();
	//Sound Driver & Hardware/Emulator Validation
	hardware_validation();
	//
	
	fill_obj_list();
	init_entity_list();
	
	//load_test();
	attributions();
	init_particle();
	init_hud_events();
	
	set_camera();
	reset_player();
	anim_defs();
	add_adx_front_buffer(15360);
	add_adx_back_buffer(dirty_buf);
	pcm_stream_init(30720, PCM_TYPE_8BIT);
	pcm_stream_host(game_frame);
	return 1;
}

