//
//sound.c
// This file compiled separately.
// Contains game-specific sound & sound management function/information.
//

#include <SL_DEF.H>
#include <SGL.H>
#include <SEGA_GFS.H>

#include "pcmsys.h"
#include "pcmstm.h"

#include "def.h"
#include "sound.h"
#include "render.h"
#include "mloader.h"
#include "draw.h"
#include "mymath.h"

//////////////////////////////////////////////////////////////////////////////
//Sound Numbers
//////////////////////////////////////////////////////////////////////////////
 int snd_dash;
 int snd_lstep;
 int snd_mstep;
 int snd_slideon;
 int snd_wind;
 int snd_bstep;
 int snd_click;
 int snd_button;
 int snd_cronch;
 int snd_alarm;
 int snd_win;
 int snd_freturn;
 int snd_ftake;
 int snd_bwee;
 int snd_smack;
 int snd_boost;
 int snd_khit;
 int snd_clack;
 int snd_close;
 int snd_button2;
 int snd_ffield1;
 int snd_ffield2;
 int snd_ring1;
 int snd_ring2;
 int snd_ring3;
 int snd_ring4;
 int snd_ring5;
 int snd_ring6;
 int snd_ring7;
 Sint8 * stmsnd[64];
 int snd_win;
 int snd_freturn;
 int snd_orchit0;
 int snd_tslow;
 int snd_gpass;
 int snd_yeah;
 int snd_rlap;
 int stm_win = 0;
 int stm_freturn = 1;
 int stm_orchit0 = 2;
 Sint8 stg_mus[3][13];
 int playing_stage_music = NO_STAGE_MUSIC;
 int set_music_track = NO_STAGE_MUSIC;
 char music_volume = 150;
 char music_pan = 0;
 
 // Next steps:
 // Allow a flag to be set which will hold the music change until the entire song has played.
 
 // How do we instance a sound?
 // Realistically, we copy a pcmCtrl to a new pcmCtrl slot.
 // Ideally though we're not reading the pcmCtrl from sound RAM to do that . . . but it's fine.
 // What we also need to do with a given sound instance is to give it a 3D position with which to do 3D sound with.
 // Channel management with instanced sounds? I think all instanced sounds will be set with PCM_PROTECTED.
 // A sound instance poke will be set at vblank before the sound driver is set to run.
 // This will check which instances are still active and which aren't.
 // The master SH2 does not need to do any channel management, it just needs to know which sound instance is in which pcm ctrl slot,
 // and which sound slots are active as sound instances.
 // If a sound instance is not playing, it can be ignored.
 // In other words, pcmCtrl slots past a certain point will be marked as the sound instances. These will be checked and the first one that is inactive (sh2_permit=0) is used.

//If slot inactive, value = 0
//If slot active, value != 0
_sound_instance snds[PCM_CTRL_MAX];


void	active_slot_monitor(void)
{
	//Purpose:
	//Check all pcm_ctrl (above a certain slot) to see if they are active or not
	for(int i = SOUND_INSTANCE_FLOOR; i < PCM_CTRL_MAX; i++)
	{
		snds[i].active = (i > SOUND_INSTANCE_FLOOR) ? m68k_com->pcmCtrl[i].sh2_permit : 1;
	}
	
}

int		get_empty_instance(void)
{
	for(int i = SOUND_INSTANCE_FLOOR; i < PCM_CTRL_MAX; i++)
	{
		if(!snds[i].active) return i;
	}
	return 0;
}

void	empty_all_instances(void)
{
	for(int i = SOUND_INSTANCE_FLOOR; i < PCM_CTRL_MAX; i++)
	{
		m68k_com->pcmCtrl[i].sh2_permit = 0;
	}

}

//Play a (normal) sound instance at the given position 
int		play_sound_instance(int pcm_num, int control_type, int volume_scalar, int * pos)
{
	//Get an empty instance
	int tgt_num = get_empty_instance();
	
	//Set up the pcm_ctrl pointers to copy data
	volatile _PCM_CTRL * src = &m68k_com->pcmCtrl[pcm_num];
	volatile _PCM_CTRL * dst = &m68k_com->pcmCtrl[tgt_num];
	//Copy
	*dst = *src;
	// Get initial distance from viewport to position given
	// (viewport position is a global variable)
	snds[tgt_num].volume_scale = volume_scalar;
	snds[tgt_num].pos[X] = -pos[X];
	snds[tgt_num].pos[Y] = -pos[Y];
	snds[tgt_num].pos[Z] = -pos[Z];
	int dist = unfix_length(snds[tgt_num].pos, viewport_pos)<<16;
	//Distance is then a fixed-point more-or-less linear scale
	//Volume is normally an inverse squared law
	//A meter is more or less 64<<16 units (6400<<16 is 100 meters)
	int volume_scale = fxm(fxdiv(dist,6400<<16), volume_scalar);
	int volume = fxm(255<<16,volume_scale)>>16;
	if(volume > 255) volume = 255;
	volume = 255-volume;
	
	//nbg_sprintf(3, 10, "vol(%i)", volume);

	pcm_play(tgt_num, control_type, volume);
	snds[tgt_num].active = 1;
	return tgt_num;
}

void	operate_stage_music(void)
{
	
	// nbg_sprintf(1, 11, "loops(%i)", stm.times_to_loop);
	// nbg_sprintf(1, 12, "set(%i)",  set_music_track);
	// nbg_sprintf(1, 13, "play(%i)", playing_stage_music);
	
	if(stm.times_to_loop == 0 && !stm.playing && playing_stage_music != NO_STAGE_MUSIC)
	{
		//When a stage music event loops-out to zero, go back to the idle music, and keep playing.
		set_music_track = 0;
		stm.times_to_loop = 1;
	}
	
		if(set_music_track == NO_STAGE_MUSIC && stm.playing)
	{
		stop_pcm_stream();
		playing_stage_music = NO_STAGE_MUSIC;
		stm.times_to_loop = 0;
	} else if(playing_stage_music == set_music_track)
	{
		//This should be a fairly normal case.
		change_pcm_stream_param(music_volume, music_pan);
	} else if(set_music_track != playing_stage_music)
	{
		start_pcm_stream(&stg_mus[set_music_track][0], music_volume);
		playing_stage_music = set_music_track;
	}
	
}
 

