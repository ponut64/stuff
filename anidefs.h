//anidefs.h

#define ANIM_CONST (8)

//Variable Interpolation Animation Data. Each key frame is given an interpolation rate.
void	anim_defs(void)
{
	
	//Player model animations
	
	idle.reset_enable = 'N';													
	idle.arate[0] = 0;								
	idle.startFrm = 0;
	idle.currentFrm = idle.startFrm * ANIM_CONST;
	idle.endFrm = 0;
	
	idleB.reset_enable = 'N';													
	idleB.arate[0] = 1;	
	idleB.arate[1] = 1;	
	idleB.arate[2] = 1;		
	idleB.startFrm = 0;
	idleB.currentFrm = idle.startFrm * ANIM_CONST;
	idleB.endFrm = 3;
	
	stop.reset_enable = 'N';
	stop.arate[3] = 0;
	stop.startFrm = 3;
	stop.currentFrm = stop.startFrm * ANIM_CONST;
	stop.endFrm = 3;
	
	fall.reset_enable = 'N';
	fall.arate[8] = 0;
	fall.startFrm = 8;
	fall.currentFrm = fall.startFrm * ANIM_CONST;
	fall.endFrm = 8;
	
	slideIdle.reset_enable = 'N';
	slideIdle.arate[5] = 0;
	slideIdle.startFrm = 5;
	slideIdle.currentFrm = slideIdle.startFrm * ANIM_CONST;
	slideIdle.endFrm = 5;
	
	slideLln.reset_enable = 'N';
	slideLln.arate[6] = 0;
	slideLln.startFrm = 6;
	slideLln.currentFrm = slideLln.startFrm * ANIM_CONST;
	slideLln.endFrm = 6;
		
	slideRln.reset_enable = 'N';
	slideRln.arate[7] = 0;
	slideRln.startFrm = 7;
	slideRln.currentFrm = slideRln.startFrm * ANIM_CONST;
	slideRln.endFrm = 7;
	
	airIdle.reset_enable = 'N';
	airIdle.arate[8] = 0;
	airIdle.startFrm = 8;
	airIdle.currentFrm = airIdle.startFrm  * ANIM_CONST;
	airIdle.endFrm = 8;
	
	airLeft.reset_enable = 'N';
	airLeft.arate[10] = 0;
	airLeft.startFrm = 10;
	airLeft.currentFrm = airLeft.startFrm * ANIM_CONST;
	airLeft.endFrm = 10;
	
	airRight.reset_enable = 'N';
	airRight.arate[11] = 0;
	airRight.startFrm = 11;
	airRight.currentFrm = airRight.startFrm * ANIM_CONST;
	airRight.endFrm = 11;
	
	jump.reset_enable = 'N';
	jump.arate[12] = 0;
	jump.startFrm = 12;
	jump.currentFrm = jump.startFrm * ANIM_CONST;
	jump.endFrm = 12;
	
	hop.reset_enable = 'N';
	hop.arate[13] = 0;
	hop.startFrm = 13;
	hop.currentFrm = hop.startFrm * ANIM_CONST;
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
	walk.currentFrm = walk.startFrm * ANIM_CONST;
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
	run.currentFrm = run.startFrm  * ANIM_CONST;
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
	dbound.currentFrm = dbound.startFrm * ANIM_CONST;
	dbound.endFrm=38;
	
	climbIdle.reset_enable = 'N';
	climbIdle.arate[38] = 0;
	climbIdle.startFrm = 38;
	climbIdle.currentFrm = climbIdle.startFrm * ANIM_CONST;
	climbIdle.endFrm = 38;
	
	climbing.reset_enable = 'N';
	climbing.arate[39] = 2;
	climbing.arate[40] = 2;
	climbing.arate[41] = 2;
	climbing.arate[42] = 2;
	climbing.startFrm = 39;
	climbing.currentFrm = climbing.startFrm * ANIM_CONST;
	climbing.endFrm = 43;
	
	//Wing animation
	
	flap.reset_enable = 'N';
	flap.arate[1] = 3;
	flap.arate[2] = 3;
	flap.arate[3] = 3;
	flap.arate[4] = 0;
	flap.startFrm = 1;
	flap.currentFrm = flap.startFrm * ANIM_CONST;
	flap.endFrm = 5;
	
	

}
