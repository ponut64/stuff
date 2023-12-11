#include <sl_def.h>
#include "def.h"
#include "mloader.h"
#include "mymath.h"
#include "bounder.h"

#include "render.h"
#include "anorm.h"

animationControl AnimArea[MAX_SIMULTANEOUS_ANIMATED_ENTITIES];

spriteAnimation spriteAnims[MAX_SIMULTANEOUS_SPRITE_ANIMATIONS];

//When an animated texture is assigned to an entity, it is always assigned by index.
//Rather than being indexed from another entity, animated textures are instead indexed into this list.
//The polygon's indexed texture number is entered into this list,
// and what is returned at that point in the list is assigned to that polygon.
// e.g. "attbl[i].texno = animated_texture_list[attbl[i].texno];" - BEFORE being offset by the entity's start texture.
// This happens if "attbl[i].render_data_flags & GV_FLAG_ANIM".
int animated_texture_list[MAX_SIMULTANEOUS_SPRITE_ANIMATIONS];

void	clean_sprite_animations(void)
{
	//
	for(int i = 0; i < MAX_SIMULTANEOUS_SPRITE_ANIMATIONS; i++)
	{
		if(spriteAnims[i].lifetime < 0)
		{
			spriteAnims[i].lifetime = 0;
			spriteAnims[i].modEnt = NULL;
		} else if(spriteAnims[i].lifetime > 0)
		{
			spriteAnims[i].lifetime -= delta_time;	
		}
	}
	
}

void	operate_texture_animations(void)
{

	GVPLY * model;
	spriteAnimation * anim;

	for(int i = 0; i < MAX_SIMULTANEOUS_SPRITE_ANIMATIONS; i++)
	{
		anim = &spriteAnims[i];
		if(anim->lifetime > 0 && anim->modEnt != NULL)
		{
			model = anim->modEnt->pol;
			
			anim->curFrm += framerate;
			////////////////////////////////////////////////
			// This was written after the polygon animation.
			// Note that this is much simpler way of handling keyframed animation...
			////////////////////////////////////////////////
			if(anim->curFrm > anim->arates[anim->curKeyFrm])
			{
				anim->curKeyFrm++;
				anim->curFrm = 0;
			}
			
			if(anim->curKeyFrm >= anim->endFrm)
			{
				anim->curKeyFrm = anim->startFrm;
			}

			//So to do this, right now, we need to check if the texno is between sprite sheet start/end.
			for(unsigned int u = 0; u < model->nbPolygon; u++)
			{
				if(model->attbl[u].texno >= anim->sprite_sheet_start && model->attbl[u].texno < anim->sprite_sheet_end)
				{
					model->attbl[u].texno = anim->sprite_sheet_start + anim->curKeyFrm;
					model->lumatbl[u] = anim->lumas[anim->curKeyFrm];
				}
			}
		}
	}	

}

void	start_texture_animation(spriteAnimation * anim, entity_t * ent)
{
	if(ent->file_done != 1){return;}
	
	//First loop: Check if this texture is already being animated on this entity.
	//If it is, stop.
	for(int i = 0; i < MAX_SIMULTANEOUS_SPRITE_ANIMATIONS; i++)
	{

		if(spriteAnims[i].lifetime != 0 &&
		spriteAnims[i].sprite_sheet_start == anim->sprite_sheet_start &&
		spriteAnims[i].modEnt == ent) return;
	}	
	
	
	//Otherwise, try to find a free animation entry to work with.
	for(int i = 0; i < MAX_SIMULTANEOUS_SPRITE_ANIMATIONS; i++)
	{
		if(!spriteAnims[i].lifetime)
		{
			spriteAnims[i].modEnt = ent;
			spriteAnims[i].lifetime = anim->lifetime;
			spriteAnims[i].sprite_sheet_start = anim->sprite_sheet_start;
			spriteAnims[i].sprite_sheet_end = anim->sprite_sheet_end;
			spriteAnims[i].curFrm = 0;
			spriteAnims[i].curKeyFrm = 0;
			spriteAnims[i].startFrm = anim->startFrm;
			spriteAnims[i].endFrm = anim->endFrm;
			spriteAnims[i].arates = anim->arates;
			spriteAnims[i].lumas = anim->lumas;
			break;
		}
	}	
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
	int cue = 0;
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
	//2. Set curFrm to the AnimArea startFrm<<3
	//3. Set uniforn to 0
	//4. Set the local arate to 4
	//5. Set curKeyFrm to the AnimArea startFrm
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
	localArate = animCtrl->arate[AnimArea[anims].curKeyFrm];


	AnimArea[anims].curFrm += (localArate * framerate)>>1;

   AnimArea[anims].curKeyFrm = (AnimArea[anims].curFrm>>3);
    if (AnimArea[anims].curKeyFrm >= AnimArea[anims].endFrm)
	{
        AnimArea[anims].curFrm -= (AnimArea[anims].endFrm - AnimArea[anims].startFrm)<<3;
        AnimArea[anims].curKeyFrm = AnimArea[anims].curFrm>>3;
    } else if(AnimArea[anims].curKeyFrm < AnimArea[anims].startFrm)
	{
		AnimArea[anims].curKeyFrm = AnimArea[anims].startFrm;
		AnimArea[anims].curFrm += (AnimArea[anims].endFrm-AnimArea[anims].startFrm)<<3;
	}
    nextKeyFrm = AnimArea[anims].curKeyFrm+1;

    if (nextKeyFrm >= AnimArea[anims].endFrm){
        nextKeyFrm = AnimArea[anims].startFrm;
	} else if (nextKeyFrm <= AnimArea[anims].startFrm){
        nextKeyFrm = AnimArea[anims].startFrm+1;
	}

	
 if(animation_change == 1 && transplant != 1) 
 {
	//For single-frame interpolation between poses
	curKeyFrame = (compVert*)ent->animation[AnimArea[anims].curKeyFrm]->cVert;
	nextKeyFrame = (compVert*)ent->animation[animCtrl->startFrm]->cVert;
	frDelta = 8;
 } else {
	//For interpolation inside keyframed animation
	curKeyFrame = (compVert*)ent->animation[AnimArea[anims].curKeyFrm]->cVert;
	nextKeyFrame = (compVert*)ent->animation[nextKeyFrm]->cVert;
	///Don't touch this! **absolute** frame delta 
	frDelta = (AnimArea[anims].curFrm)-(AnimArea[anims].curKeyFrm<<3);
 }


	//Animation Data
    volatile Sint32 * dst = model->pntbl[0]; //This pointer is incremented by the animation interpolator.
    short * src = curKeyFrame[0];
    short * nxt = nextKeyFrame[0];
	int inverseZ = 0;
	////////////////////////////////////////////////////////////
	// Pre-loop
	////////////////////////////////////////////////////////////
	// ** 1 **
	// Interpolate/decompress the FIRST vertex
	// Hypothetically, we should do this, but it seems to be fine to use the last frame's vertex 0.
	// Alternatively, we know that any jump costs us a pipeline dump, but *only* if the code is not in cache.
	// Thus, a small loop which performs the decompression/interpolation before transformation occurs may be faster.
	// It might be if it fits in cache. Then again, you still lose 3 cycles to the jump, on pure instruction cost.
	// ** 2 **
	// Calculate Z for the FIRST vertex
	ssh2VertArea[0].pnt[Z] = trans_pt_by_component(model->pntbl[0], m2z);
	ssh2VertArea[0].pnt[Z] = (ssh2VertArea[0].pnt[Z] > NEAR_PLANE_DISTANCE) ? ssh2VertArea[0].pnt[Z] : NEAR_PLANE_DISTANCE;
	// ** 3 **
	// Set the division unit to work on the FIRST vertex Z
	SetFixDiv(scrn_dist, ssh2VertArea[0].pnt[Z]);

    for (unsigned int i = 0; i < model->nbPoint; i++)
    {

		// ** 1 **
        /**Calculates X and Y while waiting for screenDist/z for CURRENT vertex **/
        ssh2VertArea[i].pnt[Y] = trans_pt_by_component(model->pntbl[i], m1y);
        ssh2VertArea[i].pnt[X] = trans_pt_by_component(model->pntbl[i], m0x);
		// ** 2 **
		/**Uncompress the NEXT vertex and apply linear interpolation**/
		#pragma GCC push_options
		#pragma GCC diagnostic ignored "-Wsequence-point"
		*dst++=( *src + ((( *nxt++ - *src++) * frDelta)>>4))<<8;
		*dst++=( *src + ((( *nxt++ - *src++) * frDelta)>>4))<<8;
		*dst++=( *src + ((( *nxt++ - *src++) * frDelta)>>4))<<8;
		#pragma GCC pop_options
		// ** 3 **
        /** Retrieves the result of the division  for CURRENT vertex**/
		inverseZ = *DVDNTL;
		// ** 4 **
        /**calculate z for the NEXT vertex**/
        ssh2VertArea[i+1].pnt[Z] = trans_pt_by_component(model->pntbl[i+1], m2z);
		ssh2VertArea[i+1].pnt[Z] = (ssh2VertArea[i+1].pnt[Z] > NEAR_PLANE_DISTANCE) ? ssh2VertArea[i+1].pnt[Z] : NEAR_PLANE_DISTANCE;
		// ** 5 **
         /**Starts the division for the NEXT vertex**/
        SetFixDiv(scrn_dist, ssh2VertArea[i+1].pnt[Z]);
		// ** 6 **
        /**Transform X and Y to screen space for CURRENT vertex**/
        ssh2VertArea[i].pnt[X] = fxm(ssh2VertArea[i].pnt[X], inverseZ)>>SCR_SCALE_X;
        ssh2VertArea[i].pnt[Y] = fxm(ssh2VertArea[i].pnt[Y], inverseZ)>>SCR_SCALE_Y;
		
		//For animated models, CPU time is at a premium.
		//Simplifying the clipping system specifically for animations might be worth.
		clipping(&ssh2VertArea[i], ent->useClip);
    }

    transVerts[0] += model->nbPoint;

    dst = (Sint32 *)&model->pltbl[0];
    volatile Uint8 *src2 = ent->animation[AnimArea[anims].curKeyFrm]->cNorm; //A new 1-byte src
	VECTOR tNorm = {0, 0, 0};
	
	vertex_t * ptv[5] = {0, 0, 0, 0, 0};
	unsigned short flip = 0;
	unsigned short flags = 0;
	unsigned short pclp = 0;

    /**POLYGON PROCESSING**/ 
    for (unsigned int i = 0; i < model->nbPolygon; i++)
    {
		ptv[0] = &ssh2VertArea[model->pltbl[i].vertices[0]];
		ptv[1] = &ssh2VertArea[model->pltbl[i].vertices[1]];
		ptv[2] = &ssh2VertArea[model->pltbl[i].vertices[2]];
		ptv[3] = &ssh2VertArea[model->pltbl[i].vertices[3]];
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
		//Shift the color bank code to the appropriate bits
		colorBank<<=6;
		//Added later: In case of a polyline (or really, any untextured command),
		// the color for the draw command is defined by the draw command's "texno" or texture number data.
		// this texture number data however is inserted in the wrong parts of the draw command to be the color.
		// So here, we insert it into the correct place in the command table to be the drawn color.
		unsigned short usedCMDCTRL = (flags & GV_FLAG_POLYLINE) ? VDP1_POLYLINE_CMDCTRL : VDP1_BASE_CMDCTRL;
		colorBank += (usedCMDCTRL == VDP1_BASE_CMDCTRL) ? 0 : model->attbl[i].texno;
		
 		flags = (((flags & GV_FLAG_MESH)>>1) | ((flags & GV_FLAG_DARK)<<4))<<8;

		depth_cueing(&zDepthTgt, &cue);

        ssh2SetCommand(ptv[0]->pnt, ptv[1]->pnt, ptv[2]->pnt, ptv[3]->pnt,
		usedCMDCTRL | (flip), (VDP1_BASE_PMODE | flags | pclp | usrClp),
		pcoTexDefs[model->attbl[i].texno].SRCA, colorBank | cue, pcoTexDefs[model->attbl[i].texno].SIZE, 0, zDepthTgt);
    }
		transPolys[0] += model->nbPolygon;
		
 // Check to see if the animation matches, or if reset is enabled.
 // In these cases, re-load information from the AnimCtrl.
 if(animation_change == 1 || animCtrl->reset_enable == 'Y') 
 {
	
	if(transplant != 1)
	{
	AnimArea[anims].curFrm = animCtrl->startFrm<<3;
	} else {
		//Transplant the running frame / current frame to its place in the other animation set
	AnimArea[anims].curFrm += (animCtrl->startFrm<<3) - (AnimArea[anims].startFrm<<3) + 2;
		//Transplant the keyframe pos so it doesn't spend a frame re-seating the animation
	AnimArea[anims].curKeyFrm = AnimArea[anims].curFrm>>3;
	}
	
	AnimArea[anims].startFrm = animCtrl->startFrm;
	AnimArea[anims].endFrm = animCtrl->endFrm;
	AnimArea[anims].reset_enable = 'N';
	animCtrl->reset_enable = 'N';
 }
		
		anims++; //Increment animation work area pointer

}

//Use by either CPU to apply the animation interpolation to a mesh.
void	meshAnimProcessing(animationControl * animCtrl, entity_t * ent, Bool transplant)
{
	if(ent->file_done != 1){return;}

    GVPLY * model = ent->pol;
	
	//Process for static pose change:
	//1. Check if both animations are static poses [if arate of startFrm is 0 or if startFrm == endFrm]
	//2. Set curFrm to the AnimArea startFrm<<3
	//3. Set uniforn to 0
	//4. Set the local arate to 4
	//5. Set curKeyFrm to the AnimArea startFrm
	//6. set the nextKeyFrame to the animCtrl startFrm
	//7. Interpolate once
	//8. Return all control data as if set from the animCtrl pose
	short animation_change = (AnimArea[anims].startFrm != animCtrl->startFrm && AnimArea[anims].endFrm != animCtrl->endFrm) ? 1 : 0;
//
	unsigned char localArate;
	unsigned char nextKeyFrm;
	int frDelta;
	compVert * curKeyFrame;
	compVert * nextKeyFrame;
    /**Sets the animation data**/
	///Variable interpolation set
	localArate = animCtrl->arate[AnimArea[anims].curKeyFrm];


	////
	//
	// The interpolator has 16 steps at current + ((next - current)>>4)
	// I don't understand why the key-frames are shifted by 3, for 8 steps... but they have to be.
	//
	////
	AnimArea[anims].curFrm += (localArate * framerate)>>1;
	AnimArea[anims].curKeyFrm = (AnimArea[anims].curFrm>>3);
	
    if (AnimArea[anims].curKeyFrm >= AnimArea[anims].endFrm)
	{
        AnimArea[anims].curFrm -= (AnimArea[anims].endFrm - AnimArea[anims].startFrm)<<3;
        AnimArea[anims].curKeyFrm = AnimArea[anims].curFrm>>3;
	} else if(AnimArea[anims].curKeyFrm < AnimArea[anims].startFrm)
	{
		AnimArea[anims].curKeyFrm = AnimArea[anims].startFrm;
		AnimArea[anims].curFrm += (AnimArea[anims].endFrm-AnimArea[anims].startFrm)<<3;
	}
    nextKeyFrm = AnimArea[anims].curKeyFrm+1;

    if (nextKeyFrm >= AnimArea[anims].endFrm)
	{
        nextKeyFrm = AnimArea[anims].startFrm;
	} else if (nextKeyFrm <= AnimArea[anims].startFrm)
	{
        nextKeyFrm = AnimArea[anims].startFrm+1;
	}

	
 if(animation_change == 1 && transplant != 1) 
 {
	//For single-frame interpolation between poses
	curKeyFrame = (compVert*)ent->animation[AnimArea[anims].curKeyFrm]->cVert;
	nextKeyFrame = (compVert*)ent->animation[animCtrl->startFrm]->cVert;
	frDelta = 8;
 } else {
	//For interpolation inside keyframed animation
	curKeyFrame = (compVert*)ent->animation[AnimArea[anims].curKeyFrm]->cVert;
	nextKeyFrame = (compVert*)ent->animation[nextKeyFrm]->cVert;
	///Don't touch this! **absolute** frame delta 
	frDelta = (AnimArea[anims].curFrm)-(AnimArea[anims].curKeyFrm<<3);
 }


	//Animation Data
    volatile Sint32 * dst = model->pntbl[0];
    short * src = curKeyFrame[0];
    short * nxt = nextKeyFrame[0];
	/////
	//
	//	Primary frame delta shift is 4, or 16 delta steps possible.
	//	1 is therefore 1/16th interpolation, 2 is 2/16th, 3 is 3/16th, etc.
	//	This interpolation is from the current keyframe;
	//	At 16/16 steps, the delta will be 1, making the current the next keyframe.
	//
	/////
	for(unsigned int i = 0; i < model->nbPoint; i++)
	{
		*dst++=( *src + (( ((*nxt++) - (*src++)) * frDelta)>>4))<<8;
		*dst++=( *src + (( ((*nxt++) - (*src++)) * frDelta)>>4))<<8;
		*dst++=( *src + (( ((*nxt++) - (*src++)) * frDelta)>>4))<<8;
	}

    dst = (Sint32 *)&model->pltbl[0];
    volatile Uint8 *src2 = ent->animation[AnimArea[anims].curKeyFrm]->cNorm; //A new 1-byte src

	for(unsigned int i = 0; i < model->nbPolygon; i++)
	{
		//New normals in from animation normal table // These are not written back to memory
        model->nmtbl[i][X]=ANORMS[*src2][X];
        model->nmtbl[i][Y]=ANORMS[*src2][Y];
        model->nmtbl[i][Z]=ANORMS[*src2][Z];
		src2++;
	}
		
 // Check to see if the animation matches, or if reset is enabled.
 // In these cases, re-load information from the AnimCtrl.
 if(animation_change == 1 || animCtrl->reset_enable == 'Y') 
 {
	
	if(transplant != 1)
	{
	AnimArea[anims].curFrm = animCtrl->startFrm<<3;
	} else {
		//Transplant the running frame / current frame to its place in the other animation set
	AnimArea[anims].curFrm += (animCtrl->startFrm<<3) - (AnimArea[anims].startFrm<<3) + 2;
		//Transplant the keyframe pos so it doesn't spend a frame re-seating the animation
	AnimArea[anims].curKeyFrm = AnimArea[anims].curFrm>>3;
	}
	
	AnimArea[anims].startFrm = animCtrl->startFrm;
	AnimArea[anims].endFrm = animCtrl->endFrm;
	AnimArea[anims].reset_enable = 'N';
	animCtrl->reset_enable = 'N';
 }
		
		anims++; //Increment animation work area pointer
	
}

