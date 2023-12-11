
#include <sl_def.h>
#include "def.h"
#include "mymath.h"
#include "hmap.h"
#include "vdp2.h"
#include "bounder.h"
#include "physobjet.h"

#include "minimap.h"

unsigned char oneLineCt = 51;
unsigned short pixCt = 0;
unsigned int MapPixWidth = 0;
unsigned int MapYpixWidth = 0;
unsigned int mmapXScale = 0;
unsigned int mmapYScale = 0;
unsigned int approx_row = 0;
unsigned int raw_pix_data = 0;
unsigned int exact_map_data = 0;
unsigned int xDrawPos = 0;
unsigned int yDrawPos = 0;
unsigned short colorData = 0;
unsigned short minimap[50 * 51];

#define MMAP_BASE_Y (173)
#define MMAP_BASE_X (12)
#define MMAP_CNTR_Y (MMAP_BASE_Y+25)
#define MMAP_CNTR_X (MMAP_BASE_X+25)
#define MMAP_CTR_PIX (1300)
#define MMAP_WIDTH	(50)
#define	MMAP_HEIGHT	(50)

void	init_minimap(void)
{

	 approx_row = 0;
	 raw_pix_data = 0;
	 exact_map_data = 0;
	 xDrawPos = 0;
	 yDrawPos = 0;
	 colorData = 0;
	 pixCt = 0;
	 oneLineCt = 0;
	 
	mmapXScale = fxdiv(MMAP_WIDTH<<16, main_map_x_pix<<16);
	mmapYScale = fxdiv(MMAP_HEIGHT<<16, main_map_y_pix<<16);
	
	MapPixWidth = fxdiv(main_map_x_pix<<16, MMAP_WIDTH<<16);
	MapYpixWidth = fxdiv(main_map_y_pix<<16, MMAP_HEIGHT<<16);
	
	//I want to re-do this in floating points (or find some way around the number cap of fixed points) so I can make bigger map
	//But then again, the map is so huge then, what use is it? Hmm...
	
	//Parses map data of variable size into 51x51 15-bit grayscale image, writes to screen and stores in array.
	//Potential problem: Size of FIXED-point means the total pixels for a map can't exceed 65535. The constrained variable is raw_pix_data.

}

void	add_position_to_minimap(int xpos, int ypos, unsigned short color, short pattern)
{
	int xSclPos = -fxm(xpos<<16, mmapXScale)>>16;
	int ySclPos = fxm(ypos<<16, mmapYScale)>>16;
	
	int tgtPix = MMAP_CTR_PIX + xSclPos + (ySclPos * 51);
	int patPix[12];
	int patPixPos[12][2];
	
	switch(pattern)
	{
		case(MINIMAP_ONE_PIXEL):
		patPix[0] = tgtPix;
		patPixPos[0][X] = MMAP_CNTR_X+xSclPos;
		patPixPos[0][Y] = MMAP_CNTR_Y+ySclPos;
		draw_hud_pixel(patPixPos[0][X], patPixPos[0][Y], color);
		minimap[patPix[0]] = color;
		break;
		case(MINIMAP_X_PATTERN):
		patPix[0] = tgtPix;
		patPixPos[0][X] = MMAP_CNTR_X+xSclPos;
		patPixPos[0][Y] = MMAP_CNTR_Y+ySclPos;
		patPix[1] = tgtPix+MMAP_WIDTH+1;
		patPixPos[1][X] = patPixPos[0][X] + 1;
		patPixPos[1][Y] = patPixPos[0][Y] + 1;
		patPix[2] = tgtPix+MMAP_WIDTH-1;
		patPixPos[2][X] = patPixPos[0][X] - 1;
		patPixPos[2][Y] = patPixPos[0][Y] + 1;
		patPix[3] = tgtPix-MMAP_WIDTH+1;
		patPixPos[3][X] = patPixPos[0][X] + 1;
		patPixPos[3][Y] = patPixPos[0][Y] - 1;
		patPix[4] = tgtPix-MMAP_WIDTH-1;
		patPixPos[4][X] = patPixPos[0][X] - 1;
		patPixPos[4][Y] = patPixPos[0][Y] - 1;
		for(int i = 0; i < 5; i++)
		{
			draw_hud_pixel(patPixPos[i][X], patPixPos[i][Y], color);
			minimap[patPix[i]] = color;
		}
		break;
		case(MINIMAP_P_PATTERN):
		patPix[0] = tgtPix;
		patPixPos[0][X] = MMAP_CNTR_X+xSclPos;
		patPixPos[0][Y] = MMAP_CNTR_Y+ySclPos;
		patPix[1] = tgtPix+1;
		patPixPos[1][X] = patPixPos[0][X] + 1;
		patPixPos[1][Y] = patPixPos[0][Y];
		patPix[2] = tgtPix+MMAP_WIDTH;
		patPixPos[2][X] = patPixPos[0][X];
		patPixPos[2][Y] = patPixPos[0][Y] + 1;
		patPix[3] = tgtPix-1;
		patPixPos[3][X] = patPixPos[0][X] - 1;
		patPixPos[3][Y] = patPixPos[0][Y];
		patPix[4] = tgtPix-MMAP_WIDTH;
		patPixPos[4][X] = patPixPos[0][X];
		patPixPos[4][Y] = patPixPos[0][Y] - 1;
		for(int i = 0; i < 5; i++)
		{
			draw_hud_pixel(patPixPos[i][X], patPixPos[i][Y], color);
			minimap[patPix[i]] = color;
		}
		break;
		default:
		break;
	}
}

void	update_mmap_1pass(void)
{
	int oneLinePix = 0;
		if(oneLineCt < 51 && pixCt < 2550)
		{
	for( ; oneLinePix < MMAP_WIDTH; oneLinePix++)
	{
		colorData = (main_map[exact_map_data]>>1)+127;
		draw_hud_pixel(MMAP_BASE_X+xDrawPos, MMAP_BASE_Y+yDrawPos, colorData); //Draw to VDP2 NBG1, displaying as bitmap mode.
		minimap[pixCt] = colorData;						//Statement stores current data into an an array in high memory at given index [array pos = bitmap pos]
		raw_pix_data += MapPixWidth;									//Downsampling fixed-point math. Adds the approximate number of pixels to skip to scale PGM file to 51x51.
		exact_map_data = raw_pix_data>>16;								//Back-sample to integers; finds the exact pixel we are going to sample.
		approx_row = (xDrawPos < MMAP_WIDTH) ? approx_row : approx_row + MapYpixWidth; //Determines the Y image coordinate on the minimap
		raw_pix_data = (xDrawPos < MMAP_WIDTH) ? raw_pix_data : ((main_map_x_pix*(approx_row>>16))<<16); //Jumps raw_pix_data every time we jump Y image coordinate on the minimap
		xDrawPos = (xDrawPos < MMAP_WIDTH) ? xDrawPos + 1 : 0;			//Increments X draw coordinate
		yDrawPos = (xDrawPos < MMAP_WIDTH) ? yDrawPos : yDrawPos + 1;	//Increments Y draw coordinate (rows)
		pixCt++;
	}
		}
	oneLineCt++;
}

void	draw_minimap(void)
{
	static int scaledPlrPos[2];
	static int dirPixPos1[2];
	static int prevPlrPos[2];
	
	
	//Re-draws minimap data back where we were last frame. This is neccessary, because otherwise, the player location pip would just overwrite the data on screen
	//and just draw pixels across it.
	if( JO_ABS(scaledPlrPos[0]+1) < 25 && JO_ABS(scaledPlrPos[1]+1) < 25)
	{
	draw_hud_pixel(MMAP_CNTR_X+prevPlrPos[0], MMAP_CNTR_Y+prevPlrPos[1], minimap[MMAP_CTR_PIX+( prevPlrPos[0] + (prevPlrPos[1] * 51))]);
	draw_hud_pixel(MMAP_CNTR_X+prevPlrPos[0], MMAP_CNTR_Y+1+prevPlrPos[1], minimap[MMAP_CTR_PIX+( prevPlrPos[0] + ( (prevPlrPos[1]+1) * 51))]);
	draw_hud_pixel(MMAP_CNTR_X+1+prevPlrPos[0], MMAP_CNTR_Y+prevPlrPos[1], minimap[MMAP_CTR_PIX+( (prevPlrPos[0]+1) + (prevPlrPos[1] * 51))]);
	draw_hud_pixel(MMAP_CNTR_X-1+prevPlrPos[0], MMAP_CNTR_Y+prevPlrPos[1], minimap[MMAP_CTR_PIX+( (prevPlrPos[0]-1) + (prevPlrPos[1] * 51))]);
	draw_hud_pixel(MMAP_CNTR_X+prevPlrPos[0], MMAP_CNTR_Y-1+prevPlrPos[1], minimap[MMAP_CTR_PIX+( prevPlrPos[0] + ( (prevPlrPos[1]-1) * 51))]);
	
	draw_hud_pixel(MMAP_CNTR_X-1+prevPlrPos[0], MMAP_CNTR_Y-1+prevPlrPos[1], minimap[MMAP_CTR_PIX+( (prevPlrPos[0]-1) + ( (prevPlrPos[1]-1) * 51))]);
	draw_hud_pixel(MMAP_CNTR_X+1+prevPlrPos[0], MMAP_CNTR_Y+1+prevPlrPos[1], minimap[MMAP_CTR_PIX+( (prevPlrPos[0]+1) + ( (prevPlrPos[1]+1) * 51))]);
	
	draw_hud_pixel(MMAP_CNTR_X-1+prevPlrPos[0], MMAP_CNTR_Y+1+prevPlrPos[1], minimap[MMAP_CTR_PIX+( (prevPlrPos[0]-1) + ( (prevPlrPos[1]+1) * 51))]);
	draw_hud_pixel(MMAP_CNTR_X+1+prevPlrPos[0], MMAP_CNTR_Y-1+prevPlrPos[1], minimap[MMAP_CTR_PIX+( (prevPlrPos[0]+1) + ( (prevPlrPos[1]-1) * 51))]);
	}
	//Determine's the maps scale relative to minimap size

	//Scales your position to the minimap size
	scaledPlrPos[0] = (fxm(you.cellPos[X]<<16, mmapXScale)>>16);
	scaledPlrPos[1] = -(fxm(you.cellPos[Y]<<16, mmapYScale)>>16);
	//Separates the components of your view to find a pip for that, too.
	register int sinY = slSin(you.viewRot[Y]);
	register int cosY = -slCos(you.viewRot[Y]);
		dirPixPos1[0] = (sinY > 0) ? -1 : 1;
		dirPixPos1[1] = (cosY > 0) ? 1 : -1;
		dirPixPos1[0] = (JO_ABS(sinY) < 32000) ? 0 : dirPixPos1[0];
		dirPixPos1[1] = (JO_ABS(cosY) < 32000) ? 0 : dirPixPos1[1];
	//Border
	// Lines from: 	Xmax, Ymax, Xmin, Ymax - BOTTOM line
	//				Xmax, Ymax, Xmax, Ymin - RIGHT line
	//				Xmax, Ymin, Xmin, Ymin - TOP line
	//				Xmin, Ymin, Xmin, Ymax - LEFT line
	//				
	draw_hud_line(MMAP_BASE_X, MMAP_BASE_Y, MMAP_BASE_X, MMAP_BASE_Y+51, 3); //LEFT LINE
	draw_hud_line(MMAP_BASE_X+51, MMAP_BASE_Y+51, MMAP_BASE_X+51, MMAP_BASE_Y, 17); //RIGHT LINE
	draw_hud_line(MMAP_BASE_X+51, MMAP_BASE_Y+50, MMAP_BASE_X, MMAP_BASE_Y+50, 49); //BTM LINE
	draw_hud_line(MMAP_BASE_X, MMAP_BASE_Y, MMAP_BASE_X+51, MMAP_BASE_Y, 32); //TOP LINE


	
	//Draws player position pip
	if( JO_ABS(scaledPlrPos[0]+1) < 25 && JO_ABS(scaledPlrPos[1]+1) < 25)
	{
	draw_hud_pixel(MMAP_CNTR_X+scaledPlrPos[0], MMAP_CNTR_Y+scaledPlrPos[1], 19);
	draw_hud_pixel(MMAP_CNTR_X+scaledPlrPos[0], MMAP_CNTR_Y+1+scaledPlrPos[1], 19);
	draw_hud_pixel(MMAP_CNTR_X+1+scaledPlrPos[0], MMAP_CNTR_Y+scaledPlrPos[1], 19);
	draw_hud_pixel(MMAP_CNTR_X-1+scaledPlrPos[0], MMAP_CNTR_Y+scaledPlrPos[1], 19);
	draw_hud_pixel(MMAP_CNTR_X+scaledPlrPos[0], MMAP_CNTR_Y-1+scaledPlrPos[1], 19);
	}
	//Draws view direction pip
	if(JO_ABS(scaledPlrPos[0]+dirPixPos1[0]+1) < 25 && JO_ABS(scaledPlrPos[1]+dirPixPos1[1]+1) < 25)
	{
	draw_hud_pixel(MMAP_CNTR_X+scaledPlrPos[0]+dirPixPos1[0], MMAP_CNTR_Y+scaledPlrPos[1]+dirPixPos1[1], 4);
	}
	
	prevPlrPos[0] = scaledPlrPos[0];
	prevPlrPos[1] = scaledPlrPos[1];
}

