//anidefs.h

#define ANIM_CONST (8)

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
	
	static int fire_lifetimaes[4] = {1024, 1024, 1024, 1024};
	shorty_fire.lifetimes = &fire_lifetimaes[0];
	
	static bg_key * fire_keylist[4];
	fire_keylist[0] = &fire;
	fire_keylist[1] = &open;
	fire_keylist[2] = &eject;
	fire_keylist[3] = &close;
	
	shorty_fire.keyframes = fire_keylist;

	

	//////////////
	// You CANNOT animate with keyframe 0. Keyframe 0 must be a duplicate of the model at rest.
	//
	//////////////
	// reload.reset_enable = 'N';
	// reload.arate[2] = 2;
	// reload.arate[3] = 2;
	// reload.startFrm = 2;
	// reload.curFrm = reload.startFrm * ANIM_CONST;
	// reload.endFrm = 3;
	
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
