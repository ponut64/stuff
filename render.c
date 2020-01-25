#include "render.h"
#include "anorm.h"

 int * DVSR = ( int*)0xFFFFFF00;
 int * DVDNTH = ( int*)0xFFFFFF10;
 int * DVDNTL = ( int*)0xFFFFFF14;

SPRITE * localSprBuf = (SPRITE *)0x060D5B60;

//Write-only register! of CRAMoffset[1], bits 6,5,4 control SPR layer offset.
unsigned short * vdp2_CRAMoffset = (unsigned short *)(0x1800E4 + VDP2_RAMBASE);
unsigned short * vdp2_TVmode = (unsigned short *)(0x180000 + VDP2_RAMBASE);
unsigned short * vdp2_shadow = (unsigned short *)(0x1800E2 + VDP2_RAMBASE);
unsigned short * vdp2_sprMode = (unsigned short *)(0x1800E0 + VDP2_RAMBASE);
unsigned short * vdp2BackHigh = (unsigned short *)(0x1800AC + VDP2_RAMBASE);
unsigned short * vdp2BackLow = (unsigned short *)(0x1800AE + VDP2_RAMBASE);
//

vertex_t * ssh2VertArea;
vertex_t * msh2VertArea;
animationControl * AnimArea;
paletteCode * pcoTexDefs; //Defined with a LWRAM address in lwram.c

int dummy[4];
int * ssh2SentPolys;
int * msh2SentPolys;
int * transVerts;
int * transPolys;
int anims;

	FIXED	nearP	= 15<<16; // Z DISPLAY LEVEL [Actually a bit more complicated than that, but it works]
	FIXED	farP	= 1000<<16; // RENDER DIST
const unsigned short comm_p_mode = 5264; //Should add color mode table? Color mode is applied here.
const unsigned short ctrl = 2;

void	init_render_area(void){
	//Hello XL2. I bet you are wondering, why the hell are we uncaching this?
	//The theory is that these work areas are invariably going to be re-written every time a model is rendered.
	//This means the data cached in these is only useful within rendering a single model, not for multiple.
	//If you are rendering multiple small models, you'd rather fill the cache with pntbl and pltbl data.
	ssh2VertArea = (vertex_t *)((unsigned int)jo_malloc(650 * sizeof(vertex_t))|UNCACHE);
	msh2VertArea = (vertex_t *)((unsigned int)jo_malloc(850 * sizeof(vertex_t)));
	//
	AnimArea = (animationControl *)((unsigned int)jo_malloc(16 * sizeof(animationControl))|UNCACHE);
	
	localSprBuf = (SPRITE *)((unsigned int)jo_malloc( (2 * sizeof(SPRITE)) ));
	ssh2SentPolys = (int *)(((unsigned int)&dummy[0])|UNCACHE);
	msh2SentPolys = (int *)(((unsigned int)&dummy[1])|UNCACHE);
	transVerts = (int *)(((unsigned int)&dummy[2])|UNCACHE);
	transPolys = (int *)(((unsigned int)&dummy[3])|UNCACHE);
}

void	vblank_requirements(void)
{
	//jo_printf(0, 15, "(%x)", (int)BACK_CRAM);
	vdp2_CRAMoffset[1] = 16; //Moves SPR layer color banks up in color RAM by 256 entries.
	vdp2_TVmode[0] = 33027; //Set VDP2 to 704x224 [progressive scan, 704 width] - why? VDP2 will sharpen VDP1's output.
	vdp2_shadow[0] = 511;
	vdp2_sprMode[0] = 3; //Sprite Data Type Mode
}

void	frame_render_prep(void)
{	
		ssh2SentPolys[0] = 0;
		msh2SentPolys[0] = 0;
		transVerts[0] = 0;
		transPolys[0] = 0;
		anims = 0;
}

FIXED	trans_pt_by_component(POINT ptx, FIXED * normal)
{
	register FIXED transPt;
	asm(
		"clrmac;"
		"mov %[ptptr], r0;" //Moves pt ptr to discrete registers to prevent the C-level variable from being modified.
		"mov %[nmptr], r1;" //Calls the nmptr to a discrete register to "encourage" C to refresh the pointer every function call
		"mac.l @r0+,@r1+;"	//Denoting @r0+ is "data pointed to by data in register r0, r0+=(sizeof(instr_depth))"
		"mac.l @r0+,@r1+;"	//where instr_depth can be byte for mov.b , 2 bytes for mov.w, 4 bytes for mov.l
		"mac.l @r0+,@r1+;"	//MAC also stands for "multiply and accumulate" - you can probably figure that one out
		"sts MACH,r0;"		//This, by the way, is representative of matrix[comp] * pt ->
		"sts MACL,%[ox];"	//e.g. matrix[X][X] * pt[X] + matrix[Y][X] * pt[Y] + matrix[Z][X] * pt[Z] + matrix[pos][X]
		"xtrct r0,%[ox];"
		"mov.l @r1,r0;"		//This last move and add is the matrix component position
		"add r0,%[ox];"
		: 	[ox] "=r" (transPt)											//OUT
		:	[ptptr] "r" (ptx) ,	[nmptr] "r" (normal)					//IN
		:	"r0" , "r1"													//CLOBBERS
	);
	return transPt;
}							

//Set data in registers for division unit.
void		SetFixDiv(FIXED dividend, FIXED divisor) //Defined as "dividend / divisor", for fixed points, using division unit
{

/*
SH7604 Manual Information:

The 64-bit dividend is set in dividend registers H and L (DVDNTH and DVDNTL).
First set the value in DVDNTH. When a value is written to DVDNTL, the 64-bit ÷ 32-bit operation begins.
After the operation, the 32-bit remainder is written to DVDNTH and the 32-bit quotient is written to DVDNTL.

[ME:]These registers can only be accessed via pointers. . . because our compiler is not aware of them.
*/	

DVSR[0] = divisor;
DVDNTH[0] = (dividend>>16);
DVDNTL[0] = (dividend<<16);
	
}

void		ssh2SetCommand(FIXED * p1, FIXED * p2, FIXED * p3, FIXED * p4, Uint16 cmdctrl,
                                Uint16 cmdpmod, Uint16 cmdsrca, Uint16 cmdcolr,
                                Uint16 cmdsize, Uint16 cmdgrda, FIXED drawPrty) {

	SPRITE_T * user_sprite = (SPRITE_T *)&SpriteBuf[ssh2SentPolys[0] + MAX_MSH2_SENT_POLYS];
	ssh2SentPolys[0]++;

   user_sprite->CTRL = cmdctrl;
   user_sprite->LINK = 0x3000;
   user_sprite->PMOD = cmdpmod;
   user_sprite->SRCA = cmdsrca; //TEXTURE ADDRESS IN VDP1 VRAM
   user_sprite->COLR = cmdcolr; //COLOR BANK CODE IN COLOR BANK MODES, DRAW COLOR IN UNTEXTURED MODES, LUT ADDRESS IN CL16LK, IGNORED RGB
   user_sprite->SIZE = cmdsize; //VALID FOR TEXTURE DRAW COMMANDS

   user_sprite->XA=p1[X];
   user_sprite->YA=p1[Y];
   user_sprite->XB=p2[X];
   user_sprite->YB=p2[Y];
   user_sprite->XC=p3[X];
   user_sprite->YC=p3[Y];
   user_sprite->XD=p4[X];
   user_sprite->YD=p4[Y];
   user_sprite->DMMY=(drawPrty>>16);

   /**Important : Only the slave CPU is allowed to read/write the z sort buffer**/
   //IMPORTANT: We have to use the "far" screen. This is the "128". Why? Someone got angry when I asked why...
   //CRITICAL!! You MUST change sl_def to include the SPRITE_T structure. It is the same as SPRITE except with a Uint32 pointer, NEXT.
   //You must also change SpriteBuf and SpriteBuf2 to be of SPRITE_T * type.
    Uint32 ** Zentry = (Uint32**)(Zbuffer + (128 + ((user_sprite->DMMY>>8)))*4  ); //Get Z distance as entry into Z buffer
    user_sprite->NEXT=*Zentry; //Link current polygon to last entry at that Z distance in Zbuffer
    *Zentry=(void*)user_sprite; //Make last entry at that Z distance this entry
}

void		msh2SetCommand(FIXED * p1, FIXED * p2, FIXED * p3, FIXED * p4, Uint16 cmdctrl,
                                Uint16 cmdpmod, Uint16 cmdsrca, Uint16 cmdcolr,
                                Uint16 cmdsize, Uint16 cmdgrda, FIXED drawPrty) {

	SPRITE_T * user_sprite = (SPRITE_T *)&SpriteBuf[msh2SentPolys[0]];
	msh2SentPolys[0]++;

   user_sprite->CTRL = cmdctrl;
   user_sprite->LINK = 0x3000;
   user_sprite->PMOD = cmdpmod;
   user_sprite->SRCA = cmdsrca; //TEXTURE ADDRESS IN VDP1 VRAM
   user_sprite->COLR = cmdcolr; //COLOR BANK CODE IN COLOR BANK MODES, DRAW COLOR IN UNTEXTURED MODES, LUT ADDRESS IN CL16LK, IGNORED RGB
   user_sprite->SIZE = cmdsize; //VALID FOR TEXTURE DRAW COMMANDS

   user_sprite->XA=p1[X];
   user_sprite->YA=p1[Y];
   user_sprite->XB=p2[X];
   user_sprite->YB=p2[Y];
   user_sprite->XC=p3[X];
   user_sprite->YC=p3[Y];
   user_sprite->XD=p4[X];
   user_sprite->YD=p4[Y];
   user_sprite->DMMY=(drawPrty>>16);
   

}

void	sort_master_polys(void)
{
	SPRITE_T * user_sprite;
	
	for(int i = 0; i < msh2SentPolys[0]; i++)
	{
	user_sprite = (SPRITE_T *)&SpriteBuf[i];
		
   /**Important : Only the slave CPU is allowed to read/write the z sort buffer**/
   //IMPORTANT: We have to use the "far" screen. This is the "128". Why? Someone got angry when I asked why...
   //CRITICAL!! You MUST change sl_def to include the SPRITE_T structure. It is the same as SPRITE except with a Uint32 pointer, NEXT.
   //You must also change SpriteBuf and SpriteBuf2 to be of SPRITE_T * type.
    Uint32 ** Zentry = (Uint32**)(Zbuffer + (128 + ((user_sprite->DMMY>>8)))*4  ); //Get Z distance as entry into Z buffer
    user_sprite->NEXT=*Zentry; //Link current polygon to last entry at that Z distance in Zbuffer [*Zentry is a pointer to the last polygon]
    *Zentry=(void*)user_sprite; //Make last entry at that Z distance this entry [*Zentry becomes a pointer to this polygon]
	}
}

void ssh2DrawModel(entity_t * ent, POINT lightSrc) //Primary variable sorting rendering
{
	if(ent->file_done != true){return;}
	//Recommended, for performance, that large entities be placed in HWRAM.
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

    PDATA * model = ent->pol[0];
    if ( (transVerts[0]+model->nbPoint) >= INTERNAL_MAX_VERTS) return;
    if ( (transPolys[0]+model->nbPolygon) >= INTERNAL_MAX_POLY) return;
	
	FIXED luma;
	short colorBank;
 
    for (int i = 0; i < model->nbPoint; i++)
    {
        /**calculate z**/
        ssh2VertArea[i].pnt[Z] = trans_pt_by_component(model->pntbl[i], m2z);
		ssh2VertArea[i].pnt[Z] = (ssh2VertArea[i].pnt[Z] > nearP) ? ssh2VertArea[i].pnt[Z] : nearP;

         /**Starts the division**/
        SetFixDiv(MsScreenDist, ssh2VertArea[i].pnt[Z]);

        /**Calculates X and Y while waiting for screenDist/z **/
        ssh2VertArea[i].pnt[Y] = trans_pt_by_component(model->pntbl[i], m1y);
        ssh2VertArea[i].pnt[X] = trans_pt_by_component(model->pntbl[i], m0x);
		
        /** Retrieves the result of the division **/
		ssh2VertArea[i].inverseZ = *DVDNTL;

        /**Transform X and Y to screen space**/
        ssh2VertArea[i].pnt[X] = fxm(ssh2VertArea[i].pnt[X], ssh2VertArea[i].inverseZ)>>SCR_SCALE_X;
        ssh2VertArea[i].pnt[Y] = fxm(ssh2VertArea[i].pnt[Y], ssh2VertArea[i].inverseZ)>>SCR_SCALE_Y;
 
        //Screen Clip Flags for on-off screen decimation
		ssh2VertArea[i].clipFlag = (JO_ABS(ssh2VertArea[i].pnt[X]) > JO_TV_WIDTH_2) ? 1 : 0; //Simplified to increase CPU performance
		ssh2VertArea[i].clipFlag |= (JO_ABS(ssh2VertArea[i].pnt[Y]) > JO_TV_HEIGHT_2) ? 1 : 0; 
    }

    transVerts[0] += model->nbPoint;

	vertex_t * ptv[5] = {0, 0, 0, 0, 0};
	int flip = 0;

    /**POLYGON PROCESSING**/ 
		//Duplicate loops for sorting types.
		//WHY: Maximum performance
if( (model->attbl[0].sort & 3) == SORT_MAX)
{
    for (int i = 0; i < model->nbPolygon; i++)
    {
		ptv[0] = &ssh2VertArea[model->pltbl[i].Vertices[0]];
		ptv[1] = &ssh2VertArea[model->pltbl[i].Vertices[1]];
		ptv[2] = &ssh2VertArea[model->pltbl[i].Vertices[2]];
		ptv[3] = &ssh2VertArea[model->pltbl[i].Vertices[3]];
		flip = model->attbl[i].dir;
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
		register int onScrn = (ptv[0]->clipFlag & ptv[2]->clipFlag &
										ptv[1]->clipFlag & ptv[3]->clipFlag);
 
		if((cross0 >= cross1 && model->attbl[i].flag == 0) || zDepthTgt <= nearP || zDepthTgt >= farP ||
		onScrn || ssh2SentPolys[0] >= MAX_SSH2_SENT_POLYS){ continue; }
		//Goal: Flip the polygon so that vertice 0 is in the render area // This is too costly on the CPU and has been removed.
/* 		if( (ptv[0]->clipFlag - ptv[3]->clipFlag) > 0 ){ //Vertical flip // Expresses clip0 > 0 && clip3 <= 0
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
		} */
		//Preclipping is always enabled
		//Transform the polygon's normal by light source vector
		luma = fxdot(model->pltbl[i].norm, lightSrc);
		//Use transformed normal as shade determinant
		colorBank = (luma < -32768) ? 0 : 1;
		colorBank = (luma > 16384) ? 2 : colorBank;
		colorBank = (luma > 32768) ? 3 : colorBank; 
		colorBank = (luma > 49152) ? 515 : colorBank; //Make really dark? use MSB shadow
		
        ssh2SetCommand(ptv[0]->pnt,
		ptv[1]->pnt,
		ptv[2]->pnt,
		ptv[3]->pnt,
                     ctrl | (flip), (comm_p_mode | (model->attbl[i].atrb & 33024)), //Reads flip value, mesh enable, and msb bit
                     pcoTexDefs[model->attbl[i].texno].SRCA, colorBank<<6, pcoTexDefs[model->attbl[i].texno].SIZE, 0, zDepthTgt);
    } //Sort Max Endif
} else if((model->attbl[0].sort & 3) == SORT_MIN) //Sort Minimum
{
    for (int i = 0; i < model->nbPolygon; i++)
    {
		ptv[0] = &ssh2VertArea[model->pltbl[i].Vertices[0]];
		ptv[1] = &ssh2VertArea[model->pltbl[i].Vertices[1]];
		ptv[2] = &ssh2VertArea[model->pltbl[i].Vertices[2]];
		ptv[3] = &ssh2VertArea[model->pltbl[i].Vertices[3]];
		flip = model->attbl[i].dir;
		//Components of screen-space cross-product used for backface culling.
		//Vertice order hint:
		// 0 - 1
		// 3 - 2
		//A cross-product can tell us if it's facing the screen. If it is not, we do not want it.
		register int cross0 = (ptv[1]->pnt[X] - ptv[3]->pnt[X])
							* (ptv[0]->pnt[Y] - ptv[2]->pnt[Y]);
		register int cross1 = (ptv[1]->pnt[Y] - ptv[3]->pnt[Y])
							* (ptv[0]->pnt[X] - ptv[2]->pnt[X]);
		//Sorting target. Uses min.
		register int zDepthTgt = JO_MIN(
		JO_MIN(ptv[0]->pnt[Z], ptv[2]->pnt[Z]),
		JO_MIN(ptv[1]->pnt[Z], ptv[3]->pnt[Z]));
		register int onScrn = (ptv[0]->clipFlag | ptv[2]->clipFlag |
										ptv[1]->clipFlag | ptv[3]->clipFlag);
 
		if((cross0 >= cross1 && model->attbl[i].flag == 0) || zDepthTgt <= nearP || zDepthTgt >= farP ||
		onScrn || ssh2SentPolys[0] >= MAX_SSH2_SENT_POLYS){ continue; }
		//Goal: Flip the polygon so that vertice 0 is in the render area // This is too costly on the CPU and has been removed.
/* 		if( (ptv[0]->clipFlag - ptv[3]->clipFlag) > 0 ){ //Vertical flip // Expresses clip0 > 0 && clip3 <= 0
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
		} */
		//Preclipping is always enabled
		//Transform the polygon's normal by light source vector
		luma = fxdot(model->pltbl[i].norm, lightSrc);
		//Use transformed normal as shade determinant
		colorBank = (luma < -32768) ? 0 : 1;
		colorBank = (luma > 16384) ? 2 : colorBank;
		colorBank = (luma > 32768) ? 3 : colorBank; 
		colorBank = (luma > 49152) ? 515 : colorBank; //Make really dark? use MSB shadow

        ssh2SetCommand(ptv[0]->pnt,
		ptv[1]->pnt,
		ptv[2]->pnt,
		ptv[3]->pnt,
                     ctrl | (flip), (comm_p_mode | (model->attbl[i].atrb & 33024)), //Reads flip value, mesh enable, and msb bit
                     pcoTexDefs[model->attbl[i].texno].SRCA, colorBank<<6, pcoTexDefs[model->attbl[i].texno].SIZE, 0, zDepthTgt);
	}
} else {//Sort Min endif
    for (int i = 0; i < model->nbPolygon; i++) //Sort Center-ish
    {
		ptv[0] = &ssh2VertArea[model->pltbl[i].Vertices[0]];
		ptv[1] = &ssh2VertArea[model->pltbl[i].Vertices[1]];
		ptv[2] = &ssh2VertArea[model->pltbl[i].Vertices[2]];
		ptv[3] = &ssh2VertArea[model->pltbl[i].Vertices[3]];
		flip = model->attbl[i].dir;
		//Components of screen-space cross-product used for backface culling.
		//Vertice order hint:
		// 0 - 1
		// 3 - 2
		//A cross-product can tell us if it's facing the screen. If it is not, we do not want it.
		register int cross0 = (ptv[1]->pnt[X] - ptv[3]->pnt[X])
							* (ptv[0]->pnt[Y] - ptv[2]->pnt[Y]);
		register int cross1 = (ptv[1]->pnt[Y] - ptv[3]->pnt[Y])
							* (ptv[0]->pnt[X] - ptv[2]->pnt[X]);
		//Sorting target. Uses average of top-left and bottom-right. Adding logic to change sorting per-polygon HAMMERS performance in unacceptable ways.
		register int zDepthTgt = (ptv[0]->pnt[Z] + ptv[2]->pnt[Z])>>1;
 
		if((cross0 >= cross1 && model->attbl[i].flag == 0) || zDepthTgt <= nearP || zDepthTgt >= farP ||
		((ptv[0]->clipFlag &
		ptv[2]->clipFlag) == 1) ||
		ssh2SentPolys[0] >= MAX_SSH2_SENT_POLYS){ continue; }
		//Goal: Flip the polygon so that vertice 0 is in the render area // This is too costly on the CPU and has been removed.
/* 		if( (ptv[0]->clipFlag - ptv[3]->clipFlag) > 0 ){ //Vertical flip // Expresses clip0 > 0 && clip3 <= 0
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
		} */
		//Preclipping is always enabled
		//Transform the polygon's normal by light source vector
		luma = fxdot(model->pltbl[i].norm, lightSrc);
		//Use transformed normal as shade determinant
		colorBank = (luma < -32768) ? 0 : 1;
		colorBank = (luma > 16384) ? 2 : colorBank;
		colorBank = (luma > 32768) ? 3 : colorBank; 
		colorBank = (luma > 49152) ? 515 : colorBank; //Make really dark? use MSB shadow

        ssh2SetCommand(ptv[0]->pnt,
		ptv[1]->pnt,
		ptv[2]->pnt,
		ptv[3]->pnt,
                     ctrl | (flip), (comm_p_mode | (model->attbl[i].atrb & 33024)), //Reads flip value, mesh enable, and msb bit
                     pcoTexDefs[model->attbl[i].texno].SRCA, colorBank<<6, pcoTexDefs[model->attbl[i].texno].SIZE, 0, zDepthTgt);
    }
		}			
		transPolys[0] += model->nbPolygon;

}

inline void msh2DrawModel(entity_t * ent, MATRIX msMatrix, FIXED * lightSrc) //Master SH2 drawing function (needs to be sorted after by slave)
{
	if(ent->file_done != true){return;}
	//Recommended, for performance, that large entities be placed in HWRAM.
	/**WARNING: DO NOT USE SGL MATRIX SYSTEM FOR MASTER DRAWING**/
	/*
	SGL matrix pointer is uncached. Matrix operations will be confused between master/slave
	You will have to come up with your own matrix system, it's not too bad ->
	You can capture the matrix at the start of the frame instead, before the slave does any matrix ops
	MSH2 + SSH2 drawing is not the ideal. In other words, using 1 CPU at full tap and then only if you must spill over to the second.
	Diminishing returns might be a better phrase. 
	But there is no system here to do that. Using MSH2 does help; so perhaps throw a known quantity at it, rather than any dynamic entity list.
	*/
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
	
    PDATA * model = ent->pol[0];
    if ( (transVerts[0]+model->nbPoint) >= INTERNAL_MAX_VERTS) return;
    if ( (transPolys[0]+model->nbPolygon) >= INTERNAL_MAX_POLY) return;
	
	FIXED luma;
	short colorBank;

    for (int i = 0; i < model->nbPoint; i++)
    {
        /**calculate z**/
        msh2VertArea[i].pnt[Z] = trans_pt_by_component(model->pntbl[i], m2z);
		msh2VertArea[i].pnt[Z] = (msh2VertArea[i].pnt[Z] > nearP) ? msh2VertArea[i].pnt[Z] : nearP;
 
         /**Starts the division**/
        SetFixDiv(MsScreenDist, msh2VertArea[i].pnt[Z]);

        /**Calculates X and Y while waiting for screenDist/z **/
        msh2VertArea[i].pnt[Y] = trans_pt_by_component(model->pntbl[i], m1y);
        msh2VertArea[i].pnt[X] = trans_pt_by_component(model->pntbl[i], m0x);
		
        /** Retrieves the result of the division **/
		msh2VertArea[i].inverseZ = *DVDNTL;

        /**Transform X and Y to screen space**/
        msh2VertArea[i].pnt[X] = fxm(msh2VertArea[i].pnt[X], msh2VertArea[i].inverseZ)>>SCR_SCALE_X;
        msh2VertArea[i].pnt[Y] = fxm(msh2VertArea[i].pnt[Y], msh2VertArea[i].inverseZ)>>SCR_SCALE_Y;
 
        //Screen Clip Flags for on-off screen decimation
		msh2VertArea[i].clipFlag = (JO_ABS(msh2VertArea[i].pnt[X]) > JO_TV_WIDTH_2) ? 1 : 0; //Simplified to increase CPU performance
		msh2VertArea[i].clipFlag |= (JO_ABS(msh2VertArea[i].pnt[Y]) > JO_TV_HEIGHT_2) ? 1 : 0; 
    }

    transVerts[0] += model->nbPoint;

	vertex_t * ptv[5] = {0, 0, 0, 0, 0};
	int flip = 0;

    /**POLYGON PROCESSING**/ 
    for (int i = 0; i < model->nbPolygon; i++)
    {
		ptv[0] = &msh2VertArea[model->pltbl[i].Vertices[0]];
		ptv[1] = &msh2VertArea[model->pltbl[i].Vertices[1]];
		ptv[2] = &msh2VertArea[model->pltbl[i].Vertices[2]];
		ptv[3] = &msh2VertArea[model->pltbl[i].Vertices[3]];
		flip = model->attbl[i].dir;
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
 
		if((cross0 >= cross1 && model->attbl[i].flag == 0) || zDepthTgt <= nearP || zDepthTgt >= farP ||
		onScrn || msh2SentPolys[0] >= MAX_MSH2_SENT_POLYS){ continue; }
		//Goal: Flip the polygon so that vertice 0 is in the render area // This is too costly on the CPU and has been removed.
/* 		if( (ptv[0]->clipFlag - ptv[3]->clipFlag) > 0 ){ //Vertical flip // Expresses clip0 > 0 && clip3 <= 0
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
		} */
		//Preclipping is always enabled
		//Transform the polygon's normal by light source vector
		luma = fxdot(model->pltbl[i].norm, lightSrc);
		//Use transformed normal as shade determinant
		colorBank = (luma < -59000) ? 0 : 1;
		colorBank = (luma > -50000) ? 2 : colorBank;
		colorBank = (luma > -45000) ? 3 : colorBank;
		colorBank = (luma > 0) ? 515 : colorBank; //Make really dark? use MSB shadow
		// colorBank = (luma < -32768) ? 0 : 1;
		// colorBank = (luma > 16384) ? 2 : colorBank;
		// colorBank = (luma > 32768) ? 3 : colorBank; 
		// colorBank = (colorBank < 4) ? colorBank+1 : 0;

        msh2SetCommand(ptv[0]->pnt, ptv[1]->pnt,
									ptv[2]->pnt,
						ptv[3]->pnt,
                     ctrl | (flip), (comm_p_mode | (model->attbl[i].atrb & 33024)), //Reads flip value, mesh enable, and msb bit
                     pcoTexDefs[model->attbl[i].texno].SRCA, colorBank<<6, pcoTexDefs[model->attbl[i].texno].SIZE, 0, zDepthTgt);
    }
		transPolys[0] += model->nbPolygon;

}

void ssh2DrawAnimation(animationControl * animCtrl, entity_t * ent, POINT lightSrc) //Draws animted model via SSH2
{
	if(ent->file_done != true){return;}
	//WARNING:
	//Once an entity is drawn animated, *all* instances of that entity must be drawn animated, or else they will not reset the pntbl appropriately.
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

    PDATA * model = ent->pol[0];
    if ( (transVerts[0]+model->nbPoint) >= INTERNAL_MAX_VERTS) return;
    if ( (transPolys[0]+model->nbPolygon) >= INTERNAL_MAX_POLY) return;
	
	FIXED luma;
	short colorBank;
	
	//Process for static pose change:
	//1. Check if both animations are static poses [if arate of startFrm is 0 or if startFrm == endFrm]
	//2. Set currentFrm to the AnimArea startFrm<<3
	//3. Set uniforn to false
	//4. Set the local arate to 4
	//5. Set currentKeyFrm to the AnimArea startFrm
	//6. set the nextKeyFrame to the animCtrl startFrm
	//7. Interpolate once
	//8. Return all control data as if set from the animCtrl pose

//
	unsigned char localArate;
	unsigned char nextKeyFrm;
	int frDelta;
	compVert * curKeyFrame;
	compVert * nextKeyFrame;
    /**Sets the animation data**/
///Variable interpolation set
if(AnimArea[anims].uniform == false){
localArate = animCtrl->arate[AnimArea[anims].currentKeyFrm];
} else {
	localArate = 2;
}


	AnimArea[anims].currentFrm += (localArate * framerate)>>1;

   AnimArea[anims].currentKeyFrm = (AnimArea[anims].currentFrm>>3);
    if (AnimArea[anims].currentKeyFrm >= AnimArea[anims].endFrm)    {
        AnimArea[anims].currentFrm -= (AnimArea[anims].endFrm - AnimArea[anims].startFrm)<<3;
        AnimArea[anims].currentKeyFrm = AnimArea[anims].currentFrm>>3;
     } else if(AnimArea[anims].currentKeyFrm < AnimArea[anims].startFrm){
		AnimArea[anims].currentKeyFrm = AnimArea[anims].startFrm;
		AnimArea[anims].currentFrm += (AnimArea[anims].endFrm-AnimArea[anims].startFrm)<<3;
	}
    nextKeyFrm = AnimArea[anims].currentKeyFrm+1;

    if (nextKeyFrm >= AnimArea[anims].endFrm){
        nextKeyFrm = AnimArea[anims].startFrm;
	} else if (nextKeyFrm <= AnimArea[anims].startFrm){
        nextKeyFrm = AnimArea[anims].startFrm+1;
	}

	
 if(AnimArea[anims].startFrm != animCtrl->startFrm && AnimArea[anims].endFrm != animCtrl->endFrm) 
 {
	//For single-frame interpolation between poses
	curKeyFrame = (compVert*)ent->animation[AnimArea[anims].currentKeyFrm]->cVert;
	nextKeyFrame = (compVert*)ent->animation[animCtrl->startFrm]->cVert;
	frDelta = 8;
 } else {
	//For interpolation inside keyframed animation
	curKeyFrame = (compVert*)ent->animation[AnimArea[anims].currentKeyFrm]->cVert;
	nextKeyFrame = (compVert*)ent->animation[nextKeyFrm]->cVert;
	///Don't touch this! **absolute** frame delta 
	frDelta = (AnimArea[anims].currentFrm)-(AnimArea[anims].currentKeyFrm<<3);
 }


//Animation Data
    Sint32 * dst = model->pntbl[0]; //This pointer is incremented by the animation interpolator.
    short * src = curKeyFrame[0];
    short * nxt = nextKeyFrame[0];
 
    for (int i = 0; i < model->nbPoint; i++)
    {
		/**Uncompress the vertices and apply linear interpolation**/
		*dst++=( *src + ((( *nxt - *src) * frDelta)>>4))<<8;
		*src++;
		*nxt++;
		*dst++=( *src + ((( *nxt - *src) * frDelta)>>4))<<8;
		*src++;
		*nxt++;
		*dst++=( *src + ((( *nxt - *src) * frDelta)>>4))<<8;
		*src++;
		*nxt++;
		//
        /**calculate z**/
        ssh2VertArea[i].pnt[Z] = trans_pt_by_component(model->pntbl[i], m2z);
		ssh2VertArea[i].pnt[Z] = (ssh2VertArea[i].pnt[Z] > nearP) ? ssh2VertArea[i].pnt[Z] : nearP;
 
         /**Starts the division**/
        SetFixDiv(MsScreenDist, ssh2VertArea[i].pnt[Z]);

        /**Calculates X and Y while waiting for screenDist/z **/
        ssh2VertArea[i].pnt[Y] = trans_pt_by_component(model->pntbl[i], m1y);
        ssh2VertArea[i].pnt[X] = trans_pt_by_component(model->pntbl[i], m0x);
		
        /** Retrieves the result of the division **/
		ssh2VertArea[i].inverseZ = *DVDNTL;

        /**Transform X and Y to screen space**/
        ssh2VertArea[i].pnt[X] = fxm(ssh2VertArea[i].pnt[X], ssh2VertArea[i].inverseZ)>>SCR_SCALE_X;
        ssh2VertArea[i].pnt[Y] = fxm(ssh2VertArea[i].pnt[Y], ssh2VertArea[i].inverseZ)>>SCR_SCALE_Y;
 
        //Screen Clip Flags for on-off screen decimation
		ssh2VertArea[i].clipFlag = (JO_ABS(ssh2VertArea[i].pnt[X]) > JO_TV_WIDTH_2) ? 1 : 0; //Simplified to increase CPU performance
		ssh2VertArea[i].clipFlag |= (JO_ABS(ssh2VertArea[i].pnt[Y]) > JO_TV_HEIGHT_2) ? 1 : 0; 
    }

    transVerts[0] += model->nbPoint;

    dst = (Sint32 *)&model->pltbl[0];
    Uint8 *src2 = ent->animation[AnimArea[anims].currentKeyFrm]->cNorm; //A new 1-byte src
	VECTOR tNorm = {0, 0, 0};
	
	vertex_t * ptv[5] = {0, 0, 0, 0, 0};
	int flip = 0;

    /**POLYGON PROCESSING**/ 
    for (int i = 0; i < model->nbPolygon; i++)
    {
		ptv[0] = &ssh2VertArea[model->pltbl[i].Vertices[0]];
		ptv[1] = &ssh2VertArea[model->pltbl[i].Vertices[1]];
		ptv[2] = &ssh2VertArea[model->pltbl[i].Vertices[2]];
		ptv[3] = &ssh2VertArea[model->pltbl[i].Vertices[3]];
		flip = model->attbl[i].dir;
		//Components of screen-space cross-product used for backface culling.
		//Vertice order hint:
		// 0 - 1
		// 3 - 2
		//A cross-product can tell us if it's facing the screen. If it is not, we do not want it.
		register int cross0 = (ptv[1]->pnt[X] - ptv[3]->pnt[X])
							* (ptv[0]->pnt[Y] - ptv[2]->pnt[Y]);
		register int cross1 = (ptv[1]->pnt[Y] - ptv[3]->pnt[Y])
							* (ptv[0]->pnt[X] - ptv[2]->pnt[X]);
		//Sorting target. Uses average of top-left and bottom-right. Adding logic to change sorting per-polygon HAMMERS performance in unacceptable ways.
		register int zDepthTgt = (ptv[0]->pnt[Z] + ptv[2]->pnt[Z])>>1;

		src2 += (i != 0) ? 1 : 0; //Add to compressed normal pointer address, always, but only after the first polygon
 
		if((cross0 >= cross1 && model->attbl[i].flag == 0) || zDepthTgt <= nearP || zDepthTgt >= farP ||
		((ptv[0]->clipFlag & ptv[2]->clipFlag) == 1) ||
		ssh2SentPolys[0] >= MAX_SSH2_SENT_POLYS){ continue; }
/* 		//Goal: Flip the polygon so that vertice 0 is in the render area // This is too costly on the CPU and has been removed.
		if( (ptv[0]->clipFlag - ptv[3]->clipFlag) > 0 ){ //Vertical flip // Expresses clip0 > 0 && clip3 <= 0
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
		} */
		//Preclipping is always enabled
		//New normals in from animation normal table // These are not written back to memory
        tNorm[X]=ANORMS[*src2][X];
        tNorm[Y]=ANORMS[*src2][Y];
        tNorm[Z]=ANORMS[*src2][Z];
		//Transform the polygon's normal by light source vector
		luma = fxdot(tNorm, lightSrc);
		//Use transformed normal as shade determinant
		colorBank = (luma < -32768) ? 0 : 1;
		colorBank = (luma > 16384) ? 2 : colorBank;
		colorBank = (luma > 32768) ? 3 : colorBank; 
		colorBank = (luma > 49152) ? 515 : colorBank; //Make really dark? use MSB shadow

        ssh2SetCommand(ptv[0]->pnt,
		ptv[1]->pnt,
		ptv[2]->pnt,
		ptv[3]->pnt,
                     ctrl | (flip), (comm_p_mode | (model->attbl[i].atrb & 33024)), //Reads flip value, mesh enable, and msb bit
                     pcoTexDefs[model->attbl[i].texno].SRCA, colorBank<<6, pcoTexDefs[model->attbl[i].texno].SIZE, 0, zDepthTgt);
    }
		transPolys[0] += model->nbPolygon;
		
 if(AnimArea[anims].startFrm != animCtrl->startFrm && AnimArea[anims].endFrm != animCtrl->endFrm) // Check to see if the animation matches.
 {
	AnimArea[anims].uniform = animCtrl->uniform;
	AnimArea[anims].currentFrm = animCtrl->startFrm<<3;
	AnimArea[anims].startFrm = animCtrl->startFrm;
	AnimArea[anims].endFrm = animCtrl->endFrm;
 }
		
		anims++; //Increment animation work area pointer

}


