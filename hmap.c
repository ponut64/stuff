//Heightmap Code
//This file is compiled separately.
//hmap.c

#include "hmap.h"

Uint8 * main_map = (Uint8*)LWRAM;
Uint8 * buf_map = (Uint8*)LWRAM;

unsigned char * local_hmap;
unsigned int * yValueIndex;
unsigned short * minimap;
int main_map_total_pix = 625;
int main_map_x_pix = 25;
int main_map_y_pix = 25;
int main_map_strata[4] = {40, 70, 100, 170};
bool map_update_complete;
bool map_chg = false;

void	read_gmp_header(_heightmap * map){
				Uint8 newlines = 0;
				Uint8 NumberOfNumbers;
				Uint8 spaceChar = 0;
				Uint8 numLeftOfSpace = 0;
				Uint8 numRightOfSpace = 0;
				Uint8 leftFactor = 0;
				Uint8 rightFactor = 0;
				
				Uint8* newlinePtr[4] = {0, 0, 0, 0};
				for(Uint8 l = 0; newlines < 4; l++){
					Uint8 * chset = (Uint8 *)map->dstAddress + l;
					if(*chset == 10){
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
					if(*arrayOfCharacters[c] == 32){
						spaceChar = c;
					}
				}
				numLeftOfSpace = spaceChar;
				numRightOfSpace = (NumberOfNumbers - (spaceChar + 1) );

				Uint8 bufCharLeft[numLeftOfSpace];
				Uint8 bufCharRight[numRightOfSpace];
				for(Uint8 r = 0; r < numLeftOfSpace; r++){
					bufCharLeft[r] = *arrayOfCharacters[r] - 48;
				}
				for(Uint8 v = 0; v < numLeftOfSpace; v++){
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
					//Here, we set the map's X and Z (Y, actually, not Z) to the data read from the header. And also multiplies them and gets the total pixels.
					map->Xval = leftFactor;
					map->Yval = rightFactor;
					map->totalPix = leftFactor * rightFactor;
					//Not included: GFS load sys, but that's ez
}

//Only works at start of program
void init_heightmap(void)
{
/*Vertice Order Hint
0 - 1
3 - 2
*/
	//jo_malloc(1024 * 200);
	local_hmap = (void*)jo_malloc(625);
	yValueIndex = (void*)jo_malloc(576 * sizeof(int));
	minimap = (void*)jo_malloc(2550 * sizeof(short));
}

void	chg_map(_heightmap * tmap){
		for(Uint16 i = 0; i < tmap->totalPix && map_chg == false && tmap->active != true && tmap->file_done == true; i++){
			main_map[i] = buf_map[i];
			if(i == tmap->totalPix-1){
				main_map_x_pix = tmap->Xval;
				main_map_y_pix = tmap->Yval;
				main_map_total_pix = tmap->totalPix;
				init_minimap();
				map_chg = true;
			}
		}
}

//Texture Table Assignment based on heights from main map strata table.
int	strataFinder(int index){
	static int avgY = 0;
	avgY = -(polymap->pntbl[polymap->pltbl[index].Vertices[0]][Y] + polymap->pntbl[polymap->pltbl[index].Vertices[1]][Y] +
	polymap->pntbl[polymap->pltbl[index].Vertices[2]][Y] + polymap->pntbl[polymap->pltbl[index].Vertices[3]][Y])>>18;
	if(avgY < main_map_strata[0]){ 
		return 0;
	} else if(avgY >= main_map_strata[0] && avgY < main_map_strata[1]){ 
		return 7;
	} else if(avgY >= main_map_strata[1] && avgY < main_map_strata[2]){ 
		return 14;
	} else if(avgY >= main_map_strata[2] && avgY < main_map_strata[3]){ 
		return 21;
	} else if(avgY >= main_map_strata[3]){ 
		return 28;
	} 
	return 0; //No purpose, simply clips compiler warning.
}

//Texture Assignment Handler stage of new data calculation in local polymap.
void	texHandler(int index){
	//if(txtbl_e[4].file_done != true) return;
	int texno = 0;
	
	POINT norm = {polymap->pltbl[index].norm[X], polymap->pltbl[index].norm[Y], polymap->pltbl[index].norm[Z]};
	POINT absN = {JO_ABS(norm[X]), JO_ABS(norm[Y]), JO_ABS(norm[Z])};
	short baseTex = strataFinder(index);
	short flip = 0;
	
	if(absN[X] <= 4096 && absN[Z] <= 4096)
	{
		texno = 0;
	} else {
		if(absN[X] > absN[Z]){
					if(absN[X] < 24576)
					{
			texno += (absN[Z] > 8192) ? 2 : 3;
			flip = (norm[X] > 0) ? FLIPH : 0;
			flip += ( ((norm[Z] & -1) | 57344) > 0 ) ? FLIPV : 0; //Condition for "if ABS(norm[Z]) >= 8192 and norm[Z] < 8192", saves branch
					} else {
			texno += (absN[Z] > 16384) ? 5 : 6;
			flip = (norm[X] > 0) ? FLIPH : 0;
			flip += ( ((norm[Z] & -1) | 57344) > 0 ) ? FLIPV : 0;
					}
		} else {
					if(absN[Z] < 24576)
					{
			texno += (absN[X] > 8192) ? 2 : 1;
			flip = (norm[Z] > 0) ? FLIPV : 0;
			flip += ( ((norm[X] & -1) | 57344) > 0 ) ? FLIPH : 0;
					} else {
			texno += (absN[X] > 16384) ? 5 : 4;
			flip = (norm[Z] > 0) ? FLIPV : 0;
			flip += ( ((norm[X] & -1) | 57344) > 0 ) ? FLIPH : 0;
					}
		}
	}
	
	polymap->attbl[index].texno = baseTex+texno;
	polymap->attbl[index].dir = flip;
}

//Calculates a normal for the polymap given the index into the polymap
void	normHandler(int index){
	static VECTOR cross = {0, 0, 0};

	static VECTOR rminusb = {-(25<<16), 0, (25<<16)};
	static VECTOR sminusb = {(25<<16), 0, (25<<16)};
	

	rminusb[Y] = (polymap->pntbl[polymap->pltbl[index].Vertices[2]][Y]) - (polymap->pntbl[polymap->pltbl[index].Vertices[0]][Y]);

	sminusb[Y] = (polymap->pntbl[polymap->pltbl[index].Vertices[3]][Y]) - (polymap->pntbl[polymap->pltbl[index].Vertices[1]][Y]);

	
	cross_fixed(rminusb, sminusb, cross);
	cross[X] = cross[X]>>8;
	cross[Y] = cross[Y]>>8;
	cross[Z] = cross[Z]>>8;

	normalize(cross, cross);
	
	polymap->pltbl[index].norm[X] = -cross[X];
	polymap->pltbl[index].norm[Y] = -cross[Y];
	polymap->pltbl[index].norm[Z] = -cross[Z];
	
}


void	update_hmap(void){
	map_update_complete = false;

	register int xDir = (you.dispPos[X] - you.prevDispPos[X]);
	register int zDir = (you.dispPos[Y] - you.prevDispPos[Y]);
	
	register int xDo = 0;
	register int zDo = 0;
	
	static int startOffset;
	startOffset = (main_map_total_pix>>1) - (main_map_x_pix * 12) - 12;
	static int rowOffset;
	static int x_pix_sample;
	static int y_pix_sample;
	static int src_pix;
	static int dst_pix;
	static int rowLimit;
	static int RightBoundPixel;
	
/*
Normal Copy Loops
We already have normals on the map and don't want to recalculate them all every frame.
There might also be attributes for each polygon that we don't want to recalculate, such as its ground scatter or doodad assignments.
So, based on the direction we've moved, we should be able to copy all present polygon data to where it's moved to.
The following accounts for the various directions we can move the map and how we should copy the data through it.
*/

//I thought the next step was zero-translation scaling, but..
//There are so many ways to add water, I kind of want to go that direction.
//
	xDo = JO_ABS(xDir);
	zDo = JO_ABS(zDir * 24);
	
			y_pix_sample = (you.dispPos[Y] * (main_map_x_pix));
		for(Uint16 k = 0; k < 25; k++){
			rowOffset = (k * main_map_x_pix) + startOffset;
			rowLimit = rowOffset / main_map_x_pix;
			for(Uint16 v = 0; v < 25; v++){
				x_pix_sample = v + rowOffset + you.dispPos[X];
				RightBoundPixel = (x_pix_sample - (rowLimit * main_map_x_pix));
				src_pix = x_pix_sample - y_pix_sample;
				dst_pix = v+(k*25);
				if(src_pix < main_map_total_pix && src_pix >= 0 && RightBoundPixel < main_map_x_pix && RightBoundPixel >= 0){
				local_hmap[dst_pix] = main_map[src_pix];
				} else { //Fill Set Value if outside of map area
				local_hmap[dst_pix] = 127;
				//Note: Presently if a vertice inside the area that is visible in proximity to pixels outside the area with the same value as this..
				//their normals are frequently copied incorrectly.
			}
			polymap->pntbl[dst_pix][Y] = -(local_hmap[dst_pix]<<16);
			}	// Row Filler Loop End Stub
		} // Row Selector Loop End Stub
		
	// jo_printf(0, 11, "(%i)", xDir);
	// jo_printf(0, 12, "(%i)", zDir);


	int normalsHit = 0;
	if( (xDir > 0 && zDir == 0) || (xDir == 0 && zDir < 0) || (xDir >= 1 && zDir <= -1) ){
		
	for(int g = 0; g < 576; g++){
	  	yValueIndex[g] = ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[3]]]) << 24)
		| ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[2]]]) << 16) 
		| ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[1]]]) << 8 )
		| ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[0]]]));
		
	if(yValueIndex[g] == yValueIndex[g+zDo+xDo] && (zDo+xDo) != 0  && (g+zDo+xDo) < 576){
	polymap->pltbl[g].norm[X] = polymap->pltbl[g+zDo+xDo].norm[X]; //DO NOT USE DMA COPY IT IS SLOWER!
	polymap->pltbl[g].norm[Y] = polymap->pltbl[g+zDo+xDo].norm[Y];
	polymap->pltbl[g].norm[Z] = polymap->pltbl[g+zDo+xDo].norm[Z];
	polymap->attbl[g] = polymap->attbl[g+zDo+xDo];
	} else {
	
	normHandler(g);
	normalsHit++;
	texHandler(g);

		}//Calculation End Stub

	} //NORMAL LOOP END STUB
		} else if(  (xDir < 0 && zDir == 0) || (xDir == 0 && zDir > 0) || (xDir <= -1 && zDir >= 1) ){ // Copy Direction Change
		
	for(int g = 575; g >= 0; g--){
	  	yValueIndex[g] = ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[3]]]) << 24)
		| ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[2]]]) << 16) 
		| ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[1]]]) << 8 )
		| ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[0]]]));
		

	if(yValueIndex[g] == yValueIndex[g-zDo-xDo] && (zDo-xDo) != 0 && (g-zDo-xDo) >= 0){
	polymap->pltbl[g].norm[X] = polymap->pltbl[g-zDo-xDo].norm[X];
	polymap->pltbl[g].norm[Y] = polymap->pltbl[g-zDo-xDo].norm[Y];
	polymap->pltbl[g].norm[Z] = polymap->pltbl[g-zDo-xDo].norm[Z];
	polymap->attbl[g] = polymap->attbl[g-zDo-xDo];
	} else {
		
	normHandler(g);
	normalsHit++;
	texHandler(g);
	
	}//Calculation End Stub

	} //NORMAL LOOP END STUB
		} else if((xDir > 0 && zDir > 0)){ //Copy Direction Change
		
	for(int g = 575; g >= 0; g--){
	  	yValueIndex[g] = ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[3]]]) << 24)
		| ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[2]]]) << 16) 
		| ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[1]]]) << 8 )
		| ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[0]]]));
		

	if(yValueIndex[g] == yValueIndex[g-zDo+xDo] && (zDo+xDo) != 0 && (g-zDo+xDo) >= 0){
	polymap->pltbl[g].norm[X] = polymap->pltbl[g-zDo+xDo].norm[X];
	polymap->pltbl[g].norm[Y] = polymap->pltbl[g-zDo+xDo].norm[Y];
	polymap->pltbl[g].norm[Z] = polymap->pltbl[g-zDo+xDo].norm[Z];
	polymap->attbl[g] = polymap->attbl[g-zDo+xDo];
	} else {
		
	normHandler(g);
	normalsHit++;
	texHandler(g);
	
	}//Calculation End Stub

	} //NORMAL LOOP END STUB
		} else if((xDir < 0 && zDir < 0)){ //Copy Direction Change
		
	for(int g = 0; g < 576; g++){
	  	yValueIndex[g] = ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[3]]]) << 24)
		| ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[2]]]) << 16) 
		| ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[1]]]) << 8 )
		| ((Uint8)(main_map[local_hmap[polymap->pltbl[g].Vertices[0]]]));
		

	if(yValueIndex[g] == yValueIndex[g+zDo-xDo] && (zDo-xDo) != 0  && (g+zDo-xDo) < 576){
	polymap->pltbl[g].norm[X] = polymap->pltbl[g+zDo-xDo].norm[X];
	polymap->pltbl[g].norm[Y] = polymap->pltbl[g+zDo-xDo].norm[Y];
	polymap->pltbl[g].norm[Z] = polymap->pltbl[g+zDo-xDo].norm[Z];
	polymap->attbl[g] = polymap->attbl[g+zDo-xDo];
	} else {
	
	normHandler(g);
	normalsHit++;
	texHandler(g);

		}//Calculation End Stub

	} //NORMAL LOOP END STUB
	
		} //Copy End
	if(normalsHit != 0){
	jo_printf(26, 3, "New Norms:(%i)", normalsHit);
	}
	map_update_complete = true;
}


void	hmap_cluster(void)
{
	chg_map(&maps[0]);
	update_hmap();
	draw_minimap();
	update_mmap_1pass();
}
