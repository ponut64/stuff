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
 char music_volume = 3;
 char music_pan = 0;
 
 // Next steps:
 // Allow a flag to be set which will hold the music change until the entire song has played.

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
 

