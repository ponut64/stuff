//anidefs.h

#define ANIM_CONST (8)

//Variable Interpolation Animation Data. Each key frame is given an interpolation rate.
void	anim_defs(void)
{
	
//Note: Of non-uniform animations, the end frame (35 in this case) is never part of the animation.
	forward.uniform = false;
	forward.arate[30] = 1;
	forward.arate[31] = 4;
	forward.arate[32] = 3;
	forward.arate[33] = 2;
	forward.arate[34] = 1;
	forward.arate[35] = 4;
	forward.arate[36] = 3;
	forward.arate[37] = 2;
	forward.startFrm = 30;
	forward.currentFrm = forward.startFrm * ANIM_CONST;
	forward.endFrm=38;
	
	run.uniform = false;
	run.arate[17] = 2;
	run.arate[18] = 3;
	run.arate[19] = 4;
	run.arate[20] = 4;
	run.arate[21] = 2;
	run.arate[22] = 3;
	run.arate[23] = 4;
	run.arate[24] = 4;
	run.startFrm = 17;
	run.currentFrm = run.startFrm  * ANIM_CONST;
	run.endFrm=25;
	
	dbound.uniform = false;
	dbound.arate[4] = 8;
	dbound.arate[5] = 8;
	dbound.arate[6] = 4;
	dbound.arate[7] = 2;
	dbound.arate[8] = 6;
	dbound.arate[9] = 8;
	dbound.arate[10] = 8;
	dbound.arate[11] = 4;
	dbound.arate[12] = 2;
	dbound.arate[13] = 6;
	dbound.startFrm = 4;
	dbound.currentFrm = dbound.startFrm * ANIM_CONST;
	dbound.endFrm=14;
	
	slideIdle.uniform = false;
	slideIdle.arate[25] = 0;
	slideIdle.startFrm = 25;
	slideIdle.currentFrm = slideIdle.startFrm * ANIM_CONST;
	slideIdle.endFrm = 25;
	
	slideFwd.uniform = false;
	slideFwd.arate[26] = 0;
	slideFwd.startFrm = 26;
	slideFwd.currentFrm = slideFwd.startFrm * ANIM_CONST;
	slideFwd.endFrm = 26;
	
	slideRvs.uniform = false;
	slideRvs.arate[27] = 0;
	slideRvs.startFrm = 27;
	slideRvs.currentFrm = slideRvs.startFrm * ANIM_CONST;
	slideRvs.endFrm = 27;
	
	slideRln.uniform = false;
	slideRln.arate[28] = 0;
	slideRln.startFrm = 28;
	slideRln.currentFrm = slideRln.startFrm * ANIM_CONST;
	slideRln.endFrm = 28;
	
	slideLln.uniform = false;
	slideLln.arate[29] = 0;
	slideLln.startFrm = 29;
	slideLln.currentFrm = slideLln.startFrm * ANIM_CONST;
	slideLln.endFrm = 29;
	
	jump.uniform = false;
	jump.arate[15] = 0;
	jump.startFrm = 15;
	jump.currentFrm = jump.startFrm * ANIM_CONST;
	jump.endFrm = 15;
	
	jump2.uniform = false;
	jump2.arate[16] = 0;
	jump2.startFrm = 16;
	jump2.currentFrm = jump2.startFrm * ANIM_CONST;
	jump2.endFrm = 16;
	
	airIdle.uniform = false;
	airIdle.arate[0] = 0;
	airIdle.startFrm = 0;
	airIdle.currentFrm = airIdle.startFrm  * ANIM_CONST;
	airIdle.endFrm = 0;
	
	airLeft.uniform = false;
	airLeft.arate[1] = 0;
	airLeft.startFrm = 1;
	airLeft.currentFrm = airLeft.startFrm * ANIM_CONST;
	airLeft.endFrm = 1;
	
	airRight.uniform = false;
	airRight.arate[2] = 0;
	airRight.startFrm = 2;
	airRight.currentFrm = airRight.startFrm * ANIM_CONST;
	airRight.endFrm = 2;
	
	airBack.uniform = false;
	airBack.arate[3] = 0;
	airBack.startFrm = 3;
	airBack.currentFrm = airBack.startFrm * ANIM_CONST;
	airBack.endFrm = 3;
	
	fall.uniform = false;
	fall.arate[14] = 0;
	fall.startFrm = 14;
	fall.currentFrm = fall.startFrm * ANIM_CONST;
	fall.endFrm = 14;
	
	idle.uniform = false;
	idle.arate[39] = 0;
	idle.startFrm = 39;
	idle.currentFrm = idle.startFrm * ANIM_CONST;
	idle.endFrm = 39;
	
	stop.uniform = false;
	stop.arate[38] = 0;
	stop.startFrm = 38;
	stop.currentFrm = stop.startFrm * ANIM_CONST;
	stop.endFrm = 38;
}
