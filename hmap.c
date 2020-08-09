//Heightmap Code
//This file is compiled separately.
//hmap.c

#include "hmap.h"

Uint8 * main_map = (Uint8*)LWRAM;
Uint8 * buf_map = (Uint8*)LWRAM;

char * normTbl;
unsigned short * minimap;
int main_map_total_pix = LCL_MAP_PIX * LCL_MAP_PIX;
int main_map_total_poly = LCL_MAP_PLY * LCL_MAP_PLY;
int main_map_x_pix = LCL_MAP_PIX;
int main_map_y_pix = LCL_MAP_PIX;
int main_map_strata[4] = {40, 70, 100, 170};
bool map_update_complete;
bool * sysbool;
bool map_chg = false;

void	read_pgm_header(_heightmap * map)
{
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
					bufCharLeft[r] = *arrayOfCharacters[r] - 48; //"48" being the number to subtract ASCII numbers by 
				}												//to get the number in binary.
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
					//Here, we set the map's X and Y to the data read from the header. And also multiplies them and gets the total pixels.
					map->Xval = leftFactor;
					map->Yval = rightFactor;
					map->totalPix = leftFactor * rightFactor;
					//Not included: GFS load sys, but that's ez
}

void	process_map_for_normals(_heightmap * map)
{
	int e = 0;
	int y0 = 0;
	int y1 = 0;
	int y2 = 0;
	int y3 = 0;
	
	VECTOR rminusb = {-(25<<16), 0, (25<<16)};
	VECTOR sminusb = {(25<<16), 0, (25<<16)};
	VECTOR cross = {0, 0, 0};
	
	int norm_index = 0;
	unsigned char * readByte = &main_map[0];
	
	main_map_total_poly = 0;
	
	//Please don't ask me why this works...
	//I was up late one day and.. oh man..
	for(int k = 0; k < (main_map_y_pix-1); k++){
		for(int v = 0; v < (main_map_x_pix-1); v++){
			e	= (k * (main_map_x_pix)) + v - 1;
			y0	= readByte[e];
			y1	= readByte[e + 1];
			y2	= readByte[e + 1 + (main_map_x_pix)];
			y3	= readByte[e + (main_map_x_pix)];

	main_map_total_poly++;
			
	rminusb[1] = (y2<<16) - (y0<<16);

	sminusb[1] = (y3<<16) - (y1<<16);
	
	cross_fixed(rminusb, sminusb, cross);

	cross[X] = cross[X]>>8; //Shift to supresss overflows
	cross[Y] = cross[Y]>>8;
	cross[Z] = cross[Z]>>8;

	double_normalize(cross, cross);
	
	normTbl[norm_index] = (cross[X] > 0) ? -JO_ABS(cross[X]>>9) : JO_ABS(cross[X]>>9); //X value write
	normTbl[norm_index+1] = (cross[Y] > 0) ? -JO_ABS(cross[Y]>>9) : JO_ABS(cross[Y]>>9); //Y value write
	normTbl[norm_index+2] = (cross[Z] > 0) ? -JO_ABS(cross[Z]>>9) : JO_ABS(cross[Z]>>9); //Z value write
	norm_index+=3;
			
		}
		
	}
	
	// slPrintFX((int)(normTbl[normCheck]<<9), slLocate(0, 8));
	// slPrintFX((int)(normTbl[normCheck+1]<<9), slLocate(0, 9));
	// slPrintFX((int)(normTbl[normCheck+2]<<9), slLocate(0, 10));
	
	//jo_printf(0, 13, "(%i)mtp", main_map_total_poly);
	
}

//Only works at start of program
void 	init_heightmap(void)
{
/*Vertice Order Hint
0 - 1
3 - 2
*/
	sysbool = (bool *)(((unsigned int)&map_update_complete)|UNCACHE);
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
				process_map_for_normals(tmap);
			}
		}
}

//Texture Table Assignment based on heights from main map strata table.
__jo_force_inline int	strataFinder(int index){
	int avgY = 0;
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

__jo_force_inline int		tex2Handler(int index, FIXED * norm, int * flip){
	//if(txtbl_e[4].file_done != true) return;
	
	POINT absN = {JO_ABS(norm[X]), JO_ABS(norm[Y]), JO_ABS(norm[Z])};
	int baseTex = strataFinder(index);
	int texno = 0;
	*flip = 0;
	
	if(absN[X] <= 4096 && absN[Z] <= 4096)
	{
		texno = 0;
	} else {
		if(absN[X] > absN[Z]){
					if(absN[X] < 24576)
					{
			texno += (absN[Z] > 8192) ? 2 : 3;
			*flip = (norm[X] > 0) ? FLIPH : 0;
			*flip += ( ((norm[Z] & -1) | 57344) > 0 ) ? FLIPV : 0; //Condition for "if ABS(norm[Z]) >= 8192 and norm[Z] < 8192", saves branch
					} else {
			texno += (absN[Z] > 16384) ? 5 : 6;
			*flip = (norm[X] > 0) ? FLIPH : 0;
			*flip += ( ((norm[Z] & -1) | 57344) > 0 ) ? FLIPV : 0;
					}
		} else {
					if(absN[Z] < 24576)
					{
			texno += (absN[X] > 8192) ? 2 : 1;
			*flip = (norm[Z] > 0) ? FLIPV : 0;
			*flip += ( ((norm[X] & -1) | 57344) > 0 ) ? FLIPH : 0;
					} else {
			texno += (absN[X] > 16384) ? 5 : 4;
			*flip = (norm[Z] > 0) ? FLIPV : 0;
			*flip += ( ((norm[X] & -1) | 57344) > 0 ) ? FLIPH : 0;
					}
		}
	}
	return baseTex+texno;
}

	//Helper function for a routine which uses per-polygon light processing.
	//This is different than the normal light processing in that it will get light data from any number of lights,
	//based only on the distance from the polygon to the light.
	/** SHOULD BE INLINED **/
__jo_force_inline int		per_polygon_light(PDATA * model, POINT wldPos, int polynumber)
{
	int luma = 0;
	for(int i = 0; i < 1; i++)
	{
	point_light * lightSrc = &active_lights[i];
			if(lightSrc->pop == 1)
			{
		POINT light_proxima = {
		(( ( (model->pntbl[model->pltbl[polynumber].Vertices[0]][X] + model->pntbl[model->pltbl[polynumber].Vertices[2]][X])>>1)
		+ wldPos[X]) + lightSrc->location[X])>>16,
		(( ( (model->pntbl[model->pltbl[polynumber].Vertices[0]][Y] + model->pntbl[model->pltbl[polynumber].Vertices[2]][Y])>>1)
		+ wldPos[Y]) + lightSrc->location[Y])>>16,
		(( ( (model->pntbl[model->pltbl[polynumber].Vertices[0]][Z] + model->pntbl[model->pltbl[polynumber].Vertices[2]][Z])>>1)
		+ wldPos[Z]) + lightSrc->location[Z])>>16
		};
		int inverted_proxima = (65536 / ( (light_proxima[X] * light_proxima[X]) +
				(light_proxima[Y] * light_proxima[Y]) +
				(light_proxima[Z] * light_proxima[Z]) ) )>>1;

		luma += inverted_proxima * (int)lightSrc->bright;
			}
	}
	return luma;
}


void	update_hmap(MATRIX msMatrix, FIXED * lightSrc)
{ //Master SH2 drawing function (needs to be sorted after by slave)

	//static int startOffset;
	//startOffset = (main_map_total_pix>>1) - (main_map_x_pix * (LCL_MAP_PIX>>1)) - (LCL_MAP_PIX>>1);
	static int rowOffset;
	static int x_pix_sample;
	static int y_pix_sample;
	// static int src_pix;
	// static int rowLimit;
	// static int RightBoundPixel;
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
	short colorBank;
//
//The only way to increase the render distance is with a non-uniform density.
//
	
	
	//Loop Concept:
	//SO just open Paint.NET make a 255x255 image.
	//Then with the selection tool, drag a box exactly 25x25 in size.
	//Then move the box around. That's exactly that this does.
	//Well, maybe that's the simple version.
	//It writes the pixels in that area to polygons, as the polygon's Y value.
	//We also do vertice transformation here.
	
	//The SCU-DSP does this now [including the division, lol].
	
			//y_pix_sample = (you.dispPos[Y] * (main_map_x_pix));
		//for(int k = 0; k < LCL_MAP_PIX; k++){
			//rowOffset = (k * main_map_x_pix) + startOffset;
			//rowLimit = rowOffset / main_map_x_pix;
			//for(int v = 0; v < LCL_MAP_PIX; v++){
			//	x_pix_sample = v + rowOffset + you.dispPos[X];
			//	RightBoundPixel = (x_pix_sample - (rowLimit * main_map_x_pix));
			//	src_pix = x_pix_sample - y_pix_sample;
			//	dst_pix = v+(k*LCL_MAP_PIX);
			//	if(src_pix < main_map_total_pix && src_pix >= 0 && RightBoundPixel < main_map_x_pix && RightBoundPixel >= 0){
			//polymap->pntbl[dst_pix][Y] = -(main_map[src_pix]<<16);
			//	} else { //Fill Set Value if outside of map area
			//polymap->pntbl[dst_pix][Y] = -(127<<16);
			//	}
		for(dst_pix = 0; dst_pix < (LCL_MAP_PIX * LCL_MAP_PIX); dst_pix++)
		{
			dspReturnTgt = dsp_output_addr[dst_pix];
			if(dspReturnTgt >= 0){
			polymap->pntbl[dst_pix][Y] = -(main_map[dspReturnTgt]<<16);
				} else { //Fill Set Value if outside of map area
			polymap->pntbl[dst_pix][Y] = -(127<<16);
				}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Vertice 3D Transformation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /**calculate z**/
        msh2VertArea[dst_pix].pnt[Z] = trans_pt_by_component(polymap->pntbl[dst_pix], m2z);
		msh2VertArea[dst_pix].pnt[Z] = (msh2VertArea[dst_pix].pnt[Z] > nearP) ? msh2VertArea[dst_pix].pnt[Z] : nearP;
 
         /**Starts the division**/
        SetFixDiv(MsScreenDist, msh2VertArea[dst_pix].pnt[Z]);

        /**Calculates X and Y while waiting for screenDist/z **/
        msh2VertArea[dst_pix].pnt[Y] = trans_pt_by_component(polymap->pntbl[dst_pix], m1y);
        msh2VertArea[dst_pix].pnt[X] = trans_pt_by_component(polymap->pntbl[dst_pix], m0x);
		
        /** Retrieves the result of the division **/
		msh2VertArea[dst_pix].inverseZ = *DVDNTL;

        /**Transform X and Y to screen space**/
        msh2VertArea[dst_pix].pnt[X] = fxm(msh2VertArea[dst_pix].pnt[X], msh2VertArea[dst_pix].inverseZ)>>SCR_SCALE_X;
        msh2VertArea[dst_pix].pnt[Y] = fxm(msh2VertArea[dst_pix].pnt[Y], msh2VertArea[dst_pix].inverseZ)>>SCR_SCALE_Y;
 
        //Screen Clip Flags for on-off screen decimation
		msh2VertArea[dst_pix].clipFlag = (JO_ABS(msh2VertArea[dst_pix].pnt[X]) > JO_TV_WIDTH_2) ? 1 : 0; //Simplified to increase CPU performance
		msh2VertArea[dst_pix].clipFlag |= (JO_ABS(msh2VertArea[dst_pix].pnt[Y]) > JO_TV_HEIGHT_2) ? 1 : 0; 
			
			
			}	// Row Filler Loop End Stub
		//} // Row Selector Loop End Stub
		
    transVerts[0] += LCL_MAP_PIX * LCL_MAP_PIX;

			//Loop explanation:
			//When we retrieve the map from the CD, we calculate the normals of every "potential" polygon in the map and write them to NormTbl.
			//The map is forced to be odd in both dimensionsm (X and Y) to force there to be an objective "center" pixel. This is neccessary so we can find our place in the map,
			//for the above loop.
			//However, this complicates assigning our "compressed" normals to polygons.
			//The local map is an odd number too (25x25 right now). The polygon maps are always even then, the local polymap being 24x24 and the big map being (map_dimension-1)^2.
			//So we have to do some mathematical gymnastics to find something we can represent as the center of the polygonal map.
			//Very important is to find the local polygon 0 ("top left") to use as the offset for our location.
			//So how do we define the integer center of even numbers? Well, we can't but we do have a guide:
			//The vertice map does have a center. So we define our center as the polygon which has the center vertice as its first vertice [vertice 0 of 0-1-2-3].
			// currently, 12 represents the polymap_width>>1
			
			//We also draw the polygons here.
			
			int poly_center = ((main_map_total_poly>>1) + 1 + ((main_map_x_pix-1)>>1)); //This polygon contains the center pixel (main_map_total_pix>>1)
			int poly_offset = (poly_center - ((main_map_x_pix-1) * (LCL_MAP_PLY>>1))) - (LCL_MAP_PLY>>1); //This is the upper-left polygon of the display area
			int src_norm = 0;
			int dst_poly = 0;
			VECTOR tempNorm = {0, 0, 0}; //Temporary normal used so the normal read does not have to be written again
			
			vertex_t * ptv[5] = {0, 0, 0, 0, 0}; //5th value used as temporary vert ID
			int flip = 0; //Temporary flip value used as the texture's flip characteristic so we don't have to write it back to memory
			int texno = 0; //Ditto
			
			y_pix_sample = ((you.dispPos[Y]) * (main_map_x_pix-1));
		for(int k = 0; k < LCL_MAP_PLY; k++){
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Row Selection
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			rowOffset = (k * (main_map_x_pix-1)) + poly_offset;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			for(int v = 0; v < LCL_MAP_PLY; v++){
				
				dst_poly = v+(k*LCL_MAP_PLY);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Backface & Screenspace Culling Section
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		ptv[0] = &msh2VertArea[polymap->pltbl[dst_poly].Vertices[0]];
		ptv[1] = &msh2VertArea[polymap->pltbl[dst_poly].Vertices[1]];
		ptv[2] = &msh2VertArea[polymap->pltbl[dst_poly].Vertices[2]];
		ptv[3] = &msh2VertArea[polymap->pltbl[dst_poly].Vertices[3]];
		
		//Components of screen-space cross-product used for backface culling.
		//Vertice order hint:
		// 0 - 1
		// 3 - 2
		//A cross-product can tell us if it's facing the screen. If it is not, we do not want it.
		register int cross0 = (ptv[1]->pnt[X] - ptv[3]->pnt[X])
							* (ptv[0]->pnt[Y] - ptv[2]->pnt[Y]);
		register int cross1 = (ptv[1]->pnt[Y] - ptv[3]->pnt[Y])
							* (ptv[0]->pnt[X] - ptv[2]->pnt[X]);
		//Sorting target. Uses max.
		register int zDepthTgt = JO_MAX(
		JO_MAX(ptv[0]->pnt[Z], ptv[2]->pnt[Z]),
		JO_MAX(ptv[1]->pnt[Z], ptv[3]->pnt[Z]));
		register int onScrn = (ptv[0]->clipFlag & ptv[2]->clipFlag & ptv[1]->clipFlag & ptv[3]->clipFlag);
		
		if((cross0 >= cross1) || onScrn || zDepthTgt <= nearP || zDepthTgt >= farP || msh2SentPolys[0] >= MAX_MSH2_SENT_POLYS){ continue; }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//HMAP Normal/Texture Finder
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				x_pix_sample = (v + rowOffset + (you.dispPos[X]));
				
				src_norm = (x_pix_sample - y_pix_sample) * 3; //*3 because there are X,Y,Z components

				tempNorm[X] = normTbl[src_norm]<<9;
				tempNorm[Y] = normTbl[src_norm+1]<<9;
				tempNorm[Z] = normTbl[src_norm+2]<<9;

		texno = tex2Handler(dst_poly, tempNorm, &flip);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Flip logic to keep vertice 0 on-screen
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 		if( (ptv[0]->clipFlag - ptv[3]->clipFlag) > 0 ){ //Vertical flip // Expresses clip0 > 0 && clip3 <= 0
			//Incoming Arrangement:
			// 0 - 1		^
			//-------- Edge | Y-
			// 3 - 2		|
			//				
            ptv[4] = ptv[3]; ptv[3] = ptv[0]; ptv[0] = ptv[4];
            ptv[4] = ptv[1]; ptv[1] = ptv[2]; ptv[2] = ptv[4];
            flip ^= 1<<5; //sprite flip value [v flip]
			//Outgoing Arrangement:
			// 3 - 2		^
			//-------- Edge | Y-
			// 0 - 1		|
		} else if( (ptv[0]->clipFlag - ptv[1]->clipFlag) > 0){//H flip // Expresses clip0 > 0 && clip1 <= 0
			//Incoming Arrangement:
			//	0 | 1
			//	3 | 2
			//	 Edge  ---> X+
            ptv[4] = ptv[1];  ptv[1]=ptv[0];  ptv[0] = ptv[4];
            ptv[4] = ptv[2];  ptv[2]=ptv[3];  ptv[3] = ptv[4];
            flip ^= 1<<4; //sprite flip value [h flip]
			//Outgoing Arrangement:
			// 1 | 0
			// 2 | 3
			//	Edge	---> X+
		}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Lighting
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		luma = per_polygon_light(polymap, hmap_actual_pos, dst_poly);
		luma = (luma < 0) ? 0 : luma; //We set the minimum luma as zero so the dynamic light does not corrupt the global light's basis.
		luma += fxdot(tempNorm, active_lights[0].ambient_light); //In normal "vision" however, bright light would do that..
		//Use transformed normal as shade determinant
		colorBank = (luma >= 98294) ? 0 : 1;
		colorBank = (luma < 49152) ? 2 : colorBank;
		colorBank = (luma < 32768) ? 3 : colorBank; 
		colorBank = (luma < 16384) ? 515 : colorBank; //Make really dark? use MSB shadow
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        msh2SetCommand(ptv[0]->pnt, ptv[1]->pnt,
					ptv[2]->pnt, ptv[3]->pnt,
                     2 | (flip), ( 5264 ), //Reads flip value
                     pcoTexDefs[texno].SRCA, colorBank<<6, pcoTexDefs[texno].SIZE, 0, zDepthTgt);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			}	// Row Filler Loop End Stub
		} // Row Selector Loop End Stub
		
		transPolys[0] += LCL_MAP_PLY * LCL_MAP_PLY;

	*sysbool = true;
}


void	hmap_cluster(void)
{
	
	chg_map(&maps[0]);
	//update_hmap();
	draw_minimap();
	update_mmap_1pass();
	
}
