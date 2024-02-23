//anidefs.h

#define ANIM_CONST (8)

//Variable Interpolation Animation Data. Each key frame is given an interpolation rate.
void	anim_defs(void)
{
	//////////////
	// You CANNOT animate with keyframe 0. Keyframe 0 must be a duplicate of the model at rest.
	//
	//////////////
	reload.reset_enable = 'N';
	reload.arate[2] = 2;
	reload.arate[3] = 2;
	reload.startFrm = 2;
	reload.curFrm = reload.startFrm * ANIM_CONST;
	reload.endFrm = 3;
	
	// idle.reset_enable = 'N';													
	// idle.arate[0] = 0;								
	// idle.startFrm = 0;
	// idle.curFrm = idle.startFrm * ANIM_CONST;
	// idle.endFrm = 0;
	
	// idleB.reset_enable = 'N';													
	// idleB.arate[0] = 1;	
	// idleB.arate[1] = 1;	
	// idleB.arate[2] = 1;		
	// idleB.startFrm = 0;
	// idleB.curFrm = idle.startFrm * ANIM_CONST;
	// idleB.endFrm = 3;

//Note: Of non-uniform animations, the end frame (19 in this case) is never part of the animation.
	// walk.reset_enable = 'N';
	// walk.arate[14] = 1;
	// walk.arate[15] = 2;
	// walk.arate[16] = 2;
	// walk.arate[17] = 1;
	// walk.arate[18] = 2;
	// walk.arate[19] = 2;
	// walk.startFrm = 14;
	// walk.curFrm = walk.startFrm * ANIM_CONST;
	// walk.endFrm=19;

	
	// static Uint8 arrow3_lumas[3];
	// static Uint8 arrow3_arates[3];
	// arrow3.lifetime = 3000<<16;
	// arrow3.arates = &arrow3_arates[0];
	// arrow3.lumas = &arrow3_lumas[0];
	// arrow3.arates[0] = 6;
	// arrow3.arates[1] = 6;
	// arrow3.arates[2] = 6;

	// arrow3.lumas[0] = 255;
	// arrow3.lumas[1] = 255;
	// arrow3.lumas[2] = 255;

	// arrow3.sprite_sheet_start = animated_texture_list[7];
	// arrow3.sprite_sheet_end = arrow3.sprite_sheet_start + 3;
	// arrow3.curFrm = 0;
	// arrow3.curKeyFrm = 0;
	// arrow3.startFrm = 0;
	// arrow3.endFrm = 3;

}
