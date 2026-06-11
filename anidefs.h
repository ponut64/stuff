//anidefs.h
//Variable Interpolation Animation Data. Each key frame is given an interpolation rate.
void	anim_defs(void)
{
	
	//Viewmodel (player item) background layer animations

	
	//For Shorty Shotgun
	static bg_key close;
	close.spos[X] = 246;
	close.spos[Y] = 70;
	close.size[X] = 249;
	close.size[Y] = 65;
	close.wpos[X] = (TV_WIDTH - close.size[X]);
	close.wpos[Y] = (TV_HEIGHT - close.size[Y]);
	
	static bg_key eject;
	eject.spos[X] = 241;
	eject.spos[Y] = 0;
	eject.size[X] = 172;
	eject.size[Y] = 70;
	eject.wpos[X] = (TV_WIDTH - eject.size[X]);
	eject.wpos[Y] = (TV_HEIGHT - eject.size[Y]);
	
	static bg_key fire;
	fire.spos[X] = 0;
	fire.spos[Y] = 63;
	fire.size[X] = 242;
	fire.size[Y] = 64;
	fire.wpos[X] = (TV_WIDTH - fire.size[X]);
	fire.wpos[Y] = (TV_HEIGHT - fire.size[Y]);
	
	static bg_key open;
	open.spos[X] = 0;
	open.spos[Y] = 127;
	open.size[X] = 246;
	open.size[Y] = 68;
	open.wpos[X] = (TV_WIDTH - open.size[X]);
	open.wpos[Y] = (TV_HEIGHT - open.size[Y]);
	
	static bg_key idle;
	idle.spos[X] = 0;
	idle.spos[Y] = 0;
	idle.size[X] = 241;
	idle.size[Y] = 63;
	idle.wpos[X] = (TV_WIDTH - idle.size[X]);
	idle.wpos[Y] = (TV_HEIGHT - idle.size[Y]);
	
	///////////////////////////////
	//Idle frame (Shorty Shotgun)
	///////////////////////////////
	shorty_idle.length = 1;
	shorty_idle.loop = 1;
	
	static int idle_lifetimes[1] = {-1};
	shorty_idle.lifetimes = &idle_lifetimes[0];
	
	static bg_key * idle_keylist[1];
	idle_keylist[0] = &idle;
	shorty_idle.keyframes = idle_keylist;	
	
	///////////////////////////////
	//Firing Cycle (shorty shotgun)
	///////////////////////////////
	shorty_fire.length = 4;
	shorty_fire.loop = 0;
	
	static int fire_lifetimaes[4] = {16384, 8192, 4096, 4096};
	shorty_fire.lifetimes = &fire_lifetimaes[0];
	
	static bg_key * fire_keylist[4];
	fire_keylist[0] = &fire;
	fire_keylist[1] = &open;
	fire_keylist[2] = &eject;
	fire_keylist[3] = &close;
	
	shorty_fire.keyframes = fire_keylist;
	shorty_fire.sequence = (void*)&shorty_idle;
	
	shorty_fire.sound = snd_shotf;
	//For Lever Pistol (this dopesheet has spare room for another item)
	static bg_key lp_idle_key;
	lp_idle_key.spos[X] = 0;
	lp_idle_key.spos[Y] = 0;
	lp_idle_key.size[X] = 77;
	lp_idle_key.size[Y] = 84;
	lp_idle_key.wpos[X] = (TV_WIDTH - lp_idle_key.size[X] - 16);
	lp_idle_key.wpos[Y] = (TV_HEIGHT - lp_idle_key.size[Y]);
	
	static bg_key lp_fire_key;
	lp_fire_key.spos[X] = 0;
	lp_fire_key.spos[Y] = 84;
	lp_fire_key.size[X] = 80;
	lp_fire_key.size[Y] = 84;
	lp_fire_key.wpos[X] = (TV_WIDTH - lp_fire_key.size[X] - 16);
	lp_fire_key.wpos[Y] = (TV_HEIGHT - lp_fire_key.size[Y]);

	static bg_key lp_open1_key;
	lp_open1_key.spos[X] = 78;
	lp_open1_key.spos[Y] = 0;
	lp_open1_key.size[X] = 74;
	lp_open1_key.size[Y] = 89;
	lp_open1_key.wpos[X] = (TV_WIDTH - lp_open1_key.size[X] - 16);
	lp_open1_key.wpos[Y] = (TV_HEIGHT - lp_open1_key.size[Y]);
	
	static bg_key lp_open2_key;
	lp_open2_key.spos[X] = 81;
	lp_open2_key.spos[Y] = 89;
	lp_open2_key.size[X] = 75;
	lp_open2_key.size[Y] = 91;
	lp_open2_key.wpos[X] = (TV_WIDTH - lp_open2_key.size[X] - 16);
	lp_open2_key.wpos[Y] = (TV_HEIGHT - lp_open2_key.size[Y]);
	
	static bg_key lp_close_key;
	lp_close_key.spos[X] = 0;
	lp_close_key.spos[Y] = 168;
	lp_close_key.size[X] = 77;
	lp_close_key.size[Y] = 84;
	lp_close_key.wpos[X] = (TV_WIDTH - lp_close_key.size[X] - 16);
	lp_close_key.wpos[Y] = (TV_HEIGHT - lp_close_key.size[Y]);
	
	///////////////////////////////
	//Idle frame (Lever Pistol)
	///////////////////////////////
	leverpistol_idle.length = 1;
	leverpistol_idle.loop = 1;
	
	//(a staple idle lifetime unit can be used here)
	leverpistol_idle.lifetimes = &idle_lifetimes[0];
	
	static bg_key * leverpistol_idle_keylist[1];
	leverpistol_idle_keylist[0] = &lp_idle_key;
	leverpistol_idle.keyframes = leverpistol_idle_keylist;	

	///////////////////////////////
	//Firing Cycle (lever pistol)
	///////////////////////////////
	leverpistol_fire.length = 4;
	leverpistol_fire.loop = 0;
	
	static int leverpistol_fire_lifetimes[4] = {16384, 4096, 8192, 4096};
	leverpistol_fire.lifetimes = &leverpistol_fire_lifetimes[0];
	
	static bg_key * leverpistol_fire_keylist[4];
	leverpistol_fire_keylist[0] = &lp_fire_key;
	leverpistol_fire_keylist[1] = &lp_open1_key;
	leverpistol_fire_keylist[2] = &lp_open2_key;
	leverpistol_fire_keylist[3] = &lp_close_key;
	
	leverpistol_fire.keyframes = leverpistol_fire_keylist;
	leverpistol_fire.sequence = (void*)&leverpistol_idle;

	leverpistol_fire.sound = snd_lpistf;
	//////////////
	// You CANNOT animate with keyframe 0. Keyframe 0 must be a duplicate of the model at rest.
	//////////////
	animationControl * anim;
	int count = 0;
	//okay, now I need to calculate out the length of each animation (in seconds)
	// -- but shouldn't I count frames instead?
	
	//Keyframes for VOIDWALKER / ELEMENTAL entity type
	vw_idle_pose.reset_enable = 'N';													
	vw_idle_pose.arate[0] = 0;								
	vw_idle_pose.startFrm = 0;
	vw_idle_pose.curFrm = vw_idle_pose.startFrm<<ANIM_SHIFT;
	vw_idle_pose.endFrm = 0;
	
	vw_dead_pose.reset_enable = 'N';													
	vw_dead_pose.arate[19] = 0;								
	vw_dead_pose.startFrm = 19;
	vw_dead_pose.curFrm = vw_dead_pose.startFrm<<ANIM_SHIFT;
	vw_dead_pose.endFrm = 19;
	
	vw_point_pose.reset_enable = 'N';													
	vw_point_pose.arate[8] = 0;								
	vw_point_pose.startFrm = 8;
	vw_point_pose.curFrm = vw_point_pose.startFrm<<ANIM_SHIFT;
	vw_point_pose.endFrm = 8;
	
	vw_look_anim.reset_enable = 'Y';													
	vw_look_anim.arate[3] = 1;		
	vw_look_anim.arate[4] = 1;	
	vw_look_anim.arate[5] = 1;	
	vw_look_anim.arate[6] = 1;	
	vw_look_anim.arate[7] = 1;	
	vw_look_anim.startFrm = 3;
	vw_look_anim.curFrm = vw_look_anim.startFrm<<ANIM_SHIFT;
	vw_look_anim.endFrm = 7;
	
	anim = &vw_look_anim;
	for(int i = anim->startFrm; i <= anim->endFrm; i++)
	{
		count += ANIM_TIME(anim->arate[i]);
	}
	count += ANIM_TIME(1);
	count += ANIM_TIME(1);
	anim->time = count;
	count = 0;
	
	vw_point_anim.reset_enable = 'Y';													
	vw_point_anim.arate[7] = 1;	
	vw_point_anim.arate[8] = 1;		
	vw_point_anim.startFrm = 7;
	vw_point_anim.curFrm = vw_point_anim.startFrm<<ANIM_SHIFT;
	vw_point_anim.endFrm = 8;
	
	anim = &vw_point_anim;
	for(int i = anim->startFrm; i <= anim->endFrm; i++)
	{
		count += ANIM_TIME(anim->arate[i]);
	}
	anim->time = count;
	count = 0;
	
	vw_move_anim.reset_enable = 'N';													
	vw_move_anim.arate[19] = 1;		
	vw_move_anim.arate[20] = 1;	
	vw_move_anim.arate[21] = 1;	
	vw_move_anim.arate[22] = 1;	
//	vw_move_anim.arate[23] = 1;	(this keyframe is included, but it is a duplicate of the first keyframe -- a neccessary artifact for interpolation)
	vw_move_anim.startFrm = 19;
	vw_move_anim.curFrm = vw_move_anim.startFrm<<ANIM_SHIFT;
	vw_move_anim.endFrm = 22;
	
	anim = &vw_move_anim;
	for(int i = anim->startFrm; i <= anim->endFrm; i++)
	{
		count += ANIM_TIME(anim->arate[i]);
	}
	anim->time = count;
	count = 0;
	
	vw_idle_anim.reset_enable = 'N';													
	vw_idle_anim.arate[0] = 1;		
	vw_idle_anim.arate[1] = 1;	
	vw_idle_anim.arate[2] = 1;	
//	vw_idle_anim.arate[3] = 1;	//(this keyframe is included, but it is a duplicate of the first keyframe -- a neccessary artifact for interpolation)
	vw_idle_anim.startFrm = 0;
	vw_idle_anim.curFrm = vw_idle_anim.startFrm<<ANIM_SHIFT;
	vw_idle_anim.endFrm = 2;
	
	anim = &vw_idle_anim;
	for(int i = anim->startFrm; i <= anim->endFrm; i++)
	{
		count += ANIM_TIME(anim->arate[i]);
	}
	anim->time = count;
	count = 0;
	
	vw_aggro_anim.reset_enable = 'N';													
	vw_aggro_anim.arate[16] = 1;		
	vw_aggro_anim.arate[17] = 1;	
	//vw_aggro_anim.arate[18] = 1;	//(this keyframe is included, but it is a duplicate of the first keyframe -- a neccessary artifact for interpolation)
	vw_aggro_anim.startFrm = 16;
	vw_aggro_anim.curFrm = vw_aggro_anim.startFrm<<ANIM_SHIFT;
	vw_aggro_anim.endFrm = 17;
	
	anim = &vw_aggro_anim;
	for(int i = anim->startFrm; i <= anim->endFrm; i++)
	{
		count += ANIM_TIME(anim->arate[i]);
	}
	anim->time = count;
	count = 0;
	
	vw_melee_anim.reset_enable = 'Y';	
	vw_melee_anim.arate[13] = 1;		
	vw_melee_anim.arate[14] = 1;		
	vw_melee_anim.arate[15] = 1;	
	vw_melee_anim.arate[16] = 1;	
	vw_melee_anim.startFrm = 13;
	vw_melee_anim.curFrm = vw_melee_anim.startFrm<<ANIM_SHIFT;
	vw_melee_anim.endFrm = 16;
	
	anim = &vw_melee_anim;
	for(int i = anim->startFrm; i <= anim->endFrm; i++)
	{
		count += ANIM_TIME(anim->arate[i]);
	}
	anim->time = count;
	count = 0;
	
	vw_dead_anim.reset_enable = 'Y';	
	vw_dead_anim.arate[23] = 1;	
	vw_dead_anim.arate[24] = 1;				
	vw_dead_anim.startFrm = 23;
	vw_dead_anim.curFrm = vw_dead_anim.startFrm<<ANIM_SHIFT;
	vw_dead_anim.endFrm = 24;
	
	anim = &vw_dead_anim;
	for(int i = anim->startFrm; i <= anim->endFrm; i++)
	{
		count += ANIM_TIME(anim->arate[i]);
	}
	anim->time = count;
	count = 0;
	
	// idle.reset_enable = 'N';													
	// idle.arate[0] = 0;								
	// idle.startFrm = 0;
	// idle.curFrm = idle.startFrm<<ANIM_SHIFT;
	// idle.endFrm = 0;
	
	// idleB.reset_enable = 'N';													
	// idleB.arate[0] = 1;	
	// idleB.arate[1] = 1;	
	// idleB.arate[2] = 1;		
	// idleB.startFrm = 0;
	// idleB.curFrm = idle.startFrm<<ANIM_SHIFT;
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
	// walk.curFrm = walk.startFrm<<ANIM_SHIFT;
	// walk.endFrm=19;

//Sprite(Texture)Animation
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
