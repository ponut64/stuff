#pragma once

#define	NO_STAGE_MUSIC	(-1)

//////////////////////////////////////////////////////////////////////////////
//Sound Numbers
//////////////////////////////////////////////////////////////////////////////
extern int snd_dash;
extern int snd_lstep;
extern int snd_mstep;
extern int snd_slideon;
extern int snd_wind;
extern int snd_bstep;
extern int snd_click;
extern int snd_button;
extern int snd_cronch;
extern int snd_alarm;
extern int snd_win;
extern int snd_freturn;
extern int snd_ftake;
extern int snd_bwee;
extern int snd_smack;
extern int snd_boost;
extern int snd_khit;
extern int snd_clack;
extern int snd_close;
extern int snd_button2;
extern int snd_ffield1;
extern int snd_ffield2;
extern int snd_ring1;
extern int snd_ring2;
extern int snd_ring3;
extern int snd_ring4;
extern int snd_ring5;
extern int snd_ring6;
extern int snd_ring7;
extern Sint8 * stmsnd[64];
extern int snd_win;
extern int snd_freturn;
extern int snd_orchit0;
extern int snd_tslow;
extern int snd_gpass;
extern int snd_yeah;
extern int snd_rlap;
extern int stm_win;
extern int stm_freturn;
extern int stm_orchit0;
extern Sint8 stg_mus[3][13];
extern int playing_stage_music;
extern int set_music_track;
extern char music_volume;
extern char music_pan;

void	operate_stage_music(void);

//Sound Instancing Controls
#define SOUND_INSTANCE_FLOOR (64)

typedef struct {
	int pos[3];
	int volume_scale;
	int active;
} _sound_instance;

extern _sound_instance snds[PCM_CTRL_MAX];

void	active_slot_monitor(void);
int		play_sound_instance(int pcm_num, int control_type, int volume_scalar, int * pos);




