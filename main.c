
//
// It might be time to add a console / event-viewer to report goings-on of the game in text.
// Reason: One way to implement subtitles.
// Of course an easier alternative is just implement a subtitle system,
// and then use it for event reporting.
//
/**

What's on my development iternerary?

Actionable plan:
Actors will also seek out a mover trigger zone if they hit a mover that blocks their path, then continue as normal.

Well, this is a more complex behavior. Only certain actor types should seek out a button or mover trigger.
But more or less it should be in the game.

-> I think the engine can handle changing level/texture data.
Other asset data may need to be assessed when assets are available to fill the spots; otherwise, a standard asset plan is used for all levels.
(To me, this makes sense ... for now)

-> Actor off-screen navigation
	- Need to implement a way for actors to move in unloaded sectors.
	I might just put them on a rail towards the next navigation node and if they spawn in a different or invalid sector, teleport them to node
	
-> Actor Implementations
	Some things to figure out with actor navigation.
	1 - Actor can easily get hung up on a wall corner. Need to find a way to detect this problem early. (probably trigger a pathing exception on X axis collision)
	2 - Actor still has animation issues when changing poses.
	3 - A momentary loss of vision will re-set the aggro chain. A timer should be set where the actor is still ready to pursue when the player is visible next.
	4 - Actor can push the player. That's all well and good, but they can push the player into oblivion. So the player needs to push back.
	Likely need to put a protection in the player's location tracking code to catch wall collisions and count time after a wall collision.
	If a wall collision was detected recently (i.e. past 5 frames), we probably want to set the player back to their previous in-sector location.

-> Performance consideration
	Animated entities are best animated when no one is thrashing the bus.
	Because applying the keyframe to the mesh permanently alters it (i.e. no copy is made),
	animation instancing is used to keep track of multiple animations --
	making it difficult for another CPU to assist in the animation processing pipeline.
	A few options are plausible.
	1 - Use memory allocations dynamically to create copies of the mesh to animate for each respective instance.
	2 - Give CPU1 exclusive processing of the animated entities, but re-allocate drawing of sectors to CPU0.
	Further testing of the quality of existing animation systems should be done.

Roadmap to playable game:
1 - > Complete simple actor implementations
2 - > Start player weapon implementations
3 - > Test/implement multiple actor simultaneous
4 - > Start working on actor <-> player interactions (because the player needs to be able to shoot first)
5 - > Start testing actor <-> actor interactions
6 - > Implement item <-> player interactions
7 - > Implement item <-> actor interactions
8 - > the list only goes on (like give stuff particles)

Mechanical concept:
A sound effect (always loaded in RAM) is played when the player is spotted by enemies once.
It will not play again until enemies that have spotted the player are either unalerted or destroyed.
This is to play on top of any callout sounds that enemies may make to report to other AI.

jump into abyss is next level door
can hide new areas off to the side of these!

water? its helpful in level design...

weapon idea:
chakra golden muzzleloader pistol
autoaim instakill weapon, but long reload
magical bone weapon?

-> Auto-movers
e.g. environmental geometry which automatically moves in a certain way
this is usually rotation, but it could be translation too
automatically triggering movers

special note:
pulping enemies with a single shot is very satisfying
i think the dualie should be able to do that
and i need to figure out a pulp animation

start the game with something artistic
a gray grass field with no enemies or items
and relaxing synths
then after a few minutes, the music shifts, and the palette turns back to color

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

//A zero vector to be used when you want zero.
POINT zPt = {0, 0, 0};
extern Sint8 SynchConst; //SGL System Variable

unsigned char * dirty_buf;
unsigned char * dirtier_buf;

int * zTable = (int *)NULL;

void * currentAddress;

volatile Uint32 * scuireg = (Uint32*)0x25FE00A4;
volatile Uint32 * scuimask = (Uint32*)0x25FE00A0;
volatile Uint32 * scudmareg =  (Uint32*)0x25FE007C;

int game_set_res = TV_320x240;

int levelPos[3];

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
	ldata_ready = false;
	new_file_request(ldat_name, dirty_buf, process_binary_ldata, HANDLE_FILE_ASAP);
	//fetch_and_load_leveldata(ldat_name);

	
}

void	game_frame(void)
{
	*masterIsDoneDrawing = 0;
	update_gamespeed();
	master_draw_stats();
	frame_render_prep();
	slSlaveFunc(scene_draw, 0); //Get SSH2 busy with its drawing stack ASAP
	slCashPurge();
	sector_vertex_remittance();
	
	maintRand();
	if(!you.inMenu)
	{
		master_draw(); 
	} else {
		start_menu();
		*masterIsDoneDrawing = 1;
	}
	operate_stage_music();
	reset_pad(&pad1);

	//ABC+Start Exit Condition
	if(is_key_down(DIGI_START) && is_key_down(DIGI_A) && is_key_down(DIGI_B) && is_key_down(DIGI_C))
	{
		SYS_Exit(0);
	}
}


//Loading. Check msfs.c and mloader c/h
void	load_test(void)
{
	int fid = GFS_NameToId((Sint8*)"NBG_PAL.TGA");
	get_file_in_memory(fid, (void*)dirty_buf);
	set_tga_to_nbg2_palette((void*)dirty_buf);

	
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


	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"SHADOW.GVP", 		HWRAM_ldptr, &shadow,	    GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"NME1.GVP",			HWRAM_ldptr, &entities[3], GV_SORT_CEN, MODEL_TYPE_ANIMATED, NULL);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"BOX.GVP",			HWRAM_ldptr, &entities[2], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"BTNSTN.GVP",		HWRAM_ldptr, &entities[1], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL);
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"TEST00.GVP",		HWRAM_ldptr, &entities[0], GV_SORT_CEN, MODEL_TYPE_TPACK, NULL);
		
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"STARSTAN.GVP",		HWRAM_ldptr, &entities[11], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"STRAN.GVP",		HWRAM_ldptr, &entities[15], GV_SORT_CEN, MODEL_TYPE_NORMAL, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"TMAP2.GVP",		HWRAM_ldptr, &entities[WORLD_ENTITY_ID], 0, MODEL_TYPE_SECTORED, &entities[0]);
	nbg_sprintf(1, 7, "Sectors complete!");
	// while(1)
	// {
		// if(is_key_pressed(DIGI_START)) break;
	// }


	HWRAM_hldptr = HWRAM_ldptr;

	init_pathing_system();
	
	//i need to figure out why they need to be loaded in order?
	load_viewmodel_to_slot(&shorty_shotgun_vm, 1);
	load_viewmodel_to_slot(&lever_pistol_vm, 0);
	
	p64MapRequest(0);
	//
}


void	my_vlank(void)
{
	vblank_requirements();
	operate_digital_pad1();
	active_slot_monitor();
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
	init_vdp2(0xE726);
	init_render_area(90 * 182);
	init_lwram();
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
	//(this has to be after all CD initialization)
	initialize_viewmodel_data();
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

