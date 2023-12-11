//anidefs.h

#define ANIM_CONST (8)

//Variable Interpolation Animation Data. Each key frame is given an interpolation rate.
void	anim_defs(void)
{
	
	//Player model animations
	
	idle.reset_enable = 'N';													
	idle.arate[0] = 0;								
	idle.startFrm = 0;
	idle.curFrm = idle.startFrm * ANIM_CONST;
	idle.endFrm = 0;
	
	idleB.reset_enable = 'N';													
	idleB.arate[0] = 1;	
	idleB.arate[1] = 1;	
	idleB.arate[2] = 1;		
	idleB.startFrm = 0;
	idleB.curFrm = idle.startFrm * ANIM_CONST;
	idleB.endFrm = 3;
	
	stop.reset_enable = 'N';
	stop.arate[3] = 0;
	stop.startFrm = 3;
	stop.curFrm = stop.startFrm * ANIM_CONST;
	stop.endFrm = 3;
	
	fall.reset_enable = 'N';
	fall.arate[8] = 0;
	fall.startFrm = 8;
	fall.curFrm = fall.startFrm * ANIM_CONST;
	fall.endFrm = 8;
	
	slideIdle.reset_enable = 'N';
	slideIdle.arate[5] = 0;
	slideIdle.startFrm = 5;
	slideIdle.curFrm = slideIdle.startFrm * ANIM_CONST;
	slideIdle.endFrm = 5;
	
	slideLln.reset_enable = 'N';
	slideLln.arate[6] = 0;
	slideLln.startFrm = 6;
	slideLln.curFrm = slideLln.startFrm * ANIM_CONST;
	slideLln.endFrm = 6;
		
	slideRln.reset_enable = 'N';
	slideRln.arate[7] = 0;
	slideRln.startFrm = 7;
	slideRln.curFrm = slideRln.startFrm * ANIM_CONST;
	slideRln.endFrm = 7;
	
	airIdle.reset_enable = 'N';
	airIdle.arate[8] = 0;
	airIdle.startFrm = 8;
	airIdle.curFrm = airIdle.startFrm  * ANIM_CONST;
	airIdle.endFrm = 8;
	
	airLeft.reset_enable = 'N';
	airLeft.arate[10] = 0;
	airLeft.startFrm = 10;
	airLeft.curFrm = airLeft.startFrm * ANIM_CONST;
	airLeft.endFrm = 10;
	
	airRight.reset_enable = 'N';
	airRight.arate[11] = 0;
	airRight.startFrm = 11;
	airRight.curFrm = airRight.startFrm * ANIM_CONST;
	airRight.endFrm = 11;
	
	jump.reset_enable = 'N';
	jump.arate[12] = 0;
	jump.startFrm = 12;
	jump.curFrm = jump.startFrm * ANIM_CONST;
	jump.endFrm = 12;
	
	hop.reset_enable = 'N';
	hop.arate[13] = 0;
	hop.startFrm = 13;
	hop.curFrm = hop.startFrm * ANIM_CONST;
	hop.endFrm = 13;
	
//Note: Of non-uniform animations, the end frame (19 in this case) is never part of the animation.
	walk.reset_enable = 'N';
	walk.arate[14] = 1;
	walk.arate[15] = 2;
	walk.arate[16] = 2;
	walk.arate[17] = 1;
	walk.arate[18] = 2;
	walk.arate[19] = 2;
	walk.startFrm = 14;
	walk.curFrm = walk.startFrm * ANIM_CONST;
	walk.endFrm=20;
	
	run.reset_enable = 'N';
	run.arate[20] = 2;
	run.arate[21] = 3;
	run.arate[22] = 4;
	run.arate[23] = 4;
	run.arate[24] = 2;
	run.arate[25] = 3;
	run.arate[26] = 4;
	run.arate[27] = 4;
	run.startFrm = 20;
	run.curFrm = run.startFrm  * ANIM_CONST;
	run.endFrm=28;
	
	dbound.reset_enable = 'N';
	dbound.arate[28] = 8;
	dbound.arate[29] = 8;
	dbound.arate[30] = 4;
	dbound.arate[31] = 2;
	dbound.arate[32] = 6;
	dbound.arate[33] = 8;
	dbound.arate[34] = 8;
	dbound.arate[35] = 4;
	dbound.arate[36] = 2;
	dbound.arate[37] = 6;
	dbound.startFrm = 28;
	dbound.curFrm = dbound.startFrm * ANIM_CONST;
	dbound.endFrm=38;
	
	climbIdle.reset_enable = 'N';
	climbIdle.arate[38] = 0;
	climbIdle.startFrm = 38;
	climbIdle.curFrm = climbIdle.startFrm * ANIM_CONST;
	climbIdle.endFrm = 38;
	
	climbing.reset_enable = 'N';
	climbing.arate[39] = 2;
	climbing.arate[40] = 2;
	climbing.arate[41] = 2;
	climbing.arate[42] = 2;
	climbing.startFrm = 39;
	climbing.curFrm = climbing.startFrm * ANIM_CONST;
	climbing.endFrm = 43;
	
	//Wing animation
	
	flap.reset_enable = 'N';
	flap.arate[1] = 3;
	flap.arate[2] = 3;
	flap.arate[3] = 3;
	flap.arate[4] = 0;
	flap.startFrm = 1;
	flap.curFrm = flap.startFrm * ANIM_CONST;
	flap.endFrm = 5;
	
	
	//Sprite (Texture) animations
	static Uint8 qmark_arate[4];
	static Uint8 qmark_lumas[4];
	qmark.lifetime = 3000<<16;
	qmark.arates = &qmark_arate[0];
	qmark.lumas = &qmark_lumas[0];
	qmark.arates[0] = 120;
	qmark.arates[1] = 15;
	qmark.arates[2] = 7;
	qmark.arates[3] = 7;
	
	qmark.lumas[0] = 255;
	qmark.lumas[1] = 0;
	qmark.lumas[2] = 0;
	qmark.lumas[3] = 0;
	qmark.sprite_sheet_start = animated_texture_list[0];
	qmark.sprite_sheet_end = qmark.sprite_sheet_start + 4;
	qmark.curFrm = 0;
	qmark.curKeyFrm = 0;
	qmark.startFrm = 0;
	qmark.endFrm = 4;
	
	static Uint8 arrow_arates[5];
	static Uint8 arrow_lumas[5];
	arrow.lifetime = 3000<<16;
	arrow.arates = &arrow_arates[0];
	arrow.lumas = &arrow_lumas[0];
	arrow.arates[0] = 6;
	arrow.arates[1] = 6;
	arrow.arates[2] = 6;
	arrow.arates[3] = 6;
	arrow.arates[4] = 6;
	
	arrow.lumas[0] = 255;
	arrow.lumas[1] = 255;
	arrow.lumas[2] = 255;
	arrow.lumas[3] = 255;
	arrow.lumas[4] = 255;
	arrow.sprite_sheet_start = animated_texture_list[1];
	arrow.sprite_sheet_end = arrow.sprite_sheet_start + 5;
	arrow.curFrm = 0;
	arrow.curKeyFrm = 0;
	arrow.startFrm = 0;
	arrow.endFrm = 5;
	
	static Uint8 check_arates[5];
	static Uint8 check_lumas[5];
	check.lifetime = 3000<<16;
	check.arates = &check_arates[0];
	check.lumas = &check_lumas[0];
	check.arates[0] = 120;
	check.arates[1] = 6;
	check.arates[2] = 12;
	check.arates[3] = 12;
	check.arates[4] = 6;
	
	check.lumas[0] = 255;
	check.lumas[1] = 255;
	check.lumas[2] = 0;
	check.lumas[3] = 0;
	check.lumas[4] = 255;
	check.sprite_sheet_start = animated_texture_list[2];
	check.sprite_sheet_end = check.sprite_sheet_start + 5;
	check.curFrm = 0;
	check.curKeyFrm = 0;
	check.startFrm = 0;
	check.endFrm = 5;
	
	static Uint8 goal_arate[4];
	static Uint8 goal_lumas[4];
	goal.lifetime = 3000<<16;
	goal.arates = &goal_arate[0];
	goal.lumas = &goal_lumas[0];
	goal.arates[0] = 120;
	goal.arates[1] = 6;
	goal.arates[2] = 6;
	goal.arates[3] = 6;
	
	goal.lumas[0] = 255;
	goal.lumas[1] = 64;
	goal.lumas[2] = 0;
	goal.lumas[3] = 64;
	goal.sprite_sheet_start = animated_texture_list[3];
	goal.sprite_sheet_end = goal.sprite_sheet_start + 4;
	goal.curFrm = 0;
	goal.curKeyFrm = 0;
	goal.startFrm = 0;
	goal.endFrm = 4;
	
	static Uint8 Leye_arates[4];
	static Uint8 Leye_lumas[4];
	LeyeAnim.lifetime = 3000<<16;
	LeyeAnim.arates = &Leye_arates[0];
	LeyeAnim.lumas = &Leye_lumas[0];
	LeyeAnim.arates[0] = 180;
	LeyeAnim.arates[1] = 1;
	LeyeAnim.arates[2] = 1;
	LeyeAnim.arates[3] = 1;

	LeyeAnim.lumas[0] = 0;
	LeyeAnim.lumas[1] = 0;
	LeyeAnim.lumas[2] = 0;
	LeyeAnim.lumas[3] = 0;
	LeyeAnim.sprite_sheet_start = animated_texture_list[4];
	LeyeAnim.sprite_sheet_end = LeyeAnim.sprite_sheet_start + 4;
	LeyeAnim.curFrm = 0;
	LeyeAnim.curKeyFrm = 0;
	LeyeAnim.startFrm = 0;
	LeyeAnim.endFrm = 4;
	
	static Uint8 Reye_arates[4];
	static Uint8 Reye_lumas[4];
	ReyeAnim.lifetime = 3000<<16;
	ReyeAnim.arates = &Reye_arates[0];
	ReyeAnim.lumas = &Reye_lumas[0];
	ReyeAnim.arates[0] = 180;
	ReyeAnim.arates[1] = 1;
	ReyeAnim.arates[2] = 1;
	ReyeAnim.arates[3] = 1;

	ReyeAnim.lumas[0] = 0;
	ReyeAnim.lumas[1] = 0;
	ReyeAnim.lumas[2] = 0;
	ReyeAnim.lumas[3] = 0;
	ReyeAnim.sprite_sheet_start = animated_texture_list[5];
	ReyeAnim.sprite_sheet_end = ReyeAnim.sprite_sheet_start + 4;
	ReyeAnim.curFrm = 0;
	ReyeAnim.curKeyFrm = 0;
	ReyeAnim.startFrm = 0;
	ReyeAnim.endFrm = 4;
	

}
