//Heightmap Code
//This file is compiled separately.
//hmap.c

#include <sl_def.h>
#include <SEGA_GFS.H>
#include <stdio.h>
#include "def.h"
#include "pcmstm.h"
#include "pcmsys.h"
#include "render.h"
#include "physobjet.h"
#include "tga.h"
#include "draw.h"
#include "ldata.h"
#include "vdp2.h"
#include "minimap.h"
#include "mymath.h"
#include "sound.h"
#include "collision.h"
//
#include "dspm.h"
//

#include "hmap.h"

Uint8 * main_map = (Uint8*)LWRAM;
Uint8 * buf_map = (Uint8*)LWRAM;

char * normTbl;
unsigned short * mapTex;
int main_map_total_pix = LCL_MAP_PIX * LCL_MAP_PIX;
int main_map_total_poly = LCL_MAP_PLY * LCL_MAP_PLY;
int main_map_x_pix = LCL_MAP_PIX;
int main_map_y_pix = LCL_MAP_PIX;
int main_map_strata[4] = {40, 70, 100, 170};
int map_texture_table_numbers[5];
int map_tex_amt = 35;
int map_last_combined_texno = 0;
int map_end_of_original_textures = 0;
_heightmap maps[4];
Bool map_update_complete;
Bool * sysBool;
Bool map_chg = true;

char map_tex_tbl_names[5][13] = {
	{'D','I','R','0','.','T','G','A', 0, 0, 0, 0, 0},
	{'D','I','R','1','.','T','G','A', 0, 0, 0, 0, 0},
	{'D','I','R','2','.','T','G','A', 0, 0, 0, 0, 0},
	{'D','I','R','3','.','T','G','A', 0, 0, 0, 0, 0},
	{'D','I','R','4','.','T','G','A', 0, 0, 0, 0, 0}
};

char old_tex_tbl_names[5][13] = {
	{'D','I','R','0','.','T','G','A', 0, 0, 0, 0, 0},
	{'D','I','R','1','.','T','G','A', 0, 0, 0, 0, 0},
	{'D','I','R','2','.','T','G','A', 0, 0, 0, 0, 0},
	{'D','I','R','3','.','T','G','A', 0, 0, 0, 0, 0},
	{'D','I','R','4','.','T','G','A', 0, 0, 0, 0, 0}
};

//Only works at start of program
void 	init_heightmap(void)
{
/*Vertice Order Hint
0 - 1
3 - 2
*/

	sysBool = (Bool *)(((unsigned int)&map_update_complete)|UNCACHE);
}

void	chg_map(_heightmap * tmap){
		for(Uint16 i = 0; i < tmap->totalPix && map_chg == false; i++){
			main_map[i] = buf_map[i];
			if(i == tmap->totalPix-1){
				main_map_x_pix = tmap->Xval;
				main_map_y_pix = tmap->Yval;
				main_map_total_pix = tmap->totalPix;
				init_minimap();
				map_chg = true;
				process_map_for_normals();
			}
		}
}

// I like that I didn't put a comment here explaining the contents of a PGM header.
// It is plain-text, but come on!
void	read_pgm_header(_heightmap * map)
{
				Uint8 newlines = 0;
				Uint8 NumberOfNumbers;
				Uint8 spaceChar = 0;
				Uint8 numLeftOfSpace = 0;
				Uint8 numRightOfSpace = 0;
				Uint16 leftFactor = 0;
				Uint16 rightFactor = 0;
				
				Uint8* newlinePtr[4] = {0, 0, 0, 0};
				for(Uint8 l = 0; newlines < 4; l++){
					Uint8 * chset = (Uint8 *)map->dstAddress + l;
					if(*chset == '\n'){
						newlinePtr[newlines] = (Uint8 *)map->dstAddress + l;
						newlines++;
					}
				} 
					//This line sets the dstAddress of the map data to the exact line where data begins.
					map->dstAddress = newlinePtr[3];
					//
					//All of this below, to the next comment, is stuff that arbitrates the ASCII characters of numbers to numbers to do math with.
					NumberOfNumbers = (newlinePtr[2] - newlinePtr[1]) - 1;
				Uint8* arrayOfCharacters[NumberOfNumbers];
			
				for(Uint8 c = 0; c < NumberOfNumbers; c++){
					arrayOfCharacters[c] = newlinePtr[1] + 1 + c;
					if(*arrayOfCharacters[c] == ' '){
						spaceChar = c;
					}
				}
				numLeftOfSpace = spaceChar;
				numRightOfSpace = (NumberOfNumbers - (spaceChar + 1) );

				Uint8 bufCharLeft[4] = {0, 0, 0, 0};
				Uint8 bufCharRight[4] = {0, 0, 0, 0};
				for(Uint8 r = 0; r < numLeftOfSpace; r++){
					bufCharLeft[r] = *arrayOfCharacters[r] - 48; //"48" being the number to subtract ASCII numbers by 
				}												//to get the number in binary.
				for(Uint8 v = 0; v < numRightOfSpace; v++){
					bufCharRight[v] = *arrayOfCharacters[spaceChar+v+1] - 48;
				}
				
					if(numLeftOfSpace == 3){
					leftFactor += (bufCharLeft[numLeftOfSpace-3] * 100) + (bufCharLeft[numLeftOfSpace-2] * 10) + (bufCharLeft[numLeftOfSpace-1]);
					} else if(numLeftOfSpace == 2){
					leftFactor += (bufCharLeft[numLeftOfSpace-2] * 10) + (bufCharLeft[numLeftOfSpace-1]);
					} else if(numLeftOfSpace == 1){
					leftFactor += (bufCharLeft[numLeftOfSpace-1]);
					}
				
					if(numRightOfSpace == 3){
					rightFactor += (bufCharRight[numRightOfSpace-3] * 100) + (bufCharRight[numRightOfSpace-2] * 10) + (bufCharRight[numRightOfSpace-1]);
					} else if(numRightOfSpace == 2){
					rightFactor += (bufCharRight[numRightOfSpace-2] * 10) + (bufCharRight[numRightOfSpace-1]);
					} else if(numRightOfSpace == 1){
					rightFactor += (bufCharRight[numRightOfSpace-1]);
					}
					//Here, we set the map's X and Y to the data read from the header. And also multiplies them and gets the total pixels.
					map->Xval = leftFactor;
					map->Yval = rightFactor;
					map->totalPix = leftFactor * rightFactor;
					//Not included: GFS load sys, but that's ez
}

Sint8 pgm_name[11];
Sint8 ldat_name[11];

void	map_parser(void * data)
{
	maps[0].dstAddress = data;
	read_pgm_header(&maps[0]);
	
	if(JO_IS_ODD(maps[0].Xval) && JO_IS_ODD(maps[0].Yval)){
		for(int i = 0; i < maps[0].totalPix; i++)
		{
			buf_map[i] = *((Uint8*)maps[0].dstAddress + i);
		}
		
	// nbg_sprintf(8, 20, "(%i)", maps[0].totalPix);
	// nbg_sprintf(15, 20, "(%i)", maps[0].Xval);
	// nbg_sprintf(20, 20, "(%i)", maps[0].Yval);
		} else {
	nbg_sprintf(8, 25, "MAP REJECTED - IS EVEN");
		}
		
	map_chg = false;
}

void	p64MapRequest(short levelNo)
{
	char the_number[3] = {'0', '0', '0'};
	//Why three?
	//Strings need a terminating character.
	//String "99" is actually binary #'9','9',0 (literal zero).
	int num_char = sprintf(the_number, "%i", levelNo);
	if(num_char == 1)
	{
		the_number[1] = the_number[0];
		the_number[0] = '0';
	}
///Fill out the request.
	pgm_name[0] = 'L';
	pgm_name[1] = 'E';
	pgm_name[2] = 'V';
	pgm_name[3] = 'E';
	pgm_name[4] = 'L';
	pgm_name[5] = the_number[0];
	pgm_name[6] = the_number[1];
	pgm_name[7] = '.';
	pgm_name[8] = 'P';
	pgm_name[9] = 'G';
	pgm_name[10] = 'M';
	
	maps[0].Xval = 0;
	maps[0].Yval = 0;
	maps[0].totalPix = 0;

					
 	ldat_name[0] = 'L';
	ldat_name[1] = 'E';
	ldat_name[2] = 'V';
	ldat_name[3] = 'E';
	ldat_name[4] = 'L';
	ldat_name[5] = the_number[0];
	ldat_name[6] = the_number[1];
	ldat_name[7] = '.';
	ldat_name[8] = 'L';
	ldat_name[9] = 'D';
	ldat_name[10] = 'S';
	
	set_music_track = NO_STAGE_MUSIC; //Clear stage music, preparing to change music of course.
	new_file_request(ldat_name, dirty_buf, process_binary_ldata, 0);
	new_file_request(pgm_name, dirty_buf, map_parser, 0);
	ldata_ready = false;

}

/* 		//	if(activePGM->file_done != true){
				read_pgm_header(activePGM);
				//
					if(JO_IS_ODD(activePGM->Xval) && JO_IS_ODD(activePGM->Yval)){
						for(int i = 0; i < activePGM->totalPix; i++)
						{
							buf_map[i] = *((Uint8*)activePGM->dstAddress + i);
						}
				// nbg_sprintf(8, 20, "(%i)", activePGM->totalPix);
				// nbg_sprintf(15, 20, "(%i)", activePGM->Xval);
				// nbg_sprintf(20, 20, "(%i)", activePGM->Yval);
					} else {
				nbg_sprintf(8, 25, "MAP REJECTED - IS EVEN");
					}
			NactivePGM--;
		//	} */


//Texture Table Assignment based on heights from main map strata table.
int	texture_table_by_height(int * ys)
{
	int avgY = 0;
	avgY = ((ys[0] + ys[1] + ys[2] + ys[3])>>2);
	if(avgY < main_map_strata[0]){ 
		return map_texture_table_numbers[0];
	} else if(avgY >= main_map_strata[0] && avgY < main_map_strata[1]){ 
		return map_texture_table_numbers[1];
	} else if(avgY >= main_map_strata[1] && avgY < main_map_strata[2]){ 
		return map_texture_table_numbers[2];
	} else if(avgY >= main_map_strata[2] && avgY < main_map_strata[3]){ 
		return map_texture_table_numbers[3];
	} else if(avgY >= main_map_strata[3]){ 
		return map_texture_table_numbers[4];
	} 
	return 0; //No purpose, simply clips compiler warning.
}

int		texture_angle_resolver(int baseTex, FIXED * norm, unsigned short * flip){
	//if(txtbl_e[4].file_done != true) return;
	
	POINT absN = {JO_ABS(norm[X]), JO_ABS(norm[Y]), JO_ABS(norm[Z])};
	int btdata = baseTex & 1023; // ??? ???
	int texno = 0;
	*flip = 0;
	
	//This code segment tries to orient textures consistently so that "bottom" on a texture is facing "down".
	// Basically, since we are on a height-map, the normal of the polygon is used to determine the slope of the polygon.
	// So we're trying to make an arrow from the top of the texture to the bottom of it follow the slope down.
	if(absN[X] <= 4096 && absN[Z] <= 4096)
	{
		texno = 0;
	} else {
		//In this first branch, we are deciding whether to use the "facing east/west" or "facing north/south" texture.
		if(absN[X] > absN[Z]){
			// X is greater than Z, should be facing on X+/-, henceforth, use the X+ texture.
					if(absN[X] < 24576)
					{
					// In this branch, we have a relatively low X value, so we should use a "gentle slope-ish" texture.
					// We also determine if we should use a texture which splits the directions of X and Z.
					// "facing northeast, northwest, southwest, southeast"
			texno += (absN[Z] > 8192) ? 2 : 3;
			*flip = (norm[X] > 0) ? 0 : FLIPH;
			*flip += ( ((norm[Z] & -1) | 57344) > 0 ) ? 0 : FLIPV; //Condition for "if ABS(norm[Z]) >= 8192 and norm[Z] < 8192", saves branch
					} else {
					//In this branch, we have a high X value, so we use a more wall-ish or rocky type texture. Up to the artist.
					// We do the same logic as above and detect if it's also facing Z a lot too.
			texno += (absN[Z] > 16384) ? 5 : 6;
			*flip = (norm[X] > 0) ? 0 : FLIPH;
			*flip += ( ((norm[Z] & -1) | 57344) > 0 ) ? 0 : FLIPV;
					}
		} else {
			//This section is the same as the previous, except sign-adjusted for the Z axis.
					if(absN[Z] < 24576)
					{
			texno += (absN[X] > 8192) ? 2 : 1;
			*flip = (norm[Z] > 0) ?  0 : FLIPV;
			*flip += ( ((norm[X] & -1) | 57344) > 0 ) ? 0 : FLIPH;
					} else {
			texno += (absN[X] > 16384) ? 5 : 4;
			*flip = (norm[Z] > 0) ?  0 : FLIPV;
			*flip += ( ((norm[X] & -1) | 57344) > 0 ) ? 0 : FLIPH;
					}
		}
	}
	return (btdata+texno) | (baseTex & (1<<15)); // ??? why 1<<15 ????
}

	int starting_small_crossing_texno;
	int starting_combined_crossing_texno;
	int starting_tiny_texno;
	int crossing_small_dither_texno[4][7];
	int crossing_combined_dither_texno[4][7];
	int ultra_downscale_texno[5][7];

void	make_dithered_textures_for_map(short regenerate)
{
	
	/*
		Dithers from table A to table B are done with the crossing dither table.
		In this case, a texture is dithered with its equivalent in the nearest table.
		In the angular dither table, we intend to dither:

			0	1	2	3	4	5	6
		0	X	+	+	+	+	+	+
		1	+	X	-	-	+	-	-
		2	+	-	X	-	-	+	-
		3	+	-	-	X	-	-	+
		4	+	+	-	-	X	-	-
		5	+	-	+	-	-	X	-
		6	+	-	-	+	-	-	X
		
		01, 02, 03, 04, 05, 06, 14, 25, 36
		9 results
	*/
	
		if(!regenerate) starting_combined_crossing_texno = numTex;
	//Loop to ensure the crossing small textures are in series with one another (no other textures between them).
	for(int t = 0; t < 4; t++)
	{
		for(int p = 0; p < 7; p++)
		{
			if(!regenerate)
			{
			crossing_small_dither_texno[t][p] = new_dithered_texture(map_texture_table_numbers[t]+p, map_texture_table_numbers[t+1]+p, 0);
			} else {
			new_dithered_texture(map_texture_table_numbers[t]+p, map_texture_table_numbers[t+1]+p, crossing_small_dither_texno[t][p]);
			}
		}
	}
	
		if(!regenerate) starting_small_crossing_texno = numTex;
	//Loop to ensure the crossing large textures are in series with one another (no other textures between them).
	for(int t = 0; t < 4; t++)
	{
		for(int p = 0; p < 7; p++)
		{
			if(!regenerate)
			{
			crossing_combined_dither_texno[t][p] = new_dithered_texture(map_texture_table_numbers[t]+map_tex_amt+p, map_texture_table_numbers[t+1]+p+map_tex_amt, 0);
			} else {
			new_dithered_texture(map_texture_table_numbers[t]+map_tex_amt+p, map_texture_table_numbers[t+1]+p+map_tex_amt, crossing_combined_dither_texno[t][p]);
			}
		}
	}
	
	/*
	Writing 8x8 Ultra-downscale
	The texture we have now is the original 24x24 texture, downscaled to 12x12, then tiled back to 24x24.
	It is so to represent that the polygons are subdivided.
	To save on VDP1 performance, we can also make even more trashy textures by making the far away ones 8x8.
	This of course down-scales the original texture all the way down to 4x4.
	But we're just going to use a pre-existing function to make downscales from them.
	*/
	unsigned char * readByte;
	for(int i = 0; i < 5; i++)
	{
		for(int j = 0; j < 7; j++)
		{
			if(!regenerate)
			{
				readByte = (unsigned char *)((unsigned int)(VDP1_VRAM + (pcoTexDefs[map_texture_table_numbers[i]+j+map_tex_amt].SRCA<<3)));
				ultra_downscale_texno[i][j] = numTex;
				generate_downscale_texture(24, 24, 8, 8, readByte);
			} else {
				replace_downscale_texture(map_texture_table_numbers[i]+j+map_tex_amt, ultra_downscale_texno[i][j]);
			}
		}
	}
	if(!regenerate) starting_tiny_texno = ultra_downscale_texno[0][0];
	
}

void	process_map_for_normals(void)
{
	int e = 0;
	int ys[4];
	VECTOR rminusb = {-(CELL_SIZE), 0, (CELL_SIZE)};
	VECTOR sminusb = {(CELL_SIZE), 0, (CELL_SIZE)};
	VECTOR cross = {0, 0, 0};
	
	int norm_index = 0;
	unsigned char * readByte = &main_map[0];
	
	main_map_total_poly = 0;
	
	//Please don't ask me why this works...
	//I was up late one day and.. oh man..
	for(int k = 0; k < (main_map_y_pix-1); k++){
		for(int v = 0; v < (main_map_x_pix-1); v++){
			e	= (k * (main_map_x_pix)) + v - 1;
			ys[0]	= readByte[e];
			ys[1]	= readByte[e + 1];
			ys[2]	= readByte[e + 1 + (main_map_x_pix)];
			ys[3]	= readByte[e + (main_map_x_pix)];


			
	rminusb[1] = (ys[2]<<16) - (ys[0]<<16);

	sminusb[1] = (ys[3]<<16) - (ys[1]<<16);
	
	fxcross(rminusb, sminusb, cross);

	cross[X] = cross[X]>>8; //Shift to supresss overflows
	cross[Y] = cross[Y]>>8;
	cross[Z] = cross[Z]>>8;

	double_normalize(cross, cross);
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// I should seriously consider using the ANORM.H table here.
	// That requires looping forward to calculate the real normals and then backwards to resolve to an ANORM entry --
	// BUT that is a quality optimization, because then I can use another table for pre-made dot products for real-time lighting.
	// I should then reprocess all entities to use ANORM.H entries. That will make real-time lighting a lot cheaper.
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	normTbl[norm_index] = (cross[X] > 0) ? -JO_ABS(cross[X]>>9) : JO_ABS(cross[X]>>9); //X value write
	normTbl[norm_index+1] = (cross[Y] > 0) ? -JO_ABS(cross[Y]>>9) : JO_ABS(cross[Y]>>9); //Y value write
	normTbl[norm_index+2] = (cross[Z] > 0) ? -JO_ABS(cross[Z]>>9) : JO_ABS(cross[Z]>>9); //Z value write
	norm_index+=3;

	/////////////////////
	// Pre-computation of textures on map change (needed the normal first)
	/////////////////////
	mapTex[main_map_total_poly] = texture_table_by_height(&ys[0]);

	main_map_total_poly++;
		}
		
	}
	
	////////////////////
	// Now that we've found what texture table everything is going to use...
	// Let's run some logic to assign a new texture to every texture that is bordering another texture
	////////////////////
	int counted_poly = 0;
	norm_index = 0;
	static int tex_near[4] = {0, 0, 0, 0};
	//
	//	A _bitflag_ will be added to textures with detected borders.
	//
	
	/////////////////////////////
	//	This loop finds when there is a gap in the texture tables.
	//	Textures whom are adjacent to textures using a different texture table will receive a dither assignment.
	/////////////////////////////
	/// There is a bug in this code segment causing an improperly sized texture to be used:
	/// It will use the four-way combined texture both far and near, this is incorrect.
	/// I am unsure of the cause.
/* 	for(int k = 0; k < (main_map_y_pix-1); k++){
		for(int v = 0; v < (main_map_x_pix-1); v++){

  			int this_texture = mapTex[counted_poly];
			tex_near[0] = ((counted_poly + 1) <= main_map_total_poly) ?  mapTex[counted_poly+1] : this_texture;
			tex_near[1] = ((counted_poly - 1) > 0) ?  mapTex[counted_poly-1] : this_texture;
			tex_near[2] = ((counted_poly + (main_map_x_pix-1)) <= main_map_total_poly) ?  mapTex[counted_poly+(main_map_x_pix-1)] : this_texture;
			tex_near[3] = ((counted_poly - (main_map_x_pix-1)) > 0) ? mapTex[counted_poly-(main_map_x_pix-1)] : this_texture;

				for(int h = 0; h < 4; h++)
				{
					if(this_texture != tex_near[h] && !(tex_near[h] & DITHER_CROSS))
					{
						tex_near[h] &= 1023;
						this_texture &= 1023;
			 			if(this_texture < tex_near[h])
						{
							mapTex[counted_poly] |= DITHER_CROSS;
							break;
						}
					}
				}
			counted_poly++;
		}
	} */
	counted_poly = 0;
	
	///////////////////////////////////////////////
	// This loop assigns textures and texture orientation to slopes.
	///////////////////////////////////////////////
	for(int k = 0; k < (main_map_y_pix-1); k++)
	{
		for(int v = 0; v < (main_map_x_pix-1); v++)
		{
			  
 			cross[X] = normTbl[norm_index]<<9;
			cross[Y] = normTbl[norm_index+1]<<9;
			cross[Z] = normTbl[norm_index+2]<<9;
			norm_index += 3;
			
			unsigned short flip;
			int texno = texture_angle_resolver(mapTex[counted_poly], &cross[0], &flip);
			mapTex[counted_poly] = (texno) | (flip<<8);
			 
			counted_poly++;
		}
	}
	counted_poly = 0;

}

	
	// nbg_sprintf(2, 6, "mp0(%i)", map_texture_table_numbers[0]);
	// nbg_sprintf(2, 7, "mp0(%i)", map_texture_table_numbers[1]);
	// nbg_sprintf(2, 8, "mp0(%i)", map_texture_table_numbers[2]);
	// nbg_sprintf(2, 9, "mp0(%i)", map_texture_table_numbers[3]);
	// nbg_sprintf(2, 10, "mp0(%i)", map_texture_table_numbers[4]);

	// slPrintFX((int)(normTbl[normCheck]<<9), slLocate(0, 8));
	// slPrintFX((int)(normTbl[normCheck+1]<<9), slLocate(0, 9));
	// slPrintFX((int)(normTbl[normCheck+2]<<9), slLocate(0, 10));
	
	// nbg_sprintf(0, 13, "(%i)mtp", main_map_total_poly);

	//Helper function for a routine which uses per-polygon light processing.
	//This is different than the normal light processing in that it will get light data from any number of lights,
	//based only on the distance from the polygon to the light.
	/** SHOULD BE INLINED **/
int		per_polygon_light(GVPLY * model, POINT wldPos, int polynumber)
{
	int luma = 0;
	for(int i = 0; i < MAX_DYNAMIC_LIGHTS; i++)
	{
	point_light * lightSrc = &active_lights[i];
			if(lightSrc->pop == 1)
			{
		//Get distance to the polygon approximate center (v0 + v2) - map_pos - light_pos (+ because the map is moved inverse) 
		POINT light_proxima = {
		(( ( (model->pntbl[model->pltbl[polynumber].vertices[0]][X] + model->pntbl[model->pltbl[polynumber].vertices[2]][X])>>1)
		+ wldPos[X]) + lightSrc->pos[X])>>16,
		(( ( (model->pntbl[model->pltbl[polynumber].vertices[0]][Y] + model->pntbl[model->pltbl[polynumber].vertices[2]][Y])>>1)
		+ wldPos[Y]) + lightSrc->pos[Y])>>16,
		(( ( (model->pntbl[model->pltbl[polynumber].vertices[0]][Z] + model->pntbl[model->pltbl[polynumber].vertices[2]][Z])>>1)
		+ wldPos[Z]) + lightSrc->pos[Z])>>16
		};
		// Get inverse distance (some extrapolation of 1/sqrt(d) 
		int inverted_proxima = ((light_proxima[X] * light_proxima[X]) +
								(light_proxima[Y] * light_proxima[Y]) +
								(light_proxima[Z] * light_proxima[Z]));
		inverted_proxima = (inverted_proxima < 65536) ? division_table[inverted_proxima]>>1 : 0;

		luma += inverted_proxima * (int)lightSrc->bright;
			}
	}
	return luma;
}

short preprocess_portals(_portal ** used_portal)
{
	short enable_portals = 0;
	//Primary goal:
	//Look through the portal list and:
	// 1. Find which portal is the largest. We want to use that one.
	// 2. Determine the depth of the portal
	// 3. Determine whether to front or backface the portal
	// 4. Return a pointer to the used portal
	// nbg_sprintf(0, 15, "port(%i)", current_portal_count);
	static int bestsize = 0;
	int cursize = 0;
	if(current_portal_count > 0)
	{
		bestsize = 0;
		for(int i = 0; i < current_portal_count; i++)
		{
			//Ideally, I would use the polygon with the best balance of X/Y span, in addition to best X/Y size.
			//So a tiny Y span would be tossed in favor of something that had a larger X, or vice versa.
			int szA[2] = {portals[i].verts[0][X] - portals[i].verts[1][X], portals[i].verts[0][Y] - portals[i].verts[1][Y]};
			int szB[2] = {portals[i].verts[1][X] - portals[i].verts[2][X], portals[i].verts[1][Y] - portals[i].verts[2][Y]};
			int szC[2] = {portals[i].verts[2][X] - portals[i].verts[3][X], portals[i].verts[2][Y] - portals[i].verts[3][Y]};
			int szD[2] = {portals[i].verts[3][X] - portals[i].verts[0][X], portals[i].verts[3][Y] - portals[i].verts[0][Y]};
			cursize =  (szA[X] * szA[X]) + (szA[Y] * szA[Y]);
			cursize += (szB[X] * szB[X]) + (szB[Y] * szB[Y]);
			cursize += (szC[X] * szC[X]) + (szC[Y] * szC[Y]);
			cursize += (szD[X] * szD[X]) + (szD[Y] * szD[Y]);
			if(cursize > bestsize)
			{
				*used_portal = &portals[i];
				portals[i].depth = JO_MAX(JO_MAX(portals[i].verts[0][Z], portals[i].verts[1][Z]), JO_MAX(portals[i].verts[2][Z], portals[i].verts[3][Z]));
				bestsize = cursize;
				
				//A cross-product can tell us if it's backfaced or not.
				int cross0 = (portals[i].verts[1][X] - portals[i].verts[3][X])
									* (portals[i].verts[0][Y] - portals[i].verts[2][Y]);
				int cross1 = (portals[i].verts[1][Y] - portals[i].verts[3][Y])
									* (portals[i].verts[0][X] - portals[i].verts[2][X]);
				portals[i].backface = (cross1 >= cross0) ? 1 : 0;
				
				portals[i].min[X] = JO_MIN(JO_MIN(portals[i].verts[0][X], portals[i].verts[1][X]), JO_MIN(portals[i].verts[2][X], portals[i].verts[3][X]));
				portals[i].min[Y] = JO_MIN(JO_MIN(portals[i].verts[0][Y], portals[i].verts[1][Y]), JO_MIN(portals[i].verts[2][Y], portals[i].verts[3][Y]));
				portals[i].max[X] = JO_MAX(JO_MAX(portals[i].verts[0][X], portals[i].verts[1][X]), JO_MAX(portals[i].verts[2][X], portals[i].verts[3][X]));
				portals[i].max[Y] = JO_MAX(JO_MAX(portals[i].verts[0][Y], portals[i].verts[1][Y]), JO_MAX(portals[i].verts[2][Y], portals[i].verts[3][Y]));

			}	
		}
		enable_portals = 1;
	}
	if(portal_reset == 1)
	{
		current_portal_count = 0;
		portal_reset = 0;
	}
	return enable_portals;
}

////////////////////////////
// Helper data for dynamic polygon subdivision
////////////////////////////
POINT verts_without_inverse_z[LCL_MAP_PIX * LCL_MAP_PIX];
int new_vertices[6][3];
vertex_t * new_polygons[4][4];
vertex_t scrnsub[5];

int		unmath_center[3];

////////////////////////////
// Takes a polygon from the map & its associated data from the precompiled tables and subdivides it four-ways.
// Then issues the commands. The polygon had already been checked for culling, so we can check for less of that.
////////////////////////////
void	render_map_subdivided_polygon(int * dst_poly, int * texno, unsigned short * flip, int * depth,  FIXED * tempNorm)
{
	//Shorthand Point Data
	FIXED * pnts[4];
	FIXED * umpt[4];
	int luma = 0;
	unsigned short used_flip = *flip;
	unsigned short colorBank = 0;
	unsigned short pclp = 0;

		pnts[0] = &verts_without_inverse_z[polymap->pltbl[*dst_poly].vertices[0]][0];
		pnts[1] = &verts_without_inverse_z[polymap->pltbl[*dst_poly].vertices[1]][0];
		pnts[2] = &verts_without_inverse_z[polymap->pltbl[*dst_poly].vertices[2]][0];
		pnts[3] = &verts_without_inverse_z[polymap->pltbl[*dst_poly].vertices[3]][0];
		umpt[0] = &polymap->pntbl[polymap->pltbl[*dst_poly].vertices[0]][0];
		umpt[1] = &polymap->pntbl[polymap->pltbl[*dst_poly].vertices[1]][0];
		umpt[2] = &polymap->pntbl[polymap->pltbl[*dst_poly].vertices[2]][0];
		umpt[3] = &polymap->pntbl[polymap->pltbl[*dst_poly].vertices[3]][0];
	/*
	0A			1A | 0B			1B
							
			0				1
							
	3A			2A | 3B			2B		
	
	0D			1D | 0C			1C
	
			3				2

	3D			2D | 3C			2C
	*/
	
	//Center
	// This is created to do lighting effects on each polygon in the proper vector space.
	// The other points that are rendered are created from post-matrix transformed points.
	// This one is made from points of the PDATA, without any transformation.
	unmath_center[X] = (umpt[0][X] + umpt[1][X] + umpt[2][X] + umpt[3][X])>>2;
	unmath_center[Y] = (umpt[0][Y] + umpt[1][Y] + umpt[2][Y] + umpt[3][Y])>>2;
	unmath_center[Z] = (umpt[0][Z] + umpt[1][Z] + umpt[2][Z] + umpt[3][Z])>>2;
	
	
	//Initial State
	new_polygons[0][0] = &msh2VertArea[polymap->pltbl[*dst_poly].vertices[0]];
	
	new_polygons[1][1] = &msh2VertArea[polymap->pltbl[*dst_poly].vertices[1]];
	
	new_polygons[2][2] = &msh2VertArea[polymap->pltbl[*dst_poly].vertices[2]];
	
	new_polygons[3][3] = &msh2VertArea[polymap->pltbl[*dst_poly].vertices[3]];
	// Center
	// 
	new_vertices[0][X] = (pnts[0][X] + pnts[1][X] + 
						pnts[2][X] + pnts[3][X])>>2;
	new_vertices[0][Y] = (pnts[0][Y] + pnts[1][Y] + 
						pnts[2][Y] + pnts[3][Y])>>2;
	new_vertices[0][Z] = (pnts[0][Z] + pnts[1][Z] + 
						pnts[2][Z] + pnts[3][Z])>>2;
	//
	new_polygons[0][2] = &scrnsub[0];
	new_polygons[1][3] = &scrnsub[0];
	new_polygons[2][0] = &scrnsub[0];
	new_polygons[3][1] = &scrnsub[0];
	// 0 -> 1
	new_vertices[1][X] = (pnts[0][X] + pnts[1][X])>>1;
	new_vertices[1][Y] = (pnts[0][Y] + pnts[1][Y])>>1;
	new_vertices[1][Z] = (pnts[0][Z] + pnts[1][Z])>>1;
	new_polygons[0][1] = &scrnsub[1];
	new_polygons[1][0] = &scrnsub[1];
	// 1 -> 2
	new_vertices[2][X] = (pnts[2][X] + pnts[1][X])>>1;
	new_vertices[2][Y] = (pnts[2][Y] + pnts[1][Y])>>1;
	new_vertices[2][Z] = (pnts[2][Z] + pnts[1][Z])>>1;
	new_polygons[1][2] = &scrnsub[2];
	new_polygons[2][1] = &scrnsub[2];
	// 3 -> 2
	new_vertices[3][X] = (pnts[2][X] + pnts[3][X])>>1;
	new_vertices[3][Y] = (pnts[2][Y] + pnts[3][Y])>>1;
	new_vertices[3][Z] = (pnts[2][Z] + pnts[3][Z])>>1;
	new_polygons[2][3] = &scrnsub[3];
	new_polygons[3][2] = &scrnsub[3];
	// 3 -> 0
	new_vertices[4][X] = (pnts[0][X] + pnts[3][X])>>1;
	new_vertices[4][Y] = (pnts[0][Y] + pnts[3][Y])>>1;
	new_vertices[4][Z] = (pnts[0][Z] + pnts[3][Z])>>1;
	new_polygons[0][3] = &scrnsub[4];
	new_polygons[3][0] = &scrnsub[4];
	
	//Pre-loop
	//Start division for first vertex
	// I've profiled it; doing this before the polygon body appears to... not be any faster, but it's not slower,
	// and hypothetically, it should be faster. I'll take it.
	SetFixDiv(scrn_dist, new_vertices[0][Z]);
	
	for(int w = 0; w < 5; w++)
	{
		//Get 1/z
		int inverseZ = *DVDNTL;
		//Set next 1/z
		SetFixDiv(scrn_dist, new_vertices[w+1][Z]);

        //Transform to screen-space
        scrnsub[w].pnt[X] = fxm(new_vertices[w][X], inverseZ)>>SCR_SCALE_X;
        scrnsub[w].pnt[Y] = fxm(new_vertices[w][Y], inverseZ)>>SCR_SCALE_Y;
        //Screen Clip Flags for on-off screen decimation
		scrnsub[w].clipFlag = ((scrnsub[w].pnt[X]) > TV_HALF_WIDTH) ? SCRN_CLIP_X : 0; 
		scrnsub[w].clipFlag |= ((scrnsub[w].pnt[X]) < -TV_HALF_WIDTH) ? SCRN_CLIP_NX : scrnsub[w].clipFlag; 
		scrnsub[w].clipFlag |= ((scrnsub[w].pnt[Y]) > TV_HALF_HEIGHT) ? SCRN_CLIP_Y : scrnsub[w].clipFlag;
		scrnsub[w].clipFlag |= ((scrnsub[w].pnt[Y]) < -TV_HALF_HEIGHT) ? SCRN_CLIP_NY : scrnsub[w].clipFlag;
	}
	transVerts[0] += 5;
	vertex_t * ptv[5];
	for(int q = 0; q < 4; q++)
	{
		ptv[0] = new_polygons[q][0];
		ptv[1] = new_polygons[q][1];
		ptv[2] = new_polygons[q][2];
		ptv[3] = new_polygons[q][3];
		 int offScrn = (ptv[0]->clipFlag & ptv[2]->clipFlag & ptv[1]->clipFlag & ptv[3]->clipFlag);
		
		if(offScrn || msh2SentPolys[0] >= MAX_MSH2_SENT_POLYS){ continue; }
		used_flip = *flip;
//Pre-clipping Function
		preclipping(ptv, &used_flip, &pclp);
		
		//Z position:
		//Weighted max, even further max than normal. Oddly needed to be weighted even further out.
		int zDepthTgt = (JO_MAX(JO_MAX(ptv[0]->pnt[Z], ptv[2]->pnt[Z]), JO_MAX(ptv[1]->pnt[Z], ptv[3]->pnt[Z])) + *depth)>>1;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Lighting 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		luma = 0;
/* 		for(int i = 0; i < MAX_DYNAMIC_LIGHTS; i++)
		{
		point_light * lightSrc = &active_lights[i];
				if(lightSrc->pop == 1)
				{
			//Get distance to the polygon approximate center (v0 + v2) - map_pos - light_pos (+ because the map is moved inverse) 
			POINT light_proxima = {
			(( ( (umpt[q][X] + unmath_center[X])>>1)
			+ hmap_actual_pos[X]) + lightSrc->pos[X])>>16,
			(( ( (umpt[q][Y] + unmath_center[Y])>>1)
			+ hmap_actual_pos[Y]) + lightSrc->pos[Y])>>16,
			(( ( (umpt[q][Z] + unmath_center[Z])>>1)
			+ hmap_actual_pos[Z]) + lightSrc->pos[Z])>>16
			};
			// Get inverse distance (some extrapolation of 1/sqrt(d) 
			int inverted_proxima = ((light_proxima[X] * light_proxima[X]) +
									(light_proxima[Y] * light_proxima[Y]) +
									(light_proxima[Z] * light_proxima[Z]));
			inverted_proxima = (inverted_proxima < 65536) ? division_table[inverted_proxima]>>1 : 0;
	
			luma += inverted_proxima * (int)lightSrc->bright;
				}
		}	
		luma = (luma < 0) ? 0 : luma; //We set the minimum luma as zero so the dynamic light does not corrupt the global light's basis.
 */		luma += fxdot(tempNorm, active_lights[0].ambient_light) + active_lights[0].min_bright;
		//In normal "vision" however, bright light would do that..
		//Use transformed normal as shade determinant
		determine_colorbank(&colorBank, &luma);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        msh2SetCommand(ptv[0]->pnt, ptv[1]->pnt, ptv[2]->pnt, ptv[3]->pnt,
                     VDP1_BASE_CMDCTRL | (used_flip), ( VDP1_BASE_PMODE ) | pclp, //Reads used_flip value
                     pcoTexDefs[*texno].SRCA, colorBank<<6, pcoTexDefs[*texno].SIZE, 0, zDepthTgt);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}
	transPolys[0] += 4;
}

void	update_hmap(MATRIX msMatrix)
{ //Master SH2 drawing function (needs to be sorted after by slave)

	static int rowOffset;
	static int x_pix_sample;
	static int y_pix_sample;
	static int dst_pix;
	
	register int dspReturnTgt = 0;

	static FIXED m0x[4];
	static FIXED m1y[4];
	static FIXED m2z[4];
	
	m0x[0] = msMatrix[X][X];
	m0x[1] = msMatrix[Y][X];
	m0x[2] = msMatrix[Z][X];
	m0x[3] = msMatrix[3][X];
	
	m1y[0] = msMatrix[X][Y];
	m1y[1] = msMatrix[Y][Y];
	m1y[2] = msMatrix[Z][Y];
	m1y[3] = msMatrix[3][Y];
	
	m2z[0] = msMatrix[X][Z];
	m2z[1] = msMatrix[Y][Z];
	m2z[2] = msMatrix[Z][Z];
	m2z[3] = msMatrix[3][Z];
	
	FIXED luma = 0;
	unsigned short colorBank;
	unsigned short pclp = 0;
	int inverseZ = 0;

	static _portal * used_portal = &portals[0];
	short portal_active = preprocess_portals(&used_portal);
	
	//Loop Concept:
	//SO just open Paint.NET make a 255x255 image.
	//Then with the selection tool, drag a box exactly 25x25 in size.
	//Then move the box around. That's exactly that this does.
	//Well, maybe that's the simple version.
	//It writes the pixels in that area to polygons, as the polygon's Y value.
	//We also do vertice transformation here.
	
	//The SCU-DSP does this now [including the division, lol].
	
	//The whole way its being done is very stupid, though ?
	
	// static int startOffset;
	// startOffset = (main_map_total_pix>>1) - (main_map_x_pix * (LCL_MAP_PIX>>1)) - (LCL_MAP_PIX>>1);
	// static int src_pix;
	// static int rowLimit;
	// static int RightBoundPixel;
	// y_pix_sample = (you.dispPos[Y] * (main_map_x_pix));
	// for(int k = 0; k < LCL_MAP_PIX; k++)
	// {
	// rowOffset = (k * main_map_x_pix) + startOffset;
	// rowLimit = rowOffset / main_map_x_pix;
		// for(int v = 0; v < LCL_MAP_PIX; v++)
		// {
			// x_pix_sample = v + rowOffset + you.dispPos[X];
			// RightBoundPixel = (x_pix_sample - (rowLimit * main_map_x_pix));
			// src_pix = x_pix_sample - y_pix_sample;
			// dst_pix = v+(k*LCL_MAP_PIX);
			// if(src_pix < main_map_total_pix && src_pix >= 0 && RightBoundPixel < main_map_x_pix && RightBoundPixel >= 0)
			// {
		// polymap->pntbl[dst_pix][Y] = -(main_map[src_pix]<<MAP_V_SCALE);
			// } else { //Fill Set Value if outside of map area
		// polymap->pntbl[dst_pix][Y] = -(127<<MAP_V_SCALE);
			// }
		// }
	// }
	for(dst_pix = 0; dst_pix < (LCL_MAP_PIX * LCL_MAP_PIX); dst_pix++)
	{
		dspReturnTgt = dsp_output_addr[dst_pix];
		if(dspReturnTgt >= 0){
		polymap->pntbl[dst_pix][Y] = -(main_map[dspReturnTgt]<<(MAP_V_SCALE));
			} else { //Fill Set Value if outside of map area
		polymap->pntbl[dst_pix][Y] = -(127<<(MAP_V_SCALE));
			}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Vertice 3D Transformation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /**calculate z**/
        msh2VertArea[dst_pix].pnt[Z] = trans_pt_by_component(polymap->pntbl[dst_pix], m2z);
		msh2VertArea[dst_pix].pnt[Z] = (msh2VertArea[dst_pix].pnt[Z] > NEAR_PLANE_DISTANCE) ? msh2VertArea[dst_pix].pnt[Z] : NEAR_PLANE_DISTANCE;
		verts_without_inverse_z[dst_pix][Z] = msh2VertArea[dst_pix].pnt[Z];
         /**Starts the division**/
        SetFixDiv(scrn_dist, msh2VertArea[dst_pix].pnt[Z]);

        /**Calculates X and Y while waiting for screenDist/z **/
        verts_without_inverse_z[dst_pix][Y] = trans_pt_by_component(polymap->pntbl[dst_pix], m1y);
        verts_without_inverse_z[dst_pix][X] = trans_pt_by_component(polymap->pntbl[dst_pix], m0x);
		
        /** Retrieves the result of the division **/
		inverseZ = *DVDNTL;

        /**Transform X and Y to screen space**/
        msh2VertArea[dst_pix].pnt[X] = fxm(verts_without_inverse_z[dst_pix][X], inverseZ)>>SCR_SCALE_X;
        msh2VertArea[dst_pix].pnt[Y] = fxm(verts_without_inverse_z[dst_pix][Y], inverseZ)>>SCR_SCALE_Y;
 
        //Screen Clip Flags for on-off screen decimation
		msh2VertArea[dst_pix].clipFlag = ((msh2VertArea[dst_pix].pnt[X]) > TV_HALF_WIDTH) ? SCRN_CLIP_X : 0; 
		msh2VertArea[dst_pix].clipFlag |= ((msh2VertArea[dst_pix].pnt[X]) < -TV_HALF_WIDTH) ? SCRN_CLIP_NX : msh2VertArea[dst_pix].clipFlag; 
		msh2VertArea[dst_pix].clipFlag |= ((msh2VertArea[dst_pix].pnt[Y]) > TV_HALF_HEIGHT) ? SCRN_CLIP_Y : msh2VertArea[dst_pix].clipFlag;
		msh2VertArea[dst_pix].clipFlag |= ((msh2VertArea[dst_pix].pnt[Y]) < -TV_HALF_HEIGHT) ? SCRN_CLIP_NY : msh2VertArea[dst_pix].clipFlag;
		msh2VertArea[dst_pix].clipFlag |= ((msh2VertArea[dst_pix].pnt[Z]) <= 15<<16) ? CLIP_Z : msh2VertArea[dst_pix].clipFlag;

		//Portalling
		//What if the DSP did this? in parallel?
		if(portal_active)
		{
			if(msh2VertArea[dst_pix].pnt[X] > used_portal->min[X])
			{
				if(msh2VertArea[dst_pix].pnt[X] < used_portal->max[X])
				{
					if(msh2VertArea[dst_pix].pnt[Y] > used_portal->min[Y])
					{
						if(msh2VertArea[dst_pix].pnt[Y] < used_portal->max[Y])
						{
							int discard = (used_portal->backface) ? N_Zn : N_Zp;
							if(edge_wind_test(used_portal->verts[0], used_portal->verts[1], msh2VertArea[dst_pix].pnt, discard, 0) >= 0)
							{
								if(edge_wind_test(used_portal->verts[1], used_portal->verts[2], msh2VertArea[dst_pix].pnt, discard, 0) >= 0)
								{
									if(edge_wind_test(used_portal->verts[2], used_portal->verts[3], msh2VertArea[dst_pix].pnt, discard, 0) >= 0)
									{
										if(edge_wind_test(used_portal->verts[3], used_portal->verts[0], msh2VertArea[dst_pix].pnt, discard, 0) >= 0)
										{
											msh2VertArea[dst_pix].clipFlag |= CLIP_Z;
										}
									}
								}
							}
						}
					}
				}
			}
		}

	//	}	// Row Filler Loop End Stub
	} // Row Selector Loop End Stub
	// nbg_sprintf(0, 15, "max(%i),(%i)", used_portal->max[X], used_portal->max[Y]);
	// nbg_sprintf(0, 16, "min(%i),(%i)", used_portal->min[X], used_portal->min[Y]);	
	
    transVerts[0] += LCL_MAP_PIX * LCL_MAP_PIX;

	//Loop explanation:
	//When we retrieve the map from the CD, we calculate the normals of every "potential" polygon in the map and write them to NormTbl.
	//The map is forced to be odd in both dimensions (X and Y) to force there to be an objective "center" pixel. This is necessary so we can find our place in the map,
	//for the above loop.
	//However, this complicates assigning our "compressed" normals to polygons.
	//The local map is an odd number too (25x25 right now). The polygon maps are always even then, the local polymap being 24x24 and the big map being (map_dimension-1)^2.
	//So we have to do some mathematical gymnastics to find something we can represent as the center of the polygonal map.
	//Very important is to find the local polygon 0 ("top left") to use as the offset for our pos.
	//So how do we define the integer center of even numbers? Well, we can't but we do have a guide:
	//The vertice map does have a center. So we define our center as the polygon which has the center vertice (of the main map)
	// as its first vertice [vertice 0 of 0-1-2-3].
	// currently, 12 represents the polymap_width>>1
			
	//We also draw the polygons here.
	
	//This polygon contains the center pixel (main_map_total_pix>>1)
	int poly_center = ((main_map_total_poly>>1) + 1 + ((main_map_x_pix-1)>>1)); 
	//This is the upper-left polygon of the display area
	int poly_offset = (poly_center - ((main_map_x_pix-1) * (LCL_MAP_PLY>>1))) - (LCL_MAP_PLY>>1); 
	int src_norm = 0;
	int dst_poly = 0;	//(Destination polygon # for drawing)
	int src_poly = 0; //(Source polygon # for texture data)
	VECTOR tempNorm = {0, 0, 0}; //Temporary normal used so the normal read does not have to be written again
	
	vertex_t * ptv[5] = {0, 0, 0, 0, 0}; //5th value used as temporary vert ID
	//Temporary flip value used as the texture's flip characteristic so we don't have to write it back to memory
	unsigned short flip = 0; 
	int texno = 0; //Ditto
	int dither = 0;
	
	y_pix_sample = ((you.dispPos[Y]) * (main_map_x_pix-1));
	
	int subbed_polys = 0;
			
	for(int k = 0; k < LCL_MAP_PLY; k++)
	{
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Row Selection
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		rowOffset = (k * (main_map_x_pix-1)) + poly_offset;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		for(int v = 0; v < LCL_MAP_PLY; v++)
		{
				
			dst_poly = v+(k*LCL_MAP_PLY);
			
			ptv[0] = &msh2VertArea[polymap->pltbl[dst_poly].vertices[0]];
			ptv[1] = &msh2VertArea[polymap->pltbl[dst_poly].vertices[1]];
			ptv[2] = &msh2VertArea[polymap->pltbl[dst_poly].vertices[2]];
			ptv[3] = &msh2VertArea[polymap->pltbl[dst_poly].vertices[3]];
			////////////////////////////////////////////////
			//Backface & Screenspace Culling Section
			////////////////////////////////////////////////

			//Components of screen-space cross-product used for backface culling.
			//Vertice order hint:
			// 0 - 1
			// 3 - 2
			//A cross-product can tell us if it's facing the screen. If it is not, we do not want it.
			int cross0 = (ptv[1]->pnt[X] - ptv[3]->pnt[X])
								* (ptv[0]->pnt[Y] - ptv[2]->pnt[Y]);
			int cross1 = (ptv[1]->pnt[Y] - ptv[3]->pnt[Y])
								* (ptv[0]->pnt[X] - ptv[2]->pnt[X]);
			//Sorting target. Uses weighted max.
			// int zDepthTgt = JO_MAX(
			// JO_MAX(ptv[0]->pnt[Z], ptv[2]->pnt[Z]),
			// JO_MAX(ptv[1]->pnt[Z], ptv[3]->pnt[Z]));
			int zDepthTgt = (JO_MAX(
			JO_MAX(ptv[0]->pnt[Z], ptv[2]->pnt[Z]),
			JO_MAX(ptv[1]->pnt[Z], ptv[3]->pnt[Z])) + 
			((ptv[0]->pnt[Z] + ptv[1]->pnt[Z] + ptv[2]->pnt[Z] + ptv[3]->pnt[Z])>>2))>>1;
			int offScrn = (ptv[0]->clipFlag & ptv[2]->clipFlag & ptv[1]->clipFlag & ptv[3]->clipFlag);
			
			if((cross0 >= cross1) || offScrn || zDepthTgt < NEAR_PLANE_DISTANCE || zDepthTgt > FAR_PLANE_DISTANCE || msh2SentPolys[0] >= MAX_MSH2_SENT_POLYS){ continue; }

			////////////////////////////////////////////////
			//HMAP Normal/Texture Finder
			////////////////////////////////////////////////
			x_pix_sample = (v + rowOffset + (you.dispPos[X]));
			src_poly = x_pix_sample - y_pix_sample;
			
			src_norm = (src_poly) * 3; //*3 because there are X,Y,Z components
			
			tempNorm[X] = normTbl[src_norm]<<9;
			tempNorm[Y] = normTbl[src_norm+1]<<9;
			tempNorm[Z] = normTbl[src_norm+2]<<9;

			texno = mapTex[src_poly] & 1023;
			flip = (mapTex[src_poly]>>8) & 48;
			dither = mapTex[src_poly] & DITHER_CROSS;

			////////////////////////////////////////////////
			//If any of the Z's are less than 100, render the polygon subdivided four ways.
			// Hold up: Don't you **only** want to do this if the polygon would otherwise cross the near-plane?
			// I think that's the only way this makes sense.
			////////////////////////////////////////////////	
			if(zDepthTgt < HMAP_SUBDIVISION_LEVEL)
			{	
				texno += (dither & DITHER_CROSS) ? starting_small_crossing_texno : 0;
				subbed_polys++;
				render_map_subdivided_polygon(&dst_poly, &texno, &flip, &zDepthTgt, &tempNorm[0]);
				continue;
			}
			//Since this texture was not subdivided, use its corresponding combined texture.
			//That is found by adding the base map texture amount to the texture number.
			//This also manages whether or not to use a dithered texture. It's ugly to add *another* branch to the render loop...
			//The "tiny texture" is also an option if the polygon is far away.
			if(zDepthTgt > HMAP_TINY_TEX_LEVEL)
			{
				texno -= map_texture_table_numbers[0];
				texno = ultra_downscale_texno[0][0] + texno;
			} else {
				texno += (dither & DITHER_CROSS) ? starting_combined_crossing_texno : map_tex_amt;
			}
			////////////////////////////////////////////////
			// You'd think this doesn't need to be here, because otherwise everything would be subdivided or small.
			// BUT PLEASE TRUST ME IT *NEEDS* TO BE HERE.
			////////////////////////////////////////////////
			//preclipping(ptv, &flip, &pclp);
			pclp |= VDP1_PRECLIPPING_DISABLE;
			////////////////////////////////////////////////
			//Lighting
			////////////////////////////////////////////////
			luma = 0;
			// To protect the global light from being reduced by the dynamic lights, limit the dynamic light to 0 luma.
			//luma = per_polygon_light(polymap, hmap_actual_pos, dst_poly);
			//luma = (luma < 0) ? 0 : luma; 
			
			luma += fxdot(tempNorm, active_lights[0].ambient_light) + active_lights[0].min_bright;
			//In normal "vision" however, bright light would do that..
			//Use transformed normal as shade determinant
			determine_colorbank(&colorBank, &luma);
			////////////////////////////////////////////////
			msh2SetCommand(ptv[0]->pnt, ptv[1]->pnt, ptv[2]->pnt, ptv[3]->pnt,
				VDP1_BASE_CMDCTRL | (flip), ( VDP1_BASE_PMODE ) | pclp, //Reads flip value
				pcoTexDefs[texno].SRCA, colorBank<<6, pcoTexDefs[texno].SIZE, 0, zDepthTgt);
			////////////////////////////////////////////////
		}	// Row Filler Loop End Stub
	} // Row Selector Loop End Stub
		
		transPolys[0] += LCL_MAP_PLY * LCL_MAP_PLY;
	*sysBool = true;
}


void	hmap_cluster(void)
{
	
	chg_map(&maps[0]);
	
}
