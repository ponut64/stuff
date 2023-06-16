//gamespeed.c
//This file is compiled separately.

#include <SL_DEF.H>
#include "def.h"
#include "render.h"
#include "vdp2.h"
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
	int frmrt = delta_time>>6;
	fixed_point_time();
	
 	static int lastTimes[66];
	static int time_selector = 0;

	
	lastTimes[time_selector] = frmrt;
	//If the frame-time is too fast or too slow, mark it as a bad frame.
	bad_frames += (frmrt < 30 || frmrt > 35) ? 1 : 0;
	time_selector = (time_selector > 66) ? 0 : time_selector+1;
	
    framerate = (frmrt)>>4;
	
    if (framerate <= 0) framerate=1;
    else if (framerate > 5) framerate=5;

		//Framegraph
	char curLine = frmrt;
	char prevLine = (time_selector < 1) ? lastTimes[65] : lastTimes[time_selector-1];
	char nthLine = (time_selector < 2) ? lastTimes[65] : lastTimes[time_selector-2];
	
	draw_vdp2_line(time_selector+GRAPH_X_OFFSET, 22, time_selector+GRAPH_X_OFFSET, 8, 0xC210); //(last argument is color)
	draw_vdp2_line(time_selector+GRAPH_X_OFFSET, 22, time_selector+GRAPH_X_OFFSET, (curLine>>2)+6, 0x8200);
		if(time_selector > 1){
	draw_vdp2_line((time_selector-1)+GRAPH_X_OFFSET, 22, (time_selector-1)+GRAPH_X_OFFSET, (prevLine>>2)+6, 0xC000);
	//draw_vdp2_line(20, 20, 40, 40, 0x800C);
		}
		if(time_selector > 2){
	draw_vdp2_line((time_selector-2)+GRAPH_X_OFFSET, 22, (time_selector-2)+GRAPH_X_OFFSET, (nthLine>>2)+6, 0x8010);
		} 
		//
		frmul = framerate<<16;
		
	if(viewInfoTxt == 1)
	{
	nbg_sprintf(1, 3, "(%i) Bad Frames)", bad_frames);
	nbg_sprintf(16, 4, "(%i) fmrt", frmrt);
	}

}

