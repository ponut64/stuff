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
	airIdle.arate[4] = 0;
	airIdle.startFrm = 4;
	airIdle.currentFrm = airIdle.startFrm  * ANIM_CONST;
	airIdle.endFrm = 4;
	
	airLeft.reset_enable = 'N';
	airLeft.arate[9] = 0;
	airLeft.startFrm = 9;
	airLeft.currentFrm = airLeft.startFrm * ANIM_CONST;
	airLeft.endFrm = 9;
	
	airRight.reset_enable = 'N';
	airRight.arate[10] = 0;
	airRight.startFrm = 10;
	airRight.currentFrm = airRight.startFrm * ANIM_CONST;
	airRight.endFrm = 10;
	
	jump.reset_enable = 'N';
	jump.arate[11] = 0;
	jump.startFrm = 11;
	jump.currentFrm = jump.startFrm * ANIM_CONST;
	jump.endFrm = 11;
	
	hop.reset_enable = 'N';
	hop.arate[12] = 0;
	hop.startFrm = 12;
	hop.currentFrm = hop.startFrm * ANIM_CONST;
	hop.endFrm = 12;
	
//Note: Of non-uniform animations, the end frame (19 in this case) is never part of the animation.
	walk.reset_enable = 'N';
	walk.arate[13] = 1;
	walk.arate[14] = 2;
	walk.arate[15] = 2;
	walk.arate[16] = 1;
	walk.arate[17] = 2;
	walk.arate[18] = 2;
	walk.startFrm = 13;
	walk.currentFrm = walk.startFrm * ANIM_CONST;
	walk.endFrm=19;
	
	run.reset_enable = 'N';
	run.arate[19] = 2;
	run.arate[20] = 3;
	run.arate[21] = 4;
	run.arate[22] = 4;
	run.arate[23] = 2;
	run.arate[24] = 3;
	run.arate[25] = 4;
	run.arate[26] = 4;
	run.startFrm = 19;
	run.currentFrm = run.startFrm  * ANIM_CONST;
	run.endFrm=27;
	
	dbound.reset_enable = 'N';
	dbound.arate[27] = 8;
	dbound.arate[28] = 8;
	dbound.arate[29] = 4;
	dbound.arate[30] = 2;
	dbound.arate[31] = 6;
	dbound.arate[32] = 8;
	dbound.arate[33] = 8;
	dbound.arate[34] = 4;
	dbound.arate[35] = 2;
	dbound.arate[36] = 6;
	dbound.startFrm = 27;
	dbound.currentFrm = dbound.startFrm * ANIM_CONST;
	dbound.endFrm=37;
	
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
