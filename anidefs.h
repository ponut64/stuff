//anidefs.h

#define ANIM_CONST (8)

//Variable Interpolation Animation Data. Each key frame is given an interpolation rate.
void	anim_defs(void)
{
	
	//Player model animations
	
//Note: Of non-uniform animations, the end frame (35 in this case) is never part of the animation.
	walk.reset_enable = 'N';
	walk.arate[1] = 1;
	walk.arate[2] = 2;
	walk.arate[3] = 2;
	walk.arate[4] = 1;
	walk.arate[5] = 2;
	walk.arate[6] = 2;
	walk.startFrm = 1;
	walk.currentFrm = walk.startFrm * ANIM_CONST;
	walk.endFrm=7;
	
	run.reset_enable = 'N';
	run.arate[7] = 2;
	run.arate[8] = 3;
	run.arate[9] = 4;
	run.arate[10] = 4;
	run.arate[11] = 2;
	run.arate[12] = 3;
	run.arate[13] = 4;
	run.arate[14] = 4;
	run.startFrm = 7;
	run.currentFrm = run.startFrm  * ANIM_CONST;
	run.endFrm=15;
	
	runshoot.reset_enable = 'N';
	runshoot.arate[15] = 2;
	runshoot.arate[16] = 3;
	runshoot.arate[17] = 4;
	runshoot.arate[18] = 4;
	runshoot.arate[19] = 2;
	runshoot.arate[20] = 3;
	runshoot.arate[21] = 4;
	runshoot.arate[22] = 4;
	runshoot.startFrm = 15;
	runshoot.currentFrm = runshoot.startFrm  * ANIM_CONST;
	runshoot.endFrm=23;
	
	runmelee.reset_enable = 'N';
	runmelee.arate[23] = 2;
	runmelee.arate[24] = 3;
	runmelee.arate[25] = 4;
	runmelee.arate[26] = 4;
	runmelee.arate[27] = 2;
	runmelee.arate[28] = 3;
	runmelee.arate[29] = 4;
	runmelee.arate[30] = 4;
	runmelee.startFrm = 23;
	runmelee.currentFrm = runmelee.startFrm  * ANIM_CONST;
	runmelee.endFrm=31;
	
	dbound.reset_enable = 'N';
	dbound.arate[31] = 8;
	dbound.arate[32] = 8;
	dbound.arate[33] = 4;
	dbound.arate[34] = 2;
	dbound.arate[35] = 6;
	dbound.arate[36] = 8;
	dbound.arate[37] = 8;
	dbound.arate[38] = 4;
	dbound.arate[39] = 2;
	dbound.arate[40] = 6;
	dbound.startFrm = 31;
	dbound.currentFrm = dbound.startFrm * ANIM_CONST;
	dbound.endFrm=41;
	
	melee.reset_enable = 'N';
	melee.arate[41] = 3;
	melee.arate[42] = 1;
	melee.arate[43] = 3;
	melee.arate[44] = 1;
	melee.arate[45] = 3;
	melee.arate[46] = 1;
	melee.startFrm = 41;
	melee.currentFrm = melee.startFrm * ANIM_CONST;
	melee.endFrm=47;
	
	shoot.reset_enable = 'N';
	shoot.arate[47] = 2;
	shoot.arate[48] = 1;
	shoot.startFrm = 47;
	shoot.currentFrm = shoot.startFrm * ANIM_CONST;
	shoot.endFrm = 49;
	
	idle.reset_enable = 'N';							
	//idle.arate[49] = 1;							
	idle.arate[50] = 0;								
	idle.startFrm = 50;
	idle.currentFrm = idle.startFrm * ANIM_CONST;
	idle.endFrm = 50;
	
	stop.reset_enable = 'N';
	stop.arate[51] = 0;
	stop.startFrm = 51;
	stop.currentFrm = stop.startFrm * ANIM_CONST;
	stop.endFrm = 51;
	
	slideIdle.reset_enable = 'N';
	slideIdle.arate[52] = 0;
	slideIdle.startFrm = 52;
	slideIdle.currentFrm = slideIdle.startFrm * ANIM_CONST;
	slideIdle.endFrm = 52;
	
	slideLln.reset_enable = 'N';
	slideLln.arate[53] = 0;
	slideLln.startFrm = 53;
	slideLln.currentFrm = slideLln.startFrm * ANIM_CONST;
	slideLln.endFrm = 53;
		
	slideRln.reset_enable = 'N';
	slideRln.arate[54] = 0;
	slideRln.startFrm = 54;
	slideRln.currentFrm = slideRln.startFrm * ANIM_CONST;
	slideRln.endFrm = 54;
	
	airIdle.reset_enable = 'N';
	airIdle.arate[55] = 0;
	airIdle.startFrm = 55;
	airIdle.currentFrm = airIdle.startFrm  * ANIM_CONST;
	airIdle.endFrm = 55;
	
	airLeft.reset_enable = 'N';
	airLeft.arate[56] = 0;
	airLeft.startFrm = 56;
	airLeft.currentFrm = airLeft.startFrm * ANIM_CONST;
	airLeft.endFrm = 56;
	
	airMelee.reset_enable = 'N';
	airMelee.arate[56] = 2;									
	airMelee.arate[57] = 2;									
	airMelee.arate[58] = 1;									
	airMelee.startFrm = 56;									
	airMelee.currentFrm = airMelee.startFrm * ANIM_CONST;	
	airMelee.endFrm = 59;
	
	airRight.reset_enable = 'N';
	airRight.arate[59] = 0;
	airRight.startFrm = 59;
	airRight.currentFrm = airRight.startFrm * ANIM_CONST;
	airRight.endFrm = 59;
	
	airShoot.reset_enable = 'N';
	airShoot.arate[60] = 0;
	airShoot.startFrm = 60;
	airShoot.currentFrm = airShoot.startFrm * ANIM_CONST;
	airShoot.endFrm = 60;
	
	jump.reset_enable = 'N';
	jump.arate[61] = 0;
	jump.startFrm = 61;
	jump.currentFrm = jump.startFrm * ANIM_CONST;
	jump.endFrm = 61;
	
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
