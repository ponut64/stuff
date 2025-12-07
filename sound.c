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
		snds[i].active = m68k_com->pcmCtrl[i].sh2_permit;
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

void	set_sound_instance(_sound_instance * snd, int * volume, int * pan)
{
			static int position_dif[3];
			static int normal_dif[3];
			segment_to_vector(snd->pos, viewport_pos, position_dif);
			
			int vmag = 0;
		
			register int dist = position_dif[X]>>16;
			int accumulator =  dist * dist;
				dist = position_dif[Y]>>16;
				accumulator += dist * dist;
				dist = position_dif[Z]>>16;
				accumulator += dist * dist;
			dist = slSquart(accumulator)<<16;
			vmag = dist;
		
			vmag = fxdiv(1<<16, vmag);
			//Normalize the light vector *properly*
			normal_dif[X] = fxm(vmag, position_dif[X]);
			normal_dif[Y] = fxm(vmag, position_dif[Y]);
			normal_dif[Z] = fxm(vmag, position_dif[Z]);
			
		
			//Distance is then a fixed-point more-or-less linear scale
			//Volume is normally an inverse squared law
			//A meter is more or less 64<<16 units (6400<<16 is 100 meters)
			int distance_scale = fxm(fxdiv(dist,6400<<16), snd->volume_scale);
			int lclv = fxm(255<<16,distance_scale)>>16;
			if(lclv > 255) lclv = 255;
			*volume = 255-lclv;
			
		
			//Dot product = Positive is sound source to the left, Negative is sound source to the right
			int perspective_x[3] = {perspective_root[0][X], perspective_root[1][X], perspective_root[2][X]};
			accumulator = fxdot(normal_dif, perspective_x);

			int lclp = -(accumulator>>12);
			


			
			//Pan is set as a 5-bit value. Bit 4 is a flag which sets whether the sound goes left or right. Bits 3-0 represent a linear pan level.
			//The sound driver nor the linked library sanitize this input, so we must make sure it complies with that standard.
			*pan = (lclp < 0) ? (((-lclp) & 0xF) | 0x10) : lclp & 0xF;
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
	static int volume;
	static int pan;
	
	set_sound_instance(&snds[tgt_num], &volume, &pan);

	pcm_play(tgt_num, control_type, volume);
	m68k_com->pcmCtrl[tgt_num].pan = pan;
	//Set the active status to "2" here. In doing so, the routine processor will not update the sound again this frame.
	snds[tgt_num].active = 2;
	return tgt_num;
}

void	update_3d_sounds(void)
{
	static int volume;
	static int pan;
	for(int i = SOUND_INSTANCE_FLOOR; i < PCM_CTRL_MAX; i++)
	{
		_sound_instance * snd = &snds[i];
		if(snd->active == 1)
		{
			set_sound_instance(snd, &volume, &pan);
			
			//nbg_sprintf_decimal(18, 3+((i-SOUND_INSTANCE_FLOOR)*3), snd->pos[X]);
			//nbg_sprintf_decimal(18, 4+((i-SOUND_INSTANCE_FLOOR)*3), snd->pos[Y]);
			//nbg_sprintf_decimal(18, 5+((i-SOUND_INSTANCE_FLOOR)*3), snd->pos[Z]);
			//nbg_sprintf(3, 6+(i-SOUND_INSTANCE_FLOOR), "pan(%i)", pan);

			pcm_parameter_change(i, volume, pan);
		}
		snd->active = (snd->active == 2) ? 1 : snd->active;
	}

	
	
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
 

