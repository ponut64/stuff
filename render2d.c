//
//	2drender.c
//

#include <sl_def.h>
#include "def.h"
#include "mloader.h"
#include "mymath.h"
#include "bounder.h"

#include "render.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

char sprintf_buffer[256];	
int baseAsciiTexno = 0;
int sprAsciiWidth = 0;
int sprAsciiHeight = 0;

short	add_to_sprite_list(FIXED * position, short * span, short texno, unsigned short colorBank, unsigned char mesh, char type, short useClip, int lifetime)
{
	short used_sprite = -1;
	//Find an unused sprite list entry
	for(short i = 0; i < MAX_SPRITES; i++)
	{
		if(sprWorkList[i].type == 'N')
		{
			used_sprite = i;
			break;
		}
	}
	if(used_sprite == -1) return used_sprite;
	
	sprWorkList[used_sprite].lifetime = lifetime;
	sprWorkList[used_sprite].pos[X] = position[X];
	sprWorkList[used_sprite].pos[Y] = position[Y];
	sprWorkList[used_sprite].pos[Z] = position[Z];
	sprWorkList[used_sprite].span[X] = span[X];
	sprWorkList[used_sprite].span[Y] = span[Y];
	sprWorkList[used_sprite].span[Z] = span[Z];
	sprWorkList[used_sprite].texno = texno;
	sprWorkList[used_sprite].colorBank = colorBank;
	sprWorkList[used_sprite].mesh = mesh;
	sprWorkList[used_sprite].type = type;
	sprWorkList[used_sprite].useClip = useClip;
	return used_sprite;
}

void	transform_mesh_point(FIXED * mpt, FIXED * opt, _boundBox * mpara)
{
	FIXED m0x[4];
	FIXED m1y[4];
	FIXED m2z[4];
	
	m0x[0] = mpara->UVX[X];
	m0x[1] = mpara->UVY[X];
	m0x[2] = mpara->UVZ[X];
	m0x[3] = 0;
	
	m1y[0] = mpara->UVX[Y];
	m1y[1] = mpara->UVY[Y];
	m1y[2] = mpara->UVZ[Y];
	m1y[3] = 0;
	
	m2z[0] =  mpara->UVX[Z];
	m2z[1] =  mpara->UVY[Z];
	m2z[2] =  mpara->UVZ[Z];
	m2z[3] =  0;
	
	opt[X] = trans_pt_by_component(mpt, m0x);
	opt[Y] = trans_pt_by_component(mpt, m1y);
	opt[Z] = trans_pt_by_component(mpt, m2z);
	transVerts[0]++;
}

//solid_or_border : 0 for solid, 1 for border
//mesh: 0 for solid, 1 for mesh
void	draw2dSquare(int * firstPt, int * scndPt, unsigned short colorData, unsigned short solid_or_border, unsigned short depth, unsigned short mesh)
{
	if(msh2SentPolys[0] >= MAX_MSH2_SENT_POLYS) return;
	//
	// Draw a 2D square.
	// Draws using VDP1 single-color polygon.
	// Safe to use on either MSH2 or SSH2.
	// Note: ONLY set coordinates inside the screen.
	//
	int topLeft[2] = {firstPt[X] - TV_HALF_WIDTH, firstPt[Y] - TV_HALF_HEIGHT};
	int btmRight[2] = {scndPt[X] - TV_HALF_WIDTH, scndPt[Y] - TV_HALF_HEIGHT};
	
	topLeft[X] = CLIP_TO_SCRN_X(topLeft[X]);
	btmRight[X] = CLIP_TO_SCRN_X(btmRight[X]);

	topLeft[Y] = CLIP_TO_SCRN_Y(topLeft[Y]);
	btmRight[Y] = CLIP_TO_SCRN_Y(btmRight[Y]);
	
	int topRight[2] = {btmRight[X], topLeft[Y]};
	int bottomLeft[2] = {topLeft[X], btmRight[Y]};
	msh2SetCommand(topLeft, topRight, btmRight, bottomLeft,
	4 + (solid_or_border & 1) /*Polygon or Polyline Draw Command*/, 0x8E8 | (mesh << 8) /* manual specifies 0xC0 for polygon, DO NOT CHANGE BITS 7-6*/,
	0 /* No source data, is untextured */, colorData,
	0 /*no cmdsize, is untextured*/, 0 /* no gouraud */, depth<<16 /* z */);
}


void	ssh2BillboardScaledSprite(_sprite * spr)
{
	//
	// Draw Billboard Scaled Sprite
	// Recieves a 3D position in a matrix, transforms it to screenspace, and then displays a sprite.
	//
    static MATRIX newMtx;
    slGetMatrix(newMtx);

	static FIXED m0x[4];
	static FIXED m1y[4];
	static FIXED m2z[4];
	
	m0x[0] = newMtx[X][X];
	m0x[1] = newMtx[Y][X];
	m0x[2] = newMtx[Z][X];
	m0x[3] = newMtx[3][X];
	
	m1y[0] = newMtx[X][Y];
	m1y[1] = newMtx[Y][Y];
	m1y[2] = newMtx[Z][Y];
	m1y[3] = newMtx[3][Y];
	
	m2z[0] = newMtx[X][Z];
	m2z[1] = newMtx[Y][Z];
	m2z[2] = newMtx[Z][Z];
	m2z[3] = newMtx[3][Z];
	
	unsigned short usrClp = SYS_CLIPPING; //The clipping setting added to command table
	if(spr->useClip == USER_CLIP_INSIDE)
	{
		//Clip inside the user clipping setting
		usrClp = 0x400;
	} else if(spr->useClip == USER_CLIP_OUTSIDE)
	{
		//Clip outside the user clipping setting
		usrClp = 0x600;
	}
	
	/*
	Matrix Transformation / Perspective Correciton
	*/
        /**calculate z**/
        ssh2VertArea[0].pnt[Z] = trans_pt_by_component(spr->pos, m2z);
		ssh2VertArea[0].pnt[Z] = (ssh2VertArea[0].pnt[Z] > NEAR_PLANE_DISTANCE) ? ssh2VertArea[0].pnt[Z] : NEAR_PLANE_DISTANCE;

         /**Starts the division**/
        SetFixDiv(scrn_dist, ssh2VertArea[0].pnt[Z]);

        /**Calculates X and Y while waiting for screenDist/z **/
        ssh2VertArea[0].pnt[Y] = trans_pt_by_component(spr->pos, m1y);
        ssh2VertArea[0].pnt[X] = trans_pt_by_component(spr->pos, m0x);
		
        /** Retrieves the result of the division **/
		int inverseZ = *DVDNTL;

        /**Transform X and Y to screen space**/
        ssh2VertArea[0].pnt[X] = fxm(ssh2VertArea[0].pnt[X], inverseZ)>>SCR_SCALE_X;
        ssh2VertArea[0].pnt[Y] = fxm(ssh2VertArea[0].pnt[Y], inverseZ)>>SCR_SCALE_Y;
 
        //Screen Clip Flags for on-off screen decimation
		clipping(&ssh2VertArea[0], spr->useClip);
	
		transVerts[0] += 1;
		int spanX;
		int spanY;
		
		
		if(spr->type == SPRITE_TYPE_BILLBOARD)
		{
			//ssh2VertArea[0].clipFlag &= SCRN_CLIP_FLAGS; //Ignore Z clipping for this (?)
			//If the vertice is off-screen or too far away, return.
			if(ssh2VertArea[0].clipFlag || ssh2VertArea[0].pnt[Z] > FAR_PLANE_DISTANCE) return;
			spanX = (spr->span[X] * inverseZ)>>16;
			spanY = (spr->span[Y] * inverseZ)>>16;
			FIXED pntA[2] = {ssh2VertArea[0].pnt[X] + spanX, ssh2VertArea[0].pnt[Y] + spanY};
			FIXED pntC[2] = {ssh2VertArea[0].pnt[X] - spanX, ssh2VertArea[0].pnt[Y] - spanY};
			
			ssh2SetCommand(pntA, 0, pntC, 0,
			1 /*Scaled Sprite, no zoom point*/, 0x1090 | (spr->mesh<<8) | usrClp /*64 color bank, HSS, enable/disable usr clip*/, 
			pcoTexDefs[spr->texno].SRCA, spr->colorBank, pcoTexDefs[spr->texno].SIZE, 0, ssh2VertArea[0].pnt[Z]);
		} else if(spr->type == SPRITE_TYPE_UNSCALED_BILLBOARD)
		{
			//If the vertice is off-screen, return. Note does not stop if too far away.
			if(ssh2VertArea[0].clipFlag) return;
			spanX = ((pcoTexDefs[spr->texno].SIZE & 0x3F00)>>5)>>1;
			spanY = (pcoTexDefs[spr->texno].SIZE & 0xFF)>>1;
			FIXED pntA[2] = {ssh2VertArea[0].pnt[X] - spanX, ssh2VertArea[0].pnt[Y] - spanY};
			
			ssh2SetCommand(pntA, 0, 0, 0,
			0 /*Normal Sprite*/, 0x1090 | (spr->mesh<<8) | usrClp /*64 color bank, HSS, enable/disable usr clip*/, 
			pcoTexDefs[spr->texno].SRCA, spr->colorBank, pcoTexDefs[spr->texno].SIZE, 0, 1<<16);
		}

}

void	ssh2Line(_sprite * spr)
{
	//
	// Draw A Line
	// Recieves a 3D position in a matrix, transforms it to screenspace, and then displays a line with pos + span as the line.
	//
    static MATRIX newMtx;
    slGetMatrix(newMtx);

	static FIXED m0x[4];
	static FIXED m1y[4];
	static FIXED m2z[4];
	
	m0x[0] = newMtx[X][X];
	m0x[1] = newMtx[Y][X];
	m0x[2] = newMtx[Z][X];
	m0x[3] = newMtx[3][X];
	
	m1y[0] = newMtx[X][Y];
	m1y[1] = newMtx[Y][Y];
	m1y[2] = newMtx[Z][Y];
	m1y[3] = newMtx[3][Y];
	
	m2z[0] = newMtx[X][Z];
	m2z[1] = newMtx[Y][Z];
	m2z[2] = newMtx[Z][Z];
	m2z[3] = newMtx[3][Z];
	
	unsigned short usrClp = SYS_CLIPPING; //The clipping setting added to command table
	if(spr->useClip == USER_CLIP_INSIDE)
	{
		//Clip inside the user clipping setting
		usrClp = 0x400;
	} else if(spr->useClip == USER_CLIP_OUTSIDE)
	{
		//Clip outside the user clipping setting
		usrClp = 0x600;
	}
	
	/*
	Matrix Transformation / Perspective Correciton
	*/
	static POINT used_pos;
	used_pos[X] = spr->pos[X];
	used_pos[Y] = spr->pos[Y];
	used_pos[Z] = spr->pos[Z];
	for(int i = 0; i < 2; i++)
	{
        /**calculate z**/
        ssh2VertArea[i].pnt[Z] = trans_pt_by_component(used_pos, m2z);
		ssh2VertArea[i].pnt[Z] = (ssh2VertArea[i].pnt[Z] > NEAR_PLANE_DISTANCE) ? ssh2VertArea[0].pnt[Z] : NEAR_PLANE_DISTANCE;

         /**Starts the division**/
        SetFixDiv(scrn_dist, ssh2VertArea[i].pnt[Z]);

        /**Calculates X and Y while waiting for screenDist/z **/
        ssh2VertArea[i].pnt[Y] = trans_pt_by_component(used_pos, m1y);
        ssh2VertArea[i].pnt[X] = trans_pt_by_component(used_pos, m0x);
		
        /** Retrieves the result of the division **/
		int inverseZ = *DVDNTL;

        /**Transform X and Y to screen space**/
        ssh2VertArea[i].pnt[X] = fxm(ssh2VertArea[i].pnt[X], inverseZ)>>SCR_SCALE_X;
        ssh2VertArea[i].pnt[Y] = fxm(ssh2VertArea[i].pnt[Y], inverseZ)>>SCR_SCALE_Y;
 
        //Screen Clip Flags for on-off screen decimation
		clipping(&ssh2VertArea[i], spr->useClip);
		//ssh2VertArea[i].clipFlag &= SCRN_CLIP_FLAGS; //Ignore Z clipping for this stuff.... could just make a new clipper func..
		used_pos[X] += spr->span[X]<<4;
		used_pos[Y] += spr->span[Y]<<4;
		used_pos[Z] += spr->span[Z]<<4;
	}
		transVerts[0] += 2;
		
		//If the vertice is off-screen or too far away, return.
		if(ssh2VertArea[0].clipFlag || ssh2VertArea[0].pnt[Z] > FAR_PLANE_DISTANCE) return;
		int used_z = (spr->type == SPRITE_TYPE_3DLINE) ? ssh2VertArea[0].pnt[Z] : 1<<16;
        ssh2SetCommand(ssh2VertArea[0].pnt, ssh2VertArea[0].pnt, ssh2VertArea[1].pnt, ssh2VertArea[1].pnt,
		5 /*Polyline Setting*/, 0x8C0 | usrClp/*manual specifies 0xC0 for polyline, DO NOT CHANGE BITS 7-6*/,
		0 /*SRCA*/, spr->colorBank /*COLOR BANK CODE*/, 0 /*CMDSIZE*/, 0 /*GR ADDR*/, used_z /*Z*/);
}

void	ssh2NormalSprite(_sprite * spr)
{
	int ptv[XY] = {spr->pos[X] - TV_HALF_WIDTH, spr->pos[Y] - TV_HALF_HEIGHT};
	//Let sprite type allow mesh strobing, bank strobing, or on/off strobing.
	//Let span[X] determine strobe status (strobe on, strobe off) 0 or 1.
	//Let span[Y] determine strobe interval, in sub-second unit times.
	//Let span[Z] hold a rolling timer (from 0 to 1 seconds) for the strobe.
	
	//To be modified by blink strobes
	unsigned short used_size = pcoTexDefs[spr->texno].SIZE;
	//To be modified by flash strobes
	unsigned short used_colrbnk = spr->colorBank;
	
	if(spr->type == SPRITE_MESH_STROBE || spr->type == SPRITE_FLASH_STROBE || spr->type == SPRITE_BLINK_STROBE)
	{
		unsigned short * tlimit = (unsigned short *)&spr->span[Y];
		unsigned short * tcur = (unsigned short *)&spr->span[Z];
		if(*tcur > *tlimit)
		{
			*tcur = 0;
			spr->span[X] = (spr->span[X]) ? 0 : 1;
			spr->extra += 1;
		}
		*tcur += delta_time;
		
		if(spr->type == SPRITE_BLINK_STROBE)
		{
			used_size = (spr->span[X]) ? 0 : used_size;
		} else if(spr->type == SPRITE_MESH_STROBE)
		{
			spr->mesh = (spr->span[X]) ? 0 : 1;
		} else if(spr->type == SPRITE_FLASH_STROBE)
		{
			used_colrbnk |= (spr->span[X]) ? 0x8000 : 0;
		}
	}
	
	ssh2SetCommand(ptv, 0, 0, 0, 0 /*CMD CTRL*/, 0x890 | (spr->mesh<<8) /*COMMAND MODES*/, 
				pcoTexDefs[spr->texno].SRCA /*SRCA*/, used_colrbnk /*COLOR BANK CODE*/,
				used_size /*CMDSIZE*/, 0 /*GR ADDR*/, 1<<16 /*Z*/
				);

}


	//(This function is safe to run on either master or slave)
void	drawAxis(POINT size)
{
	//The point of this function is, when placed in a matrix, will draw that matrix given its size as function arguments.
	//The matrix is drawn as polyline. (just lil thicker than a line)

    static MATRIX newMtx;
    slGetMatrix(newMtx);

	static FIXED m0x[4];
	static FIXED m1y[4];
	static FIXED m2z[4];
	
	m0x[0] = newMtx[X][X];
	m0x[1] = newMtx[Y][X];
	m0x[2] = newMtx[Z][X];
	m0x[3] = newMtx[3][X];
	
	m1y[0] = newMtx[X][Y];
	m1y[1] = newMtx[Y][Y];
	m1y[2] = newMtx[Z][Y];
	m1y[3] = newMtx[3][Y];
	
	m2z[0] = newMtx[X][Z];
	m2z[1] = newMtx[Y][Z];
	m2z[2] = newMtx[Z][Z];
	m2z[3] = newMtx[3][Z];

    if ( (transVerts[0]+3) >= INTERNAL_MAX_VERTS) return;
    if ( (transPolys[0]+3) >= INTERNAL_MAX_POLY) return;
 
	int iz = 0;
 
    for (int i = 0; i < 4; i++)
    {
		POINT target = {0, 0, 0};
		if(i == 0)
		{
			target[X] = size[X]; //No need to set Y and Z as they are declared back to 0.
		} else if(i == 1)
		{
			target[Y] = size[Y]; 
		} else if(i == 2)
		{
			target[Z] = size[Z];
		}
		//At 3, we just run zero.
		
        /**calculate z**/
        msh2VertArea[i].pnt[Z] = trans_pt_by_component(target, m2z);
		msh2VertArea[i].pnt[Z] = (msh2VertArea[i].pnt[Z] > NEAR_PLANE_DISTANCE) ? msh2VertArea[i].pnt[Z] : NEAR_PLANE_DISTANCE;

         /**Starts the division**/
        SetFixDiv(scrn_dist, msh2VertArea[i].pnt[Z]);

        /**Calculates X and Y while waiting for screenDist/z **/
        msh2VertArea[i].pnt[Y] = trans_pt_by_component(target, m1y);
        msh2VertArea[i].pnt[X] = trans_pt_by_component(target, m0x);
		
        /** Retrieves the result of the division **/
		iz = *DVDNTL;

        /**Transform X and Y to screen space**/
        msh2VertArea[i].pnt[X] = fxm(msh2VertArea[i].pnt[X], iz)>>SCR_SCALE_X;
        msh2VertArea[i].pnt[Y] = fxm(msh2VertArea[i].pnt[Y], iz)>>SCR_SCALE_Y;
 
        //Screen Clip Flags for on-off screen decimation
		msh2VertArea[i].clipFlag = (JO_ABS(msh2VertArea[i].pnt[X]) > TV_HALF_WIDTH) ? 1 : 0; //Simplified to increase CPU performance
		msh2VertArea[i].clipFlag |= (JO_ABS(msh2VertArea[i].pnt[Y]) > TV_HALF_HEIGHT) ? 1 : 0; 
    }
	
	 *transVerts += 4;
	
	vertex_t * ptv[5] = {0, 0, 0, 0, 0};
	
	unsigned short colindex = 31;

    for (int i = 0; i < 3; i++)
    {
		ptv[0] = &msh2VertArea[i];
		ptv[1] = &msh2VertArea[i];
		ptv[2] = &msh2VertArea[3];
		ptv[3] = &msh2VertArea[3];

		//Sorting target. Uses max.
		register int zDepthTgt = JO_MAX(
		JO_MAX(ptv[0]->pnt[Z], ptv[2]->pnt[Z]),
		JO_MAX(ptv[1]->pnt[Z], ptv[3]->pnt[Z]));
		register int onScrn = (ptv[0]->clipFlag & ptv[2]->clipFlag & ptv[1]->clipFlag & ptv[3]->clipFlag);
		
		if(zDepthTgt <= NEAR_PLANE_DISTANCE || zDepthTgt >= FAR_PLANE_DISTANCE ||
		onScrn || msh2SentPolys[0] >= MAX_MSH2_SENT_POLYS){ continue; }

        msh2SetCommand(ptv[0]->pnt, ptv[1]->pnt, ptv[2]->pnt, ptv[3]->pnt,
		5 /*Polyline Setting*/, 0x8C0 /*manual specifies 0xC0 for polyline, DO NOT CHANGE BITS 7-6*/,
		0 /*SRCA*/, colindex /*COLOR BANK CODE*/, 0 /*CMDSIZE*/, 0 /*GR ADDR*/, 3<<16 /*Z*/);
		colindex += 8;
    } //Sort Max Endif
	
	*transPolys += 3;
	
}

void	draw_normal_sprite(int xPos, int yPos, unsigned short texno, unsigned short colrBank)
{
	if(msh2SentPolys[0] >= MAX_MSH2_SENT_POLYS) return;
	int ptv[XY] = {xPos - TV_HALF_WIDTH, yPos - TV_HALF_HEIGHT};
	msh2SetCommand(ptv, 0, 0, 0, 0 /*CMD CTRL*/, 0x890 /*COMMAND MODES*/, 
				pcoTexDefs[texno].SRCA /*SRCA*/, colrBank /*COLOR BANK CODE*/,
				pcoTexDefs[texno].SIZE /*CMDSIZE*/, 0 /*GR ADDR*/, 1<<16 /*Z*/
				);
}

void	spr_print(int xPos, int yPos, char * data)
{
	int len = 0;
	int texIndex = 0;
	char nextChar = data[0];
	
	len = strlen(data);
	
	POINT ptv;
	
	ptv[X] = (xPos - TV_HALF_WIDTH);
	ptv[Y] = yPos - TV_HALF_HEIGHT;
	
	//Prints left to right
	for(int i = 0; i < len; i++)
	{
	nextChar = data[i];
	//Detect newlines
	if( nextChar == ('\n'))
	{
	ptv[Y] += sprAsciiHeight; //Jump a line
	ptv[X] = (xPos - TV_HALF_WIDTH); //Return to left bounds
	continue;
	}
	if(ptv[X] < -TV_HALF_WIDTH || ptv[X] > TV_HALF_WIDTH){
	ptv[Y] += sprAsciiHeight; //Jump a line
	ptv[X] = (xPos - TV_HALF_WIDTH); //Return to left bounds
	continue;
	}
	if(ptv[Y] < -TV_HALF_HEIGHT || ptv[Y] > TV_HALF_HEIGHT){
	//Exited screen. Stop.
	return;
	}
	// "baseAsciiTexno" is supplied by setting "baseAsciiTexno = numTex" then loading the ASCII texture table (WRAP_NewTable).
	texIndex = (baseAsciiTexno + nextChar)-32;
	if(msh2SentPolys[0] >= MAX_MSH2_SENT_POLYS) return;
	msh2SetCommand(ptv, 0, 0, 0, 0 /*CMD CTRL*/, 0x890 /*COMMAND MODES*/, 
				pcoTexDefs[(unsigned char)texIndex].SRCA /*SRCA*/, 2 /*COLOR BANK CODE*/,
				pcoTexDefs[(unsigned char)texIndex].SIZE /*CMDSIZE*/, 0 /*GR ADDR*/, 1<<16 /*Z*/
				);
	ptv[X] += sprAsciiWidth; //Some number added to xPos to distance the characters
	
	}

}	

// VDP1 Normal Sprite Text Routine using sprintf
// Allows user to print integers or hex codes together with text. Can also of course print addresses.
void	spr_sprintf(int xPos, int yPos, ...)
{
//I do not really understand how C does this, but it can do this and I like it. But I also hate it.
va_list vmlist;
va_start(vmlist, NULL);

do {
	snprintf(sprintf_buffer, 255, va_arg(vmlist, char *), va_arg(vmlist, int), va_arg(vmlist, int), va_arg(vmlist, int));
	spr_print(xPos, yPos, sprintf_buffer); 
	} while(0);
	
va_end(vmlist);
}

void	nbg_sprintf(int x, int y,  ...)
{
va_list vmlist;
va_start(vmlist, NULL);

do {
	snprintf(sprintf_buffer, 255, va_arg(vmlist, char *), va_arg(vmlist, int), va_arg(vmlist, int), va_arg(vmlist, int));
	slPrint(sprintf_buffer, slLocate(x,y)); 
	} while(0);
	
va_end(vmlist);
}

void	nbg_sprintf_decimal(int x, int y,  int print_data)
{
	static unsigned int decimal = 0;
	static int whole_number = 0;
	decimal = print_data & 0xFFFF;
	whole_number = print_data >> 16;
	//Trying to convert to decimals
	// 42949 = (65536) * (65536 / 100,000)
	// magic number: 42949
	// process: (number << 16)
	//			number / (int)(42949).
	decimal<<=16;
	decimal = decimal / 42949;
	nbg_sprintf(x, y, "(%i.%i)", whole_number, decimal);
}

void	spr_sprintf_decimal(int x, int y,  int print_data)
{
	static unsigned int decimal = 0;
	static int whole_number = 0;
	decimal = print_data & 0xFFFF;
	whole_number = print_data >> 16;
	//Trying to convert to decimals
	// 42949 = (65536) * (65536 / 100,000)
	// magic number: 42949
	// process: (number << 16)
	//			number / (int)(42949).
	decimal<<=16;
	decimal = decimal / 42949;
	spr_sprintf(x, y, "(%i.%i)", whole_number, decimal);
}


void	nbg_clear_text(void)
{
	for(int y = 0; y < 30; y++)
	{
		slPrint("                                            ", slLocate(0,y));
	}
}

//Creates a 2D menu on-screen using VDP1 with options:
short		menu_with_options(__basic_menu * mnu)
{
	static int btmRight[2] = {64, 128};
	mnu->btmRight = btmRight;
	
	if(mnu->option_grid[X] == 0 || mnu->option_grid[Y] == 0 || mnu->num_option == 0)
	{
		//No options....
		return 0;
	} 
	//Do not allow selection box to be more than the number of options, nor less than zero
	mnu->selection = (mnu->selection >= mnu->num_option) ? (mnu->num_option-1) : (mnu->selection < 0) ? 0 : mnu->selection;
	
	btmRight[X] = (mnu->scale[X] * mnu->option_grid[X]) + 8 + mnu->topLeft[X];
	btmRight[Y] = (mnu->scale[Y] * mnu->option_grid[Y]) + 8 + mnu->topLeft[Y];
	
	draw2dSquare(mnu->topLeft, btmRight, mnu->backColor, 0, 5, 0);
	
	int topLeftSelBox[2] = {0,0};
	int btmRiteSelBox[2] = {0,0};

	int position_of_option[2] = {0, 0};
	int offset_option[2] = {0,0};
	int start[2] = {(4) + mnu->topLeft[X], (mnu->scale[Y]>>1) + mnu->topLeft[Y]};
	int this_option = 0;
	for(int i = 0; i < mnu->option_grid[Y]; i++)
	{
		position_of_option[Y] = (i * mnu->scale[Y]) + start[Y];
		for(int j = 0; j < mnu->option_grid[X]; j++)
		{
		position_of_option[X] = (j * mnu->scale[X]) + start[X];
		if(mnu->selection == this_option)
		{
		topLeftSelBox[X] = (position_of_option[X]);
		topLeftSelBox[Y] = (position_of_option[Y]);
		btmRiteSelBox[X] = position_of_option[X] + (mnu->scale[X]);
		btmRiteSelBox[Y] = (position_of_option[Y] + 12);
		draw2dSquare(topLeftSelBox, btmRiteSelBox, mnu->optionColor, 1, 5, 0);
		}
		offset_option[X] = (position_of_option[X] + (mnu->scale[X]>>1)) - (((strlen(mnu->option_text[this_option])-1) * 8)>>1) - 4;
		spr_sprintf(offset_option[X], position_of_option[Y], mnu->option_text[this_option]);
		this_option++;
		if(this_option >= mnu->num_option) return mnu->num_option;
		}
	}
	
	return this_option;
}





