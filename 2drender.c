//
//	2drender.c
//

#include "render.h"

_sprite sprWorkList[64];

void	add_to_sprite_list(FIXED * position, short span, short texno, unsigned char mesh, char type, int time)
{
	sprWorkList[*sprite_list_size].time = time;
	sprWorkList[*sprite_list_size].pos[X] = position[X];
	sprWorkList[*sprite_list_size].pos[Y] = position[Y];
	sprWorkList[*sprite_list_size].pos[Z] = position[Z];
	sprWorkList[*sprite_list_size].span = span;
	sprWorkList[*sprite_list_size].texno = texno;
	sprWorkList[*sprite_list_size].mesh = mesh;
	sprWorkList[*sprite_list_size].type = type;
	*sprite_list_size += 1;
}

void	transform_mesh_point(FIXED * mpt, FIXED * opt, _boundBox * mpara)
{
    MATRIX newMtx;

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
	
	/*
	Matrix Transformation / Perspective Correciton
	*/
        /**calculate z**/
        ssh2VertArea[0].pnt[Z] = trans_pt_by_component(spr->pos, m2z);
		ssh2VertArea[0].pnt[Z] = (ssh2VertArea[0].pnt[Z] > nearP) ? ssh2VertArea[0].pnt[Z] : nearP;

         /**Starts the division**/
        SetFixDiv(MsScreenDist, ssh2VertArea[0].pnt[Z]);

        /**Calculates X and Y while waiting for screenDist/z **/
        ssh2VertArea[0].pnt[Y] = trans_pt_by_component(spr->pos, m1y);
        ssh2VertArea[0].pnt[X] = trans_pt_by_component(spr->pos, m0x);
		
        /** Retrieves the result of the division **/
		int inverseZ = *DVDNTL;

        /**Transform X and Y to screen space**/
        ssh2VertArea[0].pnt[X] = fxm(ssh2VertArea[0].pnt[X], inverseZ)>>SCR_SCALE_X;
        ssh2VertArea[0].pnt[Y] = fxm(ssh2VertArea[0].pnt[Y], inverseZ)>>SCR_SCALE_Y;
 
        //Screen Clip Flags for on-off screen decimation
		//Simplified to increase CPU performance
		ssh2VertArea[0].clipFlag = ((ssh2VertArea[0].pnt[X]) > JO_TV_WIDTH_2) ? SCRN_CLIP_X : 0; 
		ssh2VertArea[0].clipFlag |= ((ssh2VertArea[0].pnt[X]) < -JO_TV_WIDTH_2) ? SCRN_CLIP_NX : ssh2VertArea[0].clipFlag; 
		ssh2VertArea[0].clipFlag |= ((ssh2VertArea[0].pnt[Y]) > JO_TV_HEIGHT_2) ? SCRN_CLIP_Y : ssh2VertArea[0].clipFlag;
		ssh2VertArea[0].clipFlag |= ((ssh2VertArea[0].pnt[Y]) < -JO_TV_HEIGHT_2) ? SCRN_CLIP_NY : ssh2VertArea[0].clipFlag;
		//ssh2VertArea[0].clipFlag |= ((ssh2VertArea[0].pnt[Z]) <= nearP) ? CLIP_Z : ssh2VertArea[0].clipFlag;
	
		transVerts[0] += 1;
		
		//If the vertice is off-screen, return.
		if(ssh2VertArea[0].clipFlag) return;
		int used_span = (spr->span * inverseZ)>>16;
		FIXED pntA[2] = {ssh2VertArea[0].pnt[X] + used_span, ssh2VertArea[0].pnt[Y] + used_span};
		FIXED pntC[2] = {ssh2VertArea[0].pnt[X] - used_span, ssh2VertArea[0].pnt[Y] - used_span};
		
        ssh2SetCommand(pntA, 0, pntC, 0,
		1 /*Scaled Sprite, no zoom point*/, 5264 | (spr->mesh<<8) /*64 color bank, HSS, scrn clip*/, 
		pcoTexDefs[spr->texno].SRCA, 2<<6, pcoTexDefs[spr->texno].SIZE, 0, ssh2VertArea[0].pnt[Z]);
}

