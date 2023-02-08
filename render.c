#include <sl_def.h>
#include "def.h"
#include "mloader.h"
#include "mymath.h"
#include "bounder.h"

#include "render.h"
#include "anorm.h"

 int * DVSR = ( int*)0xFFFFFF00;
 int * DVDNTH = ( int*)0xFFFFFF10;
 int * DVDNTL = ( int*)0xFFFFFF14;

SPRITE * localSprBuf = (SPRITE *)0x060D5B60;

vertex_t ssh2VertArea[MAX_SSH2_ENTITY_VERTICES];
vertex_t msh2VertArea[MAX_MSH2_ENTITY_VERTICES];
animationControl AnimArea[MAX_SIMULTANEOUS_ANIMATED_ENTITIES];
_sprite sprWorkList[MAX_SPRITES];
paletteCode * pcoTexDefs; //Defined with a LWRAM address in lwram.c
point_light light_host[MAX_DYNAMIC_LIGHTS];
point_light * active_lights;

//Just a debug list, not a hard limit.
entity_t * drawn_entity_list[64];
short drawn_entity_count;

int dummy[4];
int * ssh2SentPolys;
int * msh2SentPolys;
int * transVerts;
int * transPolys;
int anims; //Current active animation count; increments animation control data work array.
int scrn_dist; //Distance to projection screen surface

// Software clipping settings.
// Setting user clipping will also set these.
short vert_clip_x = TV_HALF_WIDTH;
short vert_clip_nx = -TV_HALF_WIDTH;
short vert_clip_y = TV_HALF_HEIGHT;
short vert_clip_ny = -TV_HALF_HEIGHT;


int send_draw_stats; //Setting for sending draw stats to screen. 0 = no stats; 1 = send by sprites; 2 = send by NBG text
	
// Framebuffer Erase Region Settings
unsigned short top_left_erase_pt = 0;
#ifdef USE_HI_RES
int hi_res_switch = 1;
unsigned short btm_rite_erase_pt = ((TV_WIDTH / 16) << 9) | (TV_HALF_HEIGHT);
#else
int hi_res_switch = 0;
unsigned short btm_rite_erase_pt = ((TV_WIDTH / 8) << 9) | (TV_HEIGHT);
#endif
	
/*

	Don't forget:
	UV coordinates are possible on Saturn with indexed color draw commands.
	The method by which this is possible is to put your texture in color RAM, and use goraud shading as texture coordinates.
	The draw command consequently should be untextured, as VDP1 is managing the texel via goraud shading.
	The color of the polygon should be .. hmm..
	Since goraud is additive, you should be able to locate the texel in color RAM by making the start address of the texel the color.
	I plan on experimenting with a 16x16 texture in color RAM so as to use this method.
	
	It seems like this is only possible with a texture width of 32.
	The goraud coordinates use red and blue, the first ten bits.
	Red and blue each are five bits, ergo, 32.
	If you increment blue (or coordinate Y) by 1, you still increment the integer coordinate by 32.
	So yes, a 16x16 is not possible. It would have to be 32x8 at the smallest.
*/

int get_screen_distance_from_fov(short desired_horizontal_fov)
{
	//Primitive: 90 - ((fov / 2) + 90)
	int scrn_half_angle = 8192 - (desired_horizontal_fov >> 1) + 8192;

	int scrn_scalar = slTan(scrn_half_angle);

	int scrndist = fxm(scrn_scalar, (TV_HALF_WIDTH)<<16);

	return scrndist;
}

void	init_render_area(short desired_horizontal_fov)
{

	for(int i = 0; i < MAX_SPRITES; i++)
	{
		//Mark the whole sprite list as unused.
		sprWorkList[i].type = 'N'; 
	}

	scrn_dist = get_screen_distance_from_fov(desired_horizontal_fov);
	ssh2SentPolys = (int *)(((unsigned int)&dummy[0])|UNCACHE);
	msh2SentPolys = (int *)(((unsigned int)&dummy[1])|UNCACHE);
	transVerts = (int *)(((unsigned int)&dummy[2])|UNCACHE);
	transPolys = (int *)(((unsigned int)&dummy[3])|UNCACHE);
	active_lights = (point_light *)(((unsigned int)&light_host[0])|UNCACHE);
}

void	frame_render_prep(void)
{	
		ssh2SentPolys[0] = 0;
		msh2SentPolys[0] = 0;
		transVerts[0] = 0;
		transPolys[0] = 0;
		anims = 0;
		drawn_entity_count = 0;
		vert_clip_x = TV_HALF_WIDTH;
		vert_clip_nx = -TV_HALF_WIDTH;
		vert_clip_y = TV_HALF_HEIGHT;
		vert_clip_ny = -TV_HALF_HEIGHT;
		
}

//Note: Set literal screen coordinates.
///Use: Cordon off a region of the screen for drawing static items.
///In this case, the static items do not need to be re-drawn every frame, and save VDP1 on performance.
void	setFramebufferEraseRegion(int xtl, int ytl, int xbr, int ybr)
{
		if(hi_res_switch){
	top_left_erase_pt = ((xtl>>4) << 9) | (ytl>>1);
	btm_rite_erase_pt = ((xbr>>4) << 9) | (ybr>>1);
		} else {
	top_left_erase_pt = ((xtl>>3) << 9) | (ytl);
	btm_rite_erase_pt = ((xbr>>3) << 9) | (ybr);
		}
}


FIXED	trans_pt_by_component(POINT ptx, FIXED * normal)
{
	volatile FIXED transPt;
	asm(
		"clrmac;"
		"mov %[ptptr], r0;" //Moves pt ptr to discrete s to prevent the C-level variable from being modified.
		"mov %[nmptr], r1;" //Calls the nmptr to a discrete  to "encourage" C to refresh the pointer every function call
		"mac.l @r0+,@r1+;"	//Denoting @r0+ is "data pointed to by data in  r0, r0+=(sizeof(instr_depth))"
		"mac.l @r0+,@r1+;"	//where instr_depth can be byte for mov.b , 2 bytes for mov.w, 4 bytes for mov.l
		"mac.l @r0+,@r1+;"	//MAC also stands for "multiply and accumulate" - you can probably figure that one out
		"sts MACH,r0;"		//This, by the way, is representative of matrix[comp] * pt ->
		"sts MACL,%[ox];"	//e.g. matrix[X][X] * pt[X] + matrix[Y][X] * pt[Y] + matrix[Z][X] * pt[Z] + matrix[pos][X]
		"xtrct r0,%[ox];"
		"mov.l @r1,r0;"		//This last move and add is the matrix component position
		"add r0,%[ox];"
		: 	[ox] "=r" (transPt)							//OUT
		:	[ptptr] "r" (ptx) ,	[nmptr] "r" (normal)	//IN
		:	"r0" , "r1", "mach", "macl"					//CLOBBERS
	);
	return transPt;
}							

//Set data in s for division unit.
void		SetFixDiv(FIXED dividend, FIXED divisor) //Defined as "dividend / divisor", for fixed points, using division unit
{

/*
SH7604 Manual Information:

The 64-bit dividend is set in dividend s H and L (DVDNTH and DVDNTL).
First set the value in DVDNTH. When a value is written to DVDNTL, the 64-bit ÷ 32-bit operation begins.
After the operation, the 32-bit remainder is written to DVDNTH and the 32-bit quotient is written to DVDNTL.

[ME:]These s can only be accessed via pointers. . . because our compiler is not aware of them.
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
   user_sprite->GRDA = cmdgrda;

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
    Uint32 ** Zentry = (Uint32**)(Zbuffer + (128 + ((user_sprite->DMMY>>3)))*4  ); //Get Z distance as entry into Z buffer
    user_sprite->NEXT=*Zentry; //Link current polygon to last entry at that Z distance in Zbuffer
    *Zentry=(void*)user_sprite; //Make last entry at that Z distance this entry
}

inline void		msh2SetCommand(FIXED * p1, FIXED * p2, FIXED * p3, FIXED * p4, Uint16 cmdctrl,
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
   user_sprite->GRDA = cmdgrda;

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
	if(send_draw_stats == 1)
	{
		unsigned short txt_base = (hi_res_switch) ? 400 : 180;
		spr_sprintf(8, txt_base, "Polygons in Scene:(%i)", transPolys[0]);
		spr_sprintf(8, txt_base+15, "Sent Commands:(%i)", ssh2SentPolys[0] + msh2SentPolys[0]);
		spr_sprintf(8, txt_base+30, "Transformed Verts:(%i)", transVerts[0]);
	} else if(send_draw_stats == 2)
	{
		unsigned short txt_base = (hi_res_switch) ? 1 : 2;
		nbg_sprintf(txt_base, 24, "TRPLY:(%i)", transPolys[0]);
		nbg_sprintf(txt_base, 25, "SNTPL:(%i)", ssh2SentPolys[0] + msh2SentPolys[0]);
		nbg_sprintf(txt_base, 26, "VERTS:(%i)", transVerts[0]);
	}
	
	SPRITE_T * user_sprite;
	
	for(int i = 0; i < msh2SentPolys[0]; i++)
	{
	user_sprite = (SPRITE_T *)&SpriteBuf[i];
		
   /**Important : Only the slave CPU is allowed to read/write the z sort buffer**/
   //IMPORTANT: We have to use the "far" screen. This is the "128". Why? Someone got angry when I asked why...
   //CRITICAL!! You MUST change sl_def to include the SPRITE_T structure. It is the same as SPRITE except with a Uint32 pointer, NEXT.
   //You must also change SpriteBuf and SpriteBuf2 to be of SPRITE_T * type.
    Uint32 ** Zentry = (Uint32**)(Zbuffer + (128 + ((user_sprite->DMMY>>3)))*4  ); //Get Z distance as entry into Z buffer
    user_sprite->NEXT=*Zentry; //Link current polygon to last entry at that Z distance in Zbuffer [*Zentry is a pointer to the last polygon]
    *Zentry=(void*)user_sprite; //Make last entry at that Z distance this entry [*Zentry becomes a pointer to this polygon]
	}
}

//If rendering a matrix-centered object (like a gun model or a third-person player model), set "negate coordinates" to Y.
int		process_light(VECTOR lightAngle, FIXED * ambient_light, int * brightness_floor, FIXED * prematrix, char model_purpose)
{
	
	//model_purpose  .. where 'P' means "PLAYER".
	
	/*
	PRE-PROCESSOR
	
	Find the nearest light in the active light list.
	The engine expects you to populate the light list with at least 1 light.
	It can be a point light, an ambient light, or both.
	
	*/
	int nearest_dot = 0;
	int active_dot = 0;
	VECTOR lightDist = {0, 0, 0};
	point_light * light_used = &active_lights[0];
	FIXED * wldPos = &prematrix[9];
	
	nearest_dot = JO_ABS(wldPos[X] - light_used->pos[X]) +
					JO_ABS(wldPos[Y] - light_used->pos[Y]) +
					JO_ABS(wldPos[Z] - light_used->pos[Z]);

	
		for(unsigned int i = 0; i < MAX_DYNAMIC_LIGHTS; i++)
		{
			if(active_lights[i].pop == 1)
				{
					// " Manhattan Distance "
					if(model_purpose == 'P')
					{
		active_dot = JO_ABS(wldPos[X] - active_lights[i].pos[X]) +
					JO_ABS(wldPos[Y] - active_lights[i].pos[Y]) +
					JO_ABS(wldPos[Z] - active_lights[i].pos[Z]);
					} else {
		active_dot = JO_ABS(wldPos[X] + active_lights[i].pos[X]) +
					JO_ABS(wldPos[Y] + active_lights[i].pos[Y]) +
					JO_ABS(wldPos[Z] + active_lights[i].pos[Z]);
					}
	
				if(active_dot < nearest_dot)
				{
					light_used = &active_lights[i];
					nearest_dot = active_dot;
				}
				
				}
		}
		
	//Set the ambient light
	////////////////////////////////////////////////////
	/*
	
	The pre-matrix angle ( " master angle " ) must be passed into the system to rotate the ambient light angle.
	The pre-matrix angle is what the object's rotation is BEFORE matrix multiplication. In other words, how the _object_ is rotated.
	
	For some reason, this light angle has to be transformed incorrectly to be correct...
	(technical: it uses the transposed matrix)
	(e2: this is an "inverse transform", from world space back to the matrix' space, because it is going to be used in matrix space)
	
	*/
	ambient_light[X] = fxm(-light_used->ambient_light[X], prematrix[0])
					+ fxm(light_used->ambient_light[Y], prematrix[1])
					+ fxm(-light_used->ambient_light[Z], prematrix[2]);
					
	ambient_light[Y] = fxm(-light_used->ambient_light[X], prematrix[3])
					+ fxm(light_used->ambient_light[Y], prematrix[4])
					+ fxm(-light_used->ambient_light[Z], prematrix[5]);
					
	ambient_light[Z] = fxm(-light_used->ambient_light[X], prematrix[6])
					+ fxm(light_used->ambient_light[Y], prematrix[7])
					+ fxm(-light_used->ambient_light[Z], prematrix[8]);
	////////////////////////////////////////////////////
	*brightness_floor = (int)(light_used->min_bright);
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	LIGHT PROCESSING
	lightSrc is a point-light.
	The function needs to take the entities world position as an argument.
	Then get a vector from the world pos to the light pos.
	It becomes the light's distance.
	We then normalize that distance to get the light angle.
	This normalization process is sensitive, so we use the most accurate method we can.
	At that point we use the inverse squared law, plus a brightness multiplier, to find the light bright.
	If the light is far, the light angle is just considered zero. No light is contributed.
	As a result, dynamic point lights are generally quite dim.
	*/
		if(model_purpose == 'P')
		{
	lightDist[X] = wldPos[X] - light_used->pos[X];
	lightDist[Y] = wldPos[Y] - light_used->pos[Y];
	lightDist[Z] = wldPos[Z] - light_used->pos[Z];
		} else {
	lightDist[X] = wldPos[X] + light_used->pos[X];
	lightDist[Y] = wldPos[Y] + light_used->pos[Y];
	lightDist[Z] = wldPos[Z] + light_used->pos[Z];
		}
	int vmag = 0;
		if( JO_ABS(lightDist[X]) < (147<<16) && JO_ABS(lightDist[Y]) < (147<<16) && JO_ABS(lightDist[Z]) < (147<<16))
		{
	vmag = slSquartFX(fxdot(lightDist, lightDist));
		
	vmag = fxdiv(1<<16, vmag);
	//Normalize the light vector *properly*
	lightAngle[X] = fxm(vmag, lightDist[X]);
	lightAngle[Y] = fxm(vmag, lightDist[Y]);
	lightAngle[Z] = fxm(vmag, lightDist[Z]);
	
	//Now we have to transform the light angle by the object's orientation
	VECTOR tempAngle;

	tempAngle[X] = fxm(lightAngle[X], prematrix[0])
					+ fxm(lightAngle[Y], prematrix[1])
					+ fxm(lightAngle[Z], prematrix[2]);
					
	tempAngle[Y] = fxm(lightAngle[X], prematrix[3])
					+ fxm(lightAngle[Y], prematrix[4])
					+ fxm(lightAngle[Z], prematrix[5]);
					
	tempAngle[Z] = fxm(lightAngle[X], prematrix[6])
					+ fxm(lightAngle[Y], prematrix[7])
					+ fxm(lightAngle[Z], prematrix[8]);
			
		if(model_purpose == 'P')
		{
	lightAngle[X] = -tempAngle[X];
	lightAngle[Y] = -tempAngle[Y];
	lightAngle[Z] = -tempAngle[Z];
		} else {
	lightAngle[X] = tempAngle[X];
	lightAngle[Y] = tempAngle[Y];
	lightAngle[Z] = tempAngle[Z];
		}
	
	//Retrieve the inverse square of distance
	return fxdiv(1<<16, fxdot(lightDist, lightDist)) * (int)light_used->bright;
	/////////////////////////////////////////////////////////////////////////////////
		}
		
	return 0;
}

////////////////////////////
// Light shade determinant function
////////////////////////////
inline void determine_colorbank(unsigned short * colorBank, int * luma)
{
	//The point of the shades:
	// 0: Overbright / Noon, direct sunlight
	// 1: Lit / Overcast / Indoor light
	// 2: Shaded / Dim light / Twilight
	// 3: Full moon / Dark room
	// 4: Most lightless shade // Notice this shade cannot be used in hi-res mode (will just be shade #3)
	*colorBank = (*luma >= 73726) ? 0x0 : 1;
	*colorBank = (*luma < 49152) ? 0x2 : *colorBank;
	*colorBank = (*luma < 16384) ? 0x3 : *colorBank; 
	*colorBank = (*luma < 8196) ? 0x203 : *colorBank; //Make really dark? use MSB shadow
}


inline	void	preclipping(vertex_t ** ptv, unsigned short * flip, unsigned short * pclp)
{
	
		///////////////////////////////////////////
		// *flipping polygon such that vertex 0 is on-screen, or disable pre-clipping
		// Costs some CPU time, beware. Improves VDP1 performance, especially important for hi-res mode.
		///////////////////////////////////////////
 		if( (ptv[0]->clipFlag & 12) ){ //Vertical *flip
			//Incoming Arrangement:
			// 0 - 1		^
			//-------- Edge | Y-
			// 3 - 2		|
			// ("12" is both Y- and Y+ exit. Why: Dual-plane polygons
            ptv[4] = ptv[3]; ptv[3] = ptv[0]; ptv[0] = ptv[4];
            ptv[4] = ptv[1]; ptv[1] = ptv[2]; ptv[2] = ptv[4];
            *flip ^= 1<<5; //sprite *flip value [v *flip]
			//Outgoing Arrangement:
			// 3 - 2		^
			//-------- Edge | Y-
			// 0 - 1		|
		} 
		if( (ptv[0]->clipFlag & 3) ){//H *flip 
			//Incoming Arrangement:
			//	0 | 1
			//	3 | 2
			//	 Edge  ---> X+
			// ("3" is both X+ and X- screen exit). Why: dual-plane polygons
            ptv[4] = ptv[1];  ptv[1]=ptv[0];  ptv[0] = ptv[4];
            ptv[4] = ptv[2];  ptv[2]=ptv[3];  ptv[3] = ptv[4];
            *flip ^= 1<<4; //sprite *flip value [h *flip]
			//Outgoing Arrangement:
			// 1 | 0
			// 2 | 3
			//	Edge	---> X+
			return;
		} 
		if( !((ptv[0]->clipFlag | ptv[1]->clipFlag | ptv[2]->clipFlag | ptv[3]->clipFlag) & SCRN_CLIP_FLAGS))
		{
			*pclp = VDP1_PRECLIPPING_DISABLE; //Preclipping Disable
			return;
		}
		/*
		//Alternate Clip Handling
		// If NO CLIP FLAGS are high, disable preclipping.
		// This improves VDP1 performance, at the cost of some CPU time.
		ptv[0]->clipFlag |= ptv[1]->clipFlag | ptv[2]->clipFlag | ptv[3]->clipFlag;
		if( !(ptv[0]->clipFlag & SCRN_CLIP_FLAGS) )
		{
			*pclp = 2048; //Preclipping Disable
			return;
		}
		*/
		//In case no pre-clipping is NOT disabled, write it as such.
		*pclp = 0;
}

////////////////////////////
// Vertex clipping function helper
// This isn't much more complicated than it has to be.
////////////////////////////
inline	void	clipping(vertex_t * pnt, short useClip)
{
        //Screen Clip Flags for on-off screen decimation
		//No longer simplified....
		pnt->clipFlag = ((pnt->pnt[X]) > vert_clip_x) ? SCRN_CLIP_X : 0; 
		pnt->clipFlag |= ((pnt->pnt[X]) < vert_clip_nx) ? SCRN_CLIP_NX : pnt->clipFlag; 
		pnt->clipFlag |= ((pnt->pnt[Y]) > vert_clip_y) ? SCRN_CLIP_Y : pnt->clipFlag;
		pnt->clipFlag |= ((pnt->pnt[Y]) < vert_clip_ny) ? SCRN_CLIP_NY : pnt->clipFlag;
		pnt->clipFlag ^= (useClip == USER_CLIP_OUTSIDE) ? pnt->clipFlag : 0; //Clip out/in setting
		pnt->clipFlag |= ((pnt->pnt[Z]) <= NEAR_PLANE_DISTANCE) ? CLIP_Z : pnt->clipFlag;
}

////////////////////////////
// Setting User CLipping Coordinates
// Note the depth setting. Due to painter's algorithm, you must set the Z of your clipping coordinate *behind* what you want to clip.
// This is so the clipping coordinate will be processed before those polygons are, and thus effective.
// When clipping coordinates are set like this, you should take care to set new clipping coordinates carefully;
// Clipping coordinates shouldn't just be set to the very back; they should be set just behind everything you want to clip.
/*
However, the software does not manage the Z of the clipping region for vertices.
That is dependent on a portal system being implemented which can set vertex clipping by sector.
*/
// For standard resolutions, setting the clipping correctly is important for CPU performance, as it cuts down on commands sent.
// For hi-res mode, setting the clipping coordinates is mostly important for managing VDP1 performance.
// In that case, you can have a high-resolution interface off the side of the screen, and the game raster to a smaller area of it.
////////////////////////////
void setUserClippingAtDepth(int * topLeft, int * btmRight, int zDepthTgt)
{
	int tl[2] = {topLeft[X], topLeft[Y]};
	int br[2] = {btmRight[X], btmRight[Y]};
	//Note: User clipping coordinates do not have to be within the screen, but it's a good idea to make them be.
	//They do however need to be inside system clipping.
	msh2SetCommand(tl, tl, br, br, 8 /* User Clipping Command Set */, 0 /*All other but Z ignored*/, 0, 0, 0, 0, zDepthTgt);
	vert_clip_nx = topLeft[X] - TV_HALF_WIDTH;
	vert_clip_x = btmRight[X] - TV_HALF_WIDTH;
	vert_clip_ny = topLeft[Y] - TV_HALF_HEIGHT;
	vert_clip_y = btmRight[Y] - TV_HALF_HEIGHT;
}

/*Complex Setting, 0: clip in system, 1: clip in user, 2: clip out user*/
//To use user-clipping, be sure to set the clipping area in advance.
void ssh2DrawModel(entity_t * ent) //Primary variable sorting rendering
{
	if(ent->file_done != 1){return;}
	drawn_entity_list[drawn_entity_count] = ent;
	drawn_entity_count++;
	//Recommended, for performance, that large entities be placed in HWRAM.
    static MATRIX newMtx;
	slMultiMatrix((POINT *)ent->prematrix);
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

    GVPLY * model = ent->pol;
    if ( (transVerts[0]+model->nbPoint) >= INTERNAL_MAX_VERTS) return;
    if ( (transPolys[0]+model->nbPolygon) >= INTERNAL_MAX_POLY) return;
	
	unsigned short usrClp = SYS_CLIPPING; //The clipping setting added to command table
	if(ent->useClip == USER_CLIP_INSIDE)
	{
		//Clip inside the user clipping setting
		usrClp = 0x400;
	} else if(ent->useClip == USER_CLIP_OUTSIDE)
	{
		//Clip outside the user clipping setting
		usrClp = 0x600;
	}
	
	VECTOR lightAngle = {0, -65535, 0};
	VECTOR ambient_light = {0, -65535, 0};
	int ambient_bright = 0;
	
	int bright = 0;
	if(ent->type != 'F') // 'F' for 'flat', no dynamic lighting applied.
	{
	bright = process_light(lightAngle, ambient_light, &ambient_bright, ent->prematrix, ent->type);
	} else {
	ambient_bright = active_lights[0].min_bright;
	}

	FIXED luma;
	unsigned short colorBank;
	int inverseZ = 0;
 
    for (unsigned int i = 0; i < model->nbPoint; i++)
    {
        /**calculate z**/
        ssh2VertArea[i].pnt[Z] = trans_pt_by_component(model->pntbl[i], m2z);
		ssh2VertArea[i].pnt[Z] = (ssh2VertArea[i].pnt[Z] > NEAR_PLANE_DISTANCE) ? ssh2VertArea[i].pnt[Z] : NEAR_PLANE_DISTANCE;

         /**Starts the division**/
        SetFixDiv(scrn_dist, ssh2VertArea[i].pnt[Z]);

        /**Calculates X and Y while waiting for screenDist/z **/
        ssh2VertArea[i].pnt[Y] = trans_pt_by_component(model->pntbl[i], m1y);
        ssh2VertArea[i].pnt[X] = trans_pt_by_component(model->pntbl[i], m0x);
		
        /** Retrieves the result of the division **/
		inverseZ = *DVDNTL;

        /**Transform X and Y to screen space**/
        ssh2VertArea[i].pnt[X] = fxm(ssh2VertArea[i].pnt[X], inverseZ)>>SCR_SCALE_X;
        ssh2VertArea[i].pnt[Y] = fxm(ssh2VertArea[i].pnt[Y], inverseZ)>>SCR_SCALE_Y;
 
        //Screen Clip Flags for on-off screen decimation
		//No longer simplified....
		clipping(&ssh2VertArea[i], ent->useClip);
    }

    transVerts[0] += model->nbPoint;

	vertex_t * ptv[5] = {0, 0, 0, 0, 0};
	unsigned short flip = 0;
	unsigned short flags = 0;
	unsigned short pclp = 0;
	int zDepthTgt = 0;

    /**POLYGON PROCESSING**/ 
    for (unsigned int i = 0; i < model->nbPolygon; i++)
    {
		ptv[0] = &ssh2VertArea[model->pltbl[i].Vertices[0]];
		ptv[1] = &ssh2VertArea[model->pltbl[i].Vertices[1]];
		ptv[2] = &ssh2VertArea[model->pltbl[i].Vertices[2]];
		ptv[3] = &ssh2VertArea[model->pltbl[i].Vertices[3]];
		flags = model->attbl[i].render_data_flags;
		flip = GET_FLIP_DATA(flags);
		zDepthTgt = GET_SORT_DATA(flags);
		//Components of screen-space cross-product used for backface culling.
		//Vertice order hint:
		// 0 - 1
		// 3 - 2
		//A cross-product can tell us if it's facing the screen. If it is not, we do not want it.
		 int cross0 = (ptv[1]->pnt[X] - ptv[3]->pnt[X])
							* (ptv[0]->pnt[Y] - ptv[2]->pnt[Y]);
		 int cross1 = (ptv[1]->pnt[Y] - ptv[3]->pnt[Y])
							* (ptv[0]->pnt[X] - ptv[2]->pnt[X]);
		//Sorting target.
		//Uses logic to determine sorting target per polygon. This costs some CPU time.
		if( zDepthTgt == GV_SORT_MAX)
		{					
			//Sorting target. Uses max.
			zDepthTgt = JO_MAX(
			JO_MAX(ptv[0]->pnt[Z], ptv[2]->pnt[Z]),
			JO_MAX(ptv[1]->pnt[Z], ptv[3]->pnt[Z]));
		} else if( zDepthTgt == GV_SORT_MIN) 
		{
			//Sort Minimum
			zDepthTgt = JO_MIN(
			JO_MIN(ptv[0]->pnt[Z], ptv[2]->pnt[Z]),
			JO_MIN(ptv[1]->pnt[Z], ptv[3]->pnt[Z]));
		} else {
		//Sorting target. Uses average of top-left and bottom-right. 
			zDepthTgt = (ptv[0]->pnt[Z] + ptv[2]->pnt[Z])>>1;
		}
		 int offScrn = (ptv[0]->clipFlag & ptv[1]->clipFlag & ptv[2]->clipFlag & ptv[3]->clipFlag);
 
		if((cross0 >= cross1 && (flags & GV_FLAG_SINGLE)) || zDepthTgt < NEAR_PLANE_DISTANCE || zDepthTgt > FAR_PLANE_DISTANCE ||
		offScrn || ssh2SentPolys[0] >= MAX_SSH2_SENT_POLYS){ continue; }
		//Pre-clipping Function
		preclipping(ptv, &flip, &pclp);
		//Lighting
		luma = fxm(-(fxdot(model->pltbl[i].norm, lightAngle) + 32768), bright);
		//We set the minimum luma as zero so the dynamic light does not corrupt the global light's basis.
		luma = (bright < 0) ? ((luma > 0) ? 0 : luma) : ((luma < 0) ? 0 : luma);
		luma += fxdot(model->pltbl[i].norm, ambient_light) + ambient_bright; //In normal "vision" however, bright light would do that..
		//Use transformed normal as shade determinant
		determine_colorbank(&colorBank, &luma);
		
 		flags = (((flags & GV_FLAG_MESH)>>1) | ((flags & GV_FLAG_DARK)<<4))<<8;
		
        ssh2SetCommand(ptv[0]->pnt, ptv[1]->pnt, ptv[2]->pnt, ptv[3]->pnt,
		VDP1_BASE_CMDCTRL | (flip), (VDP1_BASE_PMODE | flags | pclp | usrClp),
		pcoTexDefs[model->attbl[i].texno].SRCA, colorBank<<6, pcoTexDefs[model->attbl[i].texno].SIZE, 0, zDepthTgt);
    } //Sort Max Endif
		transPolys[0] += model->nbPolygon;

}

//Master SH2 drawing function (needs to be sorted after by slave)
void msh2DrawModel(entity_t * ent, MATRIX msMatrix, FIXED * lightSrc)
{
	if(ent->file_done != 1){return;}
	drawn_entity_list[drawn_entity_count] = ent;
	drawn_entity_count++;
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
	
    GVPLY * model = ent->pol;
    if ( (transVerts[0]+model->nbPoint) >= INTERNAL_MAX_VERTS) return;
    if ( (transPolys[0]+model->nbPolygon) >= INTERNAL_MAX_POLY) return;
	
	unsigned short usrClp = SYS_CLIPPING; //The clipping setting added to command table
	if(ent->useClip == USER_CLIP_INSIDE)
	{
		//Clip inside the user clipping setting
		usrClp = 0x400;
	} else if(ent->useClip == USER_CLIP_OUTSIDE)
	{
		//Clip outside the user clipping setting
		usrClp = 0x600;
	}
	
	FIXED luma;
	unsigned short colorBank;
	int inverseZ = 0;

    for (unsigned int i = 0; i < model->nbPoint; i++)
    {
        /**calculate z**/
        msh2VertArea[i].pnt[Z] = trans_pt_by_component(model->pntbl[i], m2z);
		msh2VertArea[i].pnt[Z] = (msh2VertArea[i].pnt[Z] > NEAR_PLANE_DISTANCE) ? msh2VertArea[i].pnt[Z] : NEAR_PLANE_DISTANCE;
 
         /**Starts the division**/
        SetFixDiv(scrn_dist, msh2VertArea[i].pnt[Z]);

        /**Calculates X and Y while waiting for screenDist/z **/
        msh2VertArea[i].pnt[Y] = trans_pt_by_component(model->pntbl[i], m1y);
        msh2VertArea[i].pnt[X] = trans_pt_by_component(model->pntbl[i], m0x);
		
        /** Retrieves the result of the division **/
		inverseZ = *DVDNTL;

        /**Transform X and Y to screen space**/
        msh2VertArea[i].pnt[X] = fxm(msh2VertArea[i].pnt[X], inverseZ)>>SCR_SCALE_X;
        msh2VertArea[i].pnt[Y] = fxm(msh2VertArea[i].pnt[Y], inverseZ)>>SCR_SCALE_Y;
 
		clipping(&msh2VertArea[i], ent->useClip);
    }

    transVerts[0] += model->nbPoint;

	vertex_t * ptv[5] = {0, 0, 0, 0, 0};
	unsigned short flip = 0;
	unsigned short flags = 0;
	unsigned short pclp = 0;

    /**POLYGON PROCESSING**/ 
    for (unsigned int i = 0; i < model->nbPolygon; i++)
    {
		ptv[0] = &msh2VertArea[model->pltbl[i].Vertices[0]];
		ptv[1] = &msh2VertArea[model->pltbl[i].Vertices[1]];
		ptv[2] = &msh2VertArea[model->pltbl[i].Vertices[2]];
		ptv[3] = &msh2VertArea[model->pltbl[i].Vertices[3]];
		flags = model->attbl[i].render_data_flags;
		flip = GET_FLIP_DATA(flags);
		//Components of screen-space cross-product used for backface culling.
		//Vertice order hint:
		// 0 - 1
		// 3 - 2
		//A cross-product can tell us if it's facing the screen. If it is not, we do not want it.
		 int cross0 = (ptv[1]->pnt[X] - ptv[3]->pnt[X])
							* (ptv[0]->pnt[Y] - ptv[2]->pnt[Y]);
		 int cross1 = (ptv[1]->pnt[Y] - ptv[3]->pnt[Y])
							* (ptv[0]->pnt[X] - ptv[2]->pnt[X]);
		//Sorting target. Uses average of top-left and bottom-right. 
		//Adding logic to change sorting per-polygon can be done, but costs CPU time.
		 int zDepthTgt = JO_MAX(
		JO_MAX(ptv[0]->pnt[Z], ptv[2]->pnt[Z]),
		JO_MAX(ptv[1]->pnt[Z], ptv[3]->pnt[Z]));
		 int onScrn = (ptv[0]->clipFlag & ptv[1]->clipFlag & ptv[2]->clipFlag & ptv[3]->clipFlag);
 
		if((cross0 >= cross1 && (flags & GV_FLAG_SINGLE)) || zDepthTgt < NEAR_PLANE_DISTANCE || zDepthTgt > FAR_PLANE_DISTANCE ||
		onScrn || msh2SentPolys[0] >= MAX_MSH2_SENT_POLYS){ continue; }
		//Pre-clipping Function
		preclipping(ptv, &flip, &pclp);
		//Transform the polygon's normal by light source vector
		luma = fxdot(model->pltbl[i].norm, lightSrc);
		//Use transformed normal as shade determinant
		determine_colorbank(&colorBank, &luma);

 		flags = (((flags & GV_FLAG_MESH)>>1) | ((flags & GV_FLAG_DARK)<<4))<<8;

        msh2SetCommand(ptv[0]->pnt, ptv[1]->pnt, ptv[2]->pnt, ptv[3]->pnt,
		VDP1_BASE_CMDCTRL | (flip), (VDP1_BASE_PMODE | flags | pclp | usrClp),
		pcoTexDefs[model->attbl[i].texno].SRCA, colorBank<<6, pcoTexDefs[model->attbl[i].texno].SIZE, 0, zDepthTgt);
    }
		transPolys[0] += model->nbPolygon;

}

void ssh2DrawAnimation(animationControl * animCtrl, entity_t * ent, Bool transplant) //Draws animated model via SSH2
{
	if(ent->file_done != 1){return;}
	drawn_entity_list[drawn_entity_count] = ent;
	drawn_entity_count++;
	//WARNING:
	//Once an entity is drawn animated, *all* instances of that entity must be drawn animated, or else they will not reset the pntbl appropriately.
    static MATRIX newMtx;
	slMultiMatrix((POINT *)ent->prematrix);
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

    GVPLY * model = ent->pol;
    if ( (transVerts[0]+model->nbPoint) >= INTERNAL_MAX_VERTS) return;
    if ( (transPolys[0]+model->nbPolygon) >= INTERNAL_MAX_POLY) return;
	
	unsigned short usrClp = SYS_CLIPPING; //The clipping setting added to command table
	if(ent->useClip == USER_CLIP_INSIDE)
	{
		//Clip inside the user clipping setting
		usrClp = 0x400;
	} else if(ent->useClip == USER_CLIP_OUTSIDE)
	{
		//Clip outside the user clipping setting
		usrClp = 0x600;
	}
	
	VECTOR lightAngle = {0, -65535, 0};
	VECTOR ambient_light = {0, -65535, 0};
	int ambient_bright = 0;
	
	int bright = 0;
	if(ent->type != 'F') // 'F' for 'flat', no dynamic lighting applied.
	{
	bright = process_light(lightAngle, ambient_light, &ambient_bright, ent->prematrix, ent->type);
	} else {
	ambient_bright = active_lights[0].min_bright;
	}
	
	FIXED luma;
	unsigned short colorBank;
	
	//Process for static pose change:
	//1. Check if both animations are static poses [if arate of startFrm is 0 or if startFrm == endFrm]
	//2. Set currentFrm to the AnimArea startFrm<<3
	//3. Set uniforn to 0
	//4. Set the local arate to 4
	//5. Set currentKeyFrm to the AnimArea startFrm
	//6. set the nextKeyFrame to the animCtrl startFrm
	//7. Interpolate once
	//8. Return all control data as if set from the animCtrl pose
	Bool animation_change = (AnimArea[anims].startFrm != animCtrl->startFrm && AnimArea[anims].endFrm != animCtrl->endFrm) ? 1 : 0;
//
	unsigned char localArate;
	unsigned char nextKeyFrm;
	int frDelta;
	compVert * curKeyFrame;
	compVert * nextKeyFrame;
    /**Sets the animation data**/
///Variable interpolation set
localArate = animCtrl->arate[AnimArea[anims].currentKeyFrm];


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

	
 if(animation_change == 1 && transplant != 1) 
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
    volatile Sint32 * dst = model->pntbl[0]; //This pointer is incremented by the animation interpolator.
    short * src = curKeyFrame[0];
    short * nxt = nextKeyFrame[0];
	int inverseZ = 0;
 
    for (unsigned int i = 0; i < model->nbPoint; i++)
    {
		/**Uncompress the vertices and apply linear interpolation**/
		#pragma GCC push_options
		#pragma GCC diagnostic ignored "-Wsequence-point"
		*dst++=( *src + ((( *nxt++ - *src++) * frDelta)>>4))<<8;
		*dst++=( *src + ((( *nxt++ - *src++) * frDelta)>>4))<<8;
		*dst++=( *src + ((( *nxt++ - *src++) * frDelta)>>4))<<8;
		#pragma GCC pop_options
        /**calculate z**/
        ssh2VertArea[i].pnt[Z] = trans_pt_by_component(model->pntbl[i], m2z);
		ssh2VertArea[i].pnt[Z] = (ssh2VertArea[i].pnt[Z] > NEAR_PLANE_DISTANCE) ? ssh2VertArea[i].pnt[Z] : NEAR_PLANE_DISTANCE;
 
         /**Starts the division**/
        SetFixDiv(scrn_dist, ssh2VertArea[i].pnt[Z]);

        /**Calculates X and Y while waiting for screenDist/z **/
        ssh2VertArea[i].pnt[Y] = trans_pt_by_component(model->pntbl[i], m1y);
        ssh2VertArea[i].pnt[X] = trans_pt_by_component(model->pntbl[i], m0x);
		
        /** Retrieves the result of the division **/
		inverseZ = *DVDNTL;

        /**Transform X and Y to screen space**/
        ssh2VertArea[i].pnt[X] = fxm(ssh2VertArea[i].pnt[X], inverseZ)>>SCR_SCALE_X;
        ssh2VertArea[i].pnt[Y] = fxm(ssh2VertArea[i].pnt[Y], inverseZ)>>SCR_SCALE_Y;
		
		//For animated models, CPU time is at a premium.
		//Simplifying the clipping system specifically for animations might be worth.
		clipping(&ssh2VertArea[i], ent->useClip);
    }

    transVerts[0] += model->nbPoint;

    dst = (Sint32 *)&model->pltbl[0];
    volatile Uint8 *src2 = ent->animation[AnimArea[anims].currentKeyFrm]->cNorm; //A new 1-byte src
	VECTOR tNorm = {0, 0, 0};
	
	vertex_t * ptv[5] = {0, 0, 0, 0, 0};
	unsigned short flip = 0;
	unsigned short flags = 0;
	unsigned short pclp = 0;

    /**POLYGON PROCESSING**/ 
    for (unsigned int i = 0; i < model->nbPolygon; i++)
    {
		ptv[0] = &ssh2VertArea[model->pltbl[i].Vertices[0]];
		ptv[1] = &ssh2VertArea[model->pltbl[i].Vertices[1]];
		ptv[2] = &ssh2VertArea[model->pltbl[i].Vertices[2]];
		ptv[3] = &ssh2VertArea[model->pltbl[i].Vertices[3]];
		flags = model->attbl[i].render_data_flags;
		flip = GET_FLIP_DATA(flags);
		//Components of screen-space cross-product used for backface culling.
		//Vertice order hint:
		// 0 - 1
		// 3 - 2
		//A cross-product can tell us if it's facing the screen. If it is not, we do not want it.
		 int cross0 = (ptv[1]->pnt[X] - ptv[3]->pnt[X])
							* (ptv[0]->pnt[Y] - ptv[2]->pnt[Y]);
		 int cross1 = (ptv[1]->pnt[Y] - ptv[3]->pnt[Y])
							* (ptv[0]->pnt[X] - ptv[2]->pnt[X]);
		//Sorting target. Uses average of top-left and bottom-right. 
		//Adding logic to change sorting per-polygon can be done, but costs CPU time.
		 int zDepthTgt = (ptv[0]->pnt[Z] + ptv[2]->pnt[Z])>>1;

		src2 += (i != 0) ? 1 : 0; //Add to compressed normal pointer address, always, but only after the first polygon
 
		if((cross0 >= cross1 && (flags & GV_FLAG_SINGLE)) || zDepthTgt < NEAR_PLANE_DISTANCE || zDepthTgt > FAR_PLANE_DISTANCE ||
		((ptv[0]->clipFlag & ptv[2]->clipFlag) == 1) ||
		ssh2SentPolys[0] >= MAX_SSH2_SENT_POLYS){ continue; }
		//Pre-clipping Function
		preclipping(ptv, &flip, &pclp);
		//New normals in from animation normal table // These are not written back to memory
        tNorm[X]=ANORMS[*src2][X];
        tNorm[Y]=ANORMS[*src2][Y];
        tNorm[Z]=ANORMS[*src2][Z];
		//Transform the polygon's normal by light source vector
		luma = fxm(-(fxdot(tNorm, lightAngle) + 32768), bright);
		//We set the minimum luma as zero so the dynamic light does not corrupt the global light's basis.
		luma = (bright < 0) ? ((luma > 0) ? 0 : luma) : ((luma < 0) ? 0 : luma);
		luma += fxdot(tNorm, ambient_light) + ambient_bright; //In normal "vision" however, bright light would do that..
		//Use transformed normal as shade determinant
		determine_colorbank(&colorBank, &luma);
		
 		flags = (((flags & GV_FLAG_MESH)>>1) | ((flags & GV_FLAG_DARK)<<4))<<8;

        ssh2SetCommand(ptv[0]->pnt, ptv[1]->pnt, ptv[2]->pnt, ptv[3]->pnt,
		VDP1_BASE_CMDCTRL | (flip), (VDP1_BASE_PMODE | flags | pclp | usrClp),
		pcoTexDefs[model->attbl[i].texno].SRCA, colorBank<<6, pcoTexDefs[model->attbl[i].texno].SIZE, 0, zDepthTgt);
    }
		transPolys[0] += model->nbPolygon;
		
 // Check to see if the animation matches, or if reset is enabled.
 // In these cases, re-load information from the AnimCtrl.
 if(animation_change == 1 || animCtrl->reset_enable == 'Y') 
 {
	
	if(transplant != 1)
	{
	AnimArea[anims].currentFrm = animCtrl->startFrm<<3;
	} else {
		//Transplant the running frame / current frame to its place in the other animation set
	AnimArea[anims].currentFrm += (animCtrl->startFrm<<3) - (AnimArea[anims].startFrm<<3) + 2;
		//Transplant the keyframe pos so it doesn't spend a frame re-seating the animation
	AnimArea[anims].currentKeyFrm = AnimArea[anims].currentFrm>>3;
	}
	
	AnimArea[anims].startFrm = animCtrl->startFrm;
	AnimArea[anims].endFrm = animCtrl->endFrm;
	AnimArea[anims].reset_enable = 'N';
	animCtrl->reset_enable = 'N';
 }
		
		anims++; //Increment animation work area pointer

}


