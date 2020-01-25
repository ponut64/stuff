#ifndef __MSFS_H__
# define __MSFS_H__

#include "ZT/ZT_COMMON.H"
#include "ZT/ZT_LOAD_MODEL.H"
#include "hmap.h"
#include "ldata.h"
#include <jo/jo.h>

//Playback buffers start 352 KB into sound RAM. From 40Kb into sound RAM to 352 KB, area is OK for storing sound.
//Each buffer is 8 sectors / 16 KB
//This is sound RAM, addressable by the MC68EC000 / SCSP
/**MUSIC BUFFER REGION 160KB / 80 SECTORS / 5 * 32768 **/
#define PCMBUF1 (PCMEND - 32768)
#define PCMBUF2 (PCMBUF1 - 32768)
#define PCMBUF3 (PCMBUF2 - 32768)
#define PCMBUF4	(PCMBUF3 - 32768)
#define PCMBUF5	(PCMBUF4 - 32768)
/**END MUSIC BUFFER REGION**/

#define MUS_BUFCNT (5)

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

extern snd_ring		music_buf[MUS_BUFCNT];
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
void	music_vblIn(Uint8 vol);
//
void	ztModelRequest(Sint8 * name, entity_t * model, char workRAM, char sortType, short base_texture);
//
void	file_request_loop(void);
void	master_file_system(void(*game_code)(void));
//
void	p64MapRequest(Sint8 * levelNo, Uint8 mapNum);

#endif

