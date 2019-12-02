
#include "minimap.h"

void	init_minimap(void)
{

	static Uint32 MapPixWidth = 0;
	static Uint32 MapZpixWidth = 0;
	static Uint32 approx_row = 0;
	static Uint32 raw_pix_data = 0;
	static Uint32 exact_map_data = 0;
	static Uint32 xDrawPos = 0;
	static Uint32 yDrawPos = 0;
	static Uint32 index = 0;
	static Uint16 colorData = 0;

	 approx_row = 0;
	 raw_pix_data = 0;
	 exact_map_data = 0;
	 xDrawPos = 0;
	 yDrawPos = 0;
	 colorData = 0;
	 index = 0;

	MapPixWidth = slDivFX(50<<16, main_map_x_pix<<16);
	MapZpixWidth = slDivFX(50<<16, main_map_z_pix<<16);
	
	//I want to re-do this in floating points (or find some way around the number cap of fixed points) so I can make bigger map
	//But then again, the map is so huge then, what use is it? Hmm...
	
	//Parses map data of variable size into 51x51 15-bit grayscale image, writes to screen and stores in array.
	//Potential problem: Size of FIXED-point means the total pixels for a map can't exceed 65535. The constrained variable is raw_pix_data.
	for(int r = 0; r < 2550; r++){
		colorData = 32768 | (main_map[exact_map_data]>>3) | (main_map[exact_map_data]>>3)<<5 | (main_map[exact_map_data]>>3)<<10; //Encode pixel to ON | B | G | R for grayscale
		jo_put_pixel_in_background(5+xDrawPos, 144+yDrawPos, colorData); //Draw to VDP2 NBG1, displaying as bitmap mode.
		minimap[r] = colorData;											//Statement stores current data into an an array in high memory at given index [array pos = bitmap pos]
		raw_pix_data += MapPixWidth;									//Downsampling fixed-point math. Adds the approximate number of pixels to skip to scale PGM file to 51x51.
		exact_map_data = raw_pix_data>>16;								//Back-sample to integers; finds the exact pixel we are going to sample.
		approx_row = (xDrawPos < 50) ? approx_row : approx_row + MapZpixWidth; //Determines the Y image coordinate on the minimap
		raw_pix_data = (xDrawPos < 50) ? raw_pix_data : ((main_map_x_pix*(approx_row>>16))<<16); //Jumps raw_pix_data every time we jump Y image coordinate on the minimap
		xDrawPos = (xDrawPos < 50) ? xDrawPos + 1 : 0;			//Increments X draw coordinate
		yDrawPos = (xDrawPos < 50) ? yDrawPos : yDrawPos + 1;	//Increments Y draw coordinate (rows)
	}

}


void	draw_minimap(void)
{
	static int MapXscale = 0;
	static int MapZscale = 0;
	static int scaledPlrPos[2];
	static int dirPixPos1[2];
	static int prevPlrPos[2];
	
	
	#ifdef JO_480p
	
	#else
	//Re-draws minimap data back where we were last frame. This is neccessary, because otherwise, the player location pip would just overwrite the data on screen
	//and just draw pixels across it.
	if( JO_ABS(scaledPlrPos[0]+1) < 25 && JO_ABS(scaledPlrPos[1]+1) < 25){
	jo_put_pixel_in_background(30+prevPlrPos[0], 169+prevPlrPos[1], minimap[1300+( prevPlrPos[0] + (prevPlrPos[1] * 51))]);
	jo_put_pixel_in_background(30+prevPlrPos[0], 170+prevPlrPos[1], minimap[1300+( prevPlrPos[0] + ( (prevPlrPos[1]+1) * 51))]);
	jo_put_pixel_in_background(31+prevPlrPos[0], 169+prevPlrPos[1], minimap[1300+( (prevPlrPos[0]+1) + (prevPlrPos[1] * 51))]);
	jo_put_pixel_in_background(29+prevPlrPos[0], 169+prevPlrPos[1], minimap[1300+( (prevPlrPos[0]-1) + (prevPlrPos[1] * 51))]);
	jo_put_pixel_in_background(30+prevPlrPos[0], 168+prevPlrPos[1], minimap[1300+( prevPlrPos[0] + ( (prevPlrPos[1]-1) * 51))]);
	
	jo_put_pixel_in_background(29+prevPlrPos[0], 168+prevPlrPos[1], minimap[1300+( (prevPlrPos[0]-1) + ( (prevPlrPos[1]-1) * 51))]);
	jo_put_pixel_in_background(31+prevPlrPos[0], 170+prevPlrPos[1], minimap[1300+( (prevPlrPos[0]+1) + ( (prevPlrPos[1]+1) * 51))]);
	
	jo_put_pixel_in_background(29+prevPlrPos[0], 170+prevPlrPos[1], minimap[1300+( (prevPlrPos[0]-1) + ( (prevPlrPos[1]+1) * 51))]);
	jo_put_pixel_in_background(31+prevPlrPos[0], 168+prevPlrPos[1], minimap[1300+( (prevPlrPos[0]+1) + ( (prevPlrPos[1]-1) * 51))]);
	}
	//Determine's the maps scale relative to minimap size
	MapXscale = slDivFX(main_map_x_pix<<16, 50<<16);
	MapZscale = slDivFX(main_map_z_pix<<16, 50<<16);
	//Scales your position to the minimap size
	scaledPlrPos[0] = (fxm(you.cellPos[X]<<16, MapXscale)>>16);
	scaledPlrPos[1] = -(fxm(you.cellPos[Y]<<16, MapZscale)>>16);
	//Separates the components of your view to find a pip for that, too.
	register int sinY = slSin(you.viewRot[Y]);
	register int cosY = -slCos(you.viewRot[Y]);
		dirPixPos1[0] = (sinY > 0) ? -1 : 1;
		dirPixPos1[1] = (cosY > 0) ? 1 : -1;
		dirPixPos1[0] = (JO_ABS(sinY) < 32000) ? 0 : dirPixPos1[0];
		dirPixPos1[1] = (JO_ABS(cosY) < 32000) ? 0 : dirPixPos1[1];
	//Border
	jo_draw_background_square(5, 143, 51, 51, 0xCFFF);
	
	//Draws player position pip
	if( JO_ABS(scaledPlrPos[0]+1) < 25 && JO_ABS(scaledPlrPos[1]+1) < 25){
	jo_put_pixel_in_background(30+scaledPlrPos[0], 169+scaledPlrPos[1], 0xCC1F);
	jo_put_pixel_in_background(30+scaledPlrPos[0], 170+scaledPlrPos[1], 0xCC1F);
	jo_put_pixel_in_background(31+scaledPlrPos[0], 169+scaledPlrPos[1], 0xCC1F);
	jo_put_pixel_in_background(29+scaledPlrPos[0], 169+scaledPlrPos[1], 0xCC1F);
	jo_put_pixel_in_background(30+scaledPlrPos[0], 168+scaledPlrPos[1], 0xCC1F);
	}
	//Draws view direction pip
	if(JO_ABS(scaledPlrPos[0]+dirPixPos1[0]+1) < 25 && JO_ABS(scaledPlrPos[1]+dirPixPos1[1]+1) < 25){
	jo_put_pixel_in_background(30+scaledPlrPos[0]+dirPixPos1[0], 169+scaledPlrPos[1]+dirPixPos1[1], 0x83FF);
	}
	#endif
	
	prevPlrPos[0] = scaledPlrPos[0];
	prevPlrPos[1] = scaledPlrPos[1];
}

