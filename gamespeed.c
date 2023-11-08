//gamespeed.c
//This file is compiled separately.

#include <SL_DEF.H>
#include "def.h"
#include "render.h"
#include "vdp2.h"
#include "mymath.h"

#include "menu.h"

#define GRAPH_X_OFFSET (12)


/** @brief High Free Running Counter Register (FCR), counts up to 255, then iterates FCR Low */
#define FRC_H		(0xfffffe12)
/** @brief Low Free Running Counter Register (FCR), increases every time FCR high reaches 256 */
#define FRC_L		(0xfffffe13)
/** @brief Time Control Register (TCR) */
#define	TCR			(0xfffffe16)
/** @brief System clock register */
#define SysClock	(0x6000324)

int oldtime = 0;
int time_in_seconds = 0;
int delta_time = 0;
int bad_frames = 0;
int time_delta_scale = 0;
int time_fixed_scale = 0;
int framerate = 0;

int * uncache_oldtime;
int * uncache_time;

//Clock speed from framerate based on resolution register and NSTC vs PAL
float         time_get_clockspeed(void)
{
    return (((*(unsigned short *)0x25f80004 & 0x1) == 0x1) ?
             ((*(volatile unsigned int *)(SysClock) == 0) ? (float)0.037470726 : (float)0.035164835) :
             ((*(volatile unsigned int *)(SysClock) == 0) ? (float)0.037210548 : (float)0.03492059));
}

//	Determine if clock is on 1/8, 1/32, or 1/128 of count
unsigned int  time_get_clock_mode(void)
{
    return (8 << ((*(volatile unsigned char *)(TCR) & 3) << 1));
}

	//Returns time within frame in milliseconds
int		get_time_in_frame(void)
{
	*uncache_oldtime = *uncache_time;
	int curFrc = *(volatile unsigned char *)(FRC_H) << 8 | *(volatile unsigned char *)(FRC_L);
	int time_add = (time_get_clockspeed() * curFrc * time_get_clock_mode() / 1000) * 65536.0;
	//time_in_seconds += time_add;
	return (*uncache_time + time_add) - *uncache_oldtime;
}

void                                    fixed_point_time(void)
{
	oldtime = time_in_seconds;
	int curFrc = *(volatile unsigned char *)(FRC_H) << 8 | *(volatile unsigned char *)(FRC_L);
	int time_add = (time_get_clockspeed() * curFrc * time_get_clock_mode() / 1000000) * 65536.0;
	time_in_seconds += time_add;
	delta_time = time_in_seconds - oldtime;
	*(volatile unsigned char *)(FRC_H) = 0;
	*(volatile unsigned char *)(FRC_L) = 0;
}

//borrowed/given by XL2 -- Frame limiter to 30 FPS. EXTREMELY USEFUL.
void	update_gamespeed(void)
{
	
	/////////////////////////////////////////////
	// Timekeeper
	/////////////////////////////////////////////
	uncache_oldtime = &oldtime;
	uncache_oldtime = (int*)((unsigned int)uncache_oldtime | UNCACHE);
	
	uncache_time = &time_in_seconds;
	uncache_time = (int*)((unsigned int)uncache_time | UNCACHE);
	
	int frmrt = delta_time>>6;
	fixed_point_time();
	
    framerate = (frmrt)>>4;
	
    if (framerate <= 0) framerate=1;
    else if (framerate > 5) framerate=5;

	time_delta_scale = fxdiv(65536, delta_time);
	time_fixed_scale = framerate<<16;
	
		
	if(viewInfoTxt == 1)
	{
	nbg_sprintf(1, 3, "(%i) Bad Frames)", bad_frames);
	nbg_sprintf(24, 4, "Fmrt:(%i)", frmrt);
	}
	
	/////////////////////////////////////////////
	// Framegraph
	/////////////////////////////////////////////
	
 	static int lastTimes[66];
	static int time_selector = 0;
	
	lastTimes[time_selector] = frmrt;
	
	//Clean the times so they fit within the graph
	for(int i = 0; i < 66; i++)
	{
	lastTimes[i] = (lastTimes[i] < 0) ? 0 : (lastTimes[i] > 70) ? 70 : lastTimes[i];
	}

	//If the frame-time is too fast or too slow, mark it as a bad frame.
	bad_frames += (frmrt < 30 || frmrt > 35) ? 1 : 0;
	if(time_selector >= 66)
	{
		time_selector = 0;
	}
	time_selector++;

		//Framegraph
	char curLine = lastTimes[time_selector];
	char prevLine = (time_selector < 1) ? lastTimes[65] : lastTimes[time_selector-1];
	char nthLine = (time_selector < 2) ? lastTimes[65] : lastTimes[time_selector-2];
	
	draw_hud_line(time_selector+GRAPH_X_OFFSET, 22, time_selector+GRAPH_X_OFFSET, 5, 7); //(last argument is color)
	draw_hud_line(time_selector+GRAPH_X_OFFSET, 22, time_selector+GRAPH_X_OFFSET, (curLine>>2)+6, 53);
		if(time_selector > 1)
		{
	draw_hud_line((time_selector-1)+GRAPH_X_OFFSET, 22, (time_selector-1)+GRAPH_X_OFFSET, (prevLine>>2)+6, 41);
		}
		if(time_selector > 2)
		{
	draw_hud_line((time_selector-2)+GRAPH_X_OFFSET, 22, (time_selector-2)+GRAPH_X_OFFSET, (nthLine>>2)+6, 23);
		} 
		//

}

