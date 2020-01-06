#ifndef __MSFS_H__
# define __MSFS_H__

#include "ZT/ZT_COMMON.H"
#include "ZT/ZT_LOAD_MODEL.H"
#include "hmap.h"
#include "ldata.h"
#include <jo/jo.h>

//No Touchy Sound Ram Start!
#define SNDRAM  (631242752)
//
//Also the end of sound RAM
#define PCMEND	(631767039)
//
//Playback buffers start 352 KB into sound RAM. From 40Kb into sound RAM to 352 KB, area is OK for storing sound.
//Each buffer is 8 sectors / 16 KB
//This is sound RAM, addressable by the MC68EC000 / SCSP
/**MUSIC BUFFER REGION 160KB / 80 SECTORS / 5 * 32768 **/
#define PCMBUF1 (SNDRAM + 360448)
#define PCMBUF2 (PCMBUF1 + 32768)
#define PCMBUF3 (PCMBUF2 + 32768)
#define PCMBUF4	(PCMBUF3 + 32768)
#define PCMBUF5 (PCMBUF4 + 32768)
/**END MUSIC BUFFER REGION**/
///Each individual PCM buffer is 16384 bytes / 8 sectors for a total of 304 KB / 152 sectors from 19 buffers total.
///Each buffer can store 8 frames of 30720Hz single-channel audio or 16 frames of 15360Hz single-channel audio.
#define PCMBUF6		(SNDRAM + 40960)
#define PCMBUF7		(PCMBUF6 + 16384)
#define PCMBUF8		(PCMBUF7 + 16384)
#define PCMBUF9		(PCMBUF8 + 16384)
#define PCMBUF10	(PCMBUF9 + 16384)
#define PCMBUF11	(PCMBUF10 + 16384)
#define PCMBUF12	(PCMBUF11 + 16384)
#define PCMBUF13	(PCMBUF12 + 16384)
#define PCMBUF14	(PCMBUF13 + 16384)
#define PCMBUF15	(PCMBUF14 + 16384)
#define PCMBUF16	(PCMBUF15 + 16384)
#define PCMBUF17	(PCMBUF16 + 16384)
#define PCMBUF18	(PCMBUF17 + 16384)
#define PCMBUF19	(PCMBUF18 + 16384)
#define PCMBUF20	(PCMBUF19 + 16384)
#define PCMBUF21	(PCMBUF20 + 16384)
#define PCMBUF22	(PCMBUF21 + 16384)
#define PCMBUF23	(PCMBUF22 + 16384)
#define PCMBUF24	(PCMBUF23 + 16384)

#define MAP_TO_SCSP(sh2map_snd_adr) ((sh2map_snd_adr - SNDRAM)>>4)

//THIS IS TESTED
#define S3072KHZ	(31122)
#define S3072TIMER	(1024)
#define	S3072PALT	(1233)
#define S1536KHZ	(29074)
#define S1536TIMER	(512)
#define S1536PALT	(633)
//

typedef struct{
	void* rd_pcmbuf;
	int play_pcmbuf;
} snd_ring;

/**
PCM DATA STRUCTURE
Top lines: GFS Information
file_done : if there is actually data in this pcm data.
active : if this is actively being filled with data.
dstAddress : where the data is going to go.
fid : file ID. Use GFS_NameToId((Sint8*)name) on your file. Changing folders and such is possible but not covered here.

pitchword : The bitrate, converted into a pitch word for the sound CPU.
playsize : the size of data to be played back. NOTE: There are issues with play-sizes that run over 255 frames. Limitation of sound CPU? Dunno.
loctbl : [Archaic] Represents the order in your PCM buffer segments. A suggestion, that even I may not follow.
segments : the number of PCM buffer segments the file consumes. [Big files don't play that well..]
playtimer : an active timer of how long this PCM sound effect has been played.
frames : the number of frames this sound needs to play. The math is strange. The base factor is 1 frame per 16 KB, as derived from the reading process [8 sectors].
^< Explanation: For 15360 bitrates, 15360 bits * 16 bit PCM = 245760 raw bitrate / 8 bits = 30720 bytes/s / 30 = 1024 bytes per frame.
^< Explanation: For 30720 bitrates, 30720 bits * 16 bit PCM = 491520 raw bitrate / 8 bits = 61440 bytes/s / 30 = 2048 bytes per frame.
For 15.360KHz playback, 1KB is played per frame. For 30.720KHz playback, 2KB is played per frame.
**/
typedef struct{
	char	file_done;
	char	active; //File System Activity
	int	dstAddress;
	Sint8*	fid;
	
	short	pitchword;
	int	playsize;
	short	loctbl;
	short	segments;
	int	offset;
	short	frames;
} p64pcm;

typedef struct{
unsigned char		CH_SND_NUM;
unsigned char		ch_on;
unsigned char		ready;
unsigned char		busy;
unsigned char		ready_play;
unsigned char		volpan;
int					playtimer;
} pcmCtrlTbl;

typedef struct
{
	char	file_done;
	char	useHiMem;
	char	active; //File System Activity
	void *	dstAddress;
	entity_t * tmodel;
	Sint8 * filename;
}	request;

typedef struct
{
	char file_done;
	char active;
	Sint8 * fid;
	char type;
}	spr_rq;

extern void * active_LWRAM_ptr;
extern void * active_HWRAM_ptr;

extern request	requests[19];
extern spr_rq	tga_request[19];
extern p64pcm *	pcm_slot; //In LWRAM // 
extern pcmCtrlTbl * pcm_ctrl; //In LWRAM // 

extern snd_ring		music_buf[5];
extern Sint8*		music;
extern int			musicPitch;
extern int			musicTimer;
extern int			buf_pos;
extern void*		curpcmbuf;
extern int			buffers_filled;
extern int			fetch_timer;
extern int			music_frames;
extern Bool			m_trig;
extern Bool			chg_music;
//
void	p64SoundRequest(Sint8* name, Sint32 bitrate, Uint8 destBufSeg);
void	sound_on_channel(Uint8 sound_number, Uint8 channel);
void	music_vblIn(Uint8 vol);
void	trigger_sound(Uint8 channel, Uint8 sound_number, Uint8 vp_word);
void	stop_sound(Uint8 channel);
//
void	ztModelRequest(Sint8 * name, entity_t * model, char workRAM, char sortType, short base_texture);
//
void	file_request_loop(void);
void	master_file_system(void(*game_code)(void));
//
void	p64MapRequest(Sint8 * levelNo, Uint8 mapNum);

#endif

