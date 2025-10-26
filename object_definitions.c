

_sobject FirstRing = {
	.entity_ID = 1,
	.clone_ID = 1,
	.radius[X] = 15,
	.radius[Y] = 15,
	.radius[Z] = 15,
	.ext_dat = ITEM | 0x10,
	.light_bright = 0,
	.light_y_offset = 0,
	.effect = 1,
	.effectTimeLimit = 8192,
};

_sobject DestroyBlock = {
	.entity_ID = 2,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = OBJECT | OBJECT_DESTRUCTIBLE | 0xE,
	.light_bright = 0
};

_sobject Block = {
	.entity_ID = 2,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = OBJECT,
	.light_bright = 0
};

_sobject Build00 = {
	.entity_ID = 12,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = BUILD,
	.light_bright = 0
};


_sobject GoToLevel0 = {
	.entity_ID = 0,
	.radius[X] = 25,
	.radius[Y] = 25,
	.radius[Z] = 25,
	.ext_dat = LDATA | LEVEL_CHNG | 0x80
};

_sobject StartStand = {
	.entity_ID = 11,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = BUILD
};

_sobject Player_Start_Location = {
	.entity_ID = 0,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = LDATA | PSTART
};

_sobject PCM_EVENT_trigger = {
	.entity_ID = 0,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = LDATA | EVENT_TRIG
};

_sobject Empty = {
	.entity_ID = 0,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = GHOST
};

_sobject EmptyBuild = {
	.entity_ID = 0,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = BUILD
};

_sobject Strange = {
	.entity_ID = 15,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = BUILD
};

_sobject ToggleClosed = {
	.entity_ID = 12, //snd_wind
	.clone_ID = 14, //snd_cronch
	.radius[X] = 32,
	.radius[Y] = 128,
	.radius[Z] = 32,
	.ext_dat = LDATA | MOVER_TARGET | (MOVER_TARGET_RATE & 2) | MOVER_TARGET_CALLBACK | MOVER_TARGET_DELAYED,
	.effect = 18, //snd_khit
	.effectTimeLimit = 1<<16
};

_sobject ToggleOpen = {
	.entity_ID = 12, //snd_wind
	.clone_ID = 14, //snd_cronch
	.radius[X] = 32,
	.radius[Y] = 64,
	.radius[Z] = 32,
	.ext_dat = LDATA | MOVER_TARGET | (MOVER_TARGET_RATE & 2) | MOVER_TARGET_CALLBACK,
	.effect = 18, //snd_khit
	.effectTimeLimit = 1<<16
};

_sobject MoverClosed = {
	.entity_ID = 13, //snd_wind
	.clone_ID = 15, //snd_cronch
	.radius[X] = 32,
	.radius[Y] = 128,
	.radius[Z] = 32,
	.ext_dat = LDATA | MOVER_TARGET | (MOVER_TARGET_RATE & 2) | MOVER_TARGET_PROX | MOVER_TARGET_DELAYED,
	.effect = 17, //snd_khit
	.effectTimeLimit = 1<<16
};

_sobject MoverOpen = {
	.entity_ID = 13, //snd_wind
	.clone_ID = 15, //snd_cronch
	.radius[X] = 32,
	.radius[Y] = 64,
	.radius[Z] = 32,
	.ext_dat = LDATA | MOVER_TARGET | (MOVER_TARGET_RATE & 2) | MOVER_TARGET_RETURN,
	.effect = 17, //snd_khit
	.effectTimeLimit = 1<<16
};

_sobject TestSpawner = {
	.entity_ID = 2,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = SPAWNER | SPAWNER_T_EXAMPLE,
	.light_bright = 0
};

_sobject ButtonStand = {
	.entity_ID = 1,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = OBJECT | REMOTE_ACTIVATOR | REMOTE_ACT_USABLE | REMOTE_ACT_RESET,
	.light_bright = 0,
	.effect = 0,
	.effectTimeLimit = 0 //I forgot, conventional use applies to these...
};

_sobject * objList[OBJECT_ENTRY_CAP];

void	fill_obj_list(void)
{
	for(int i = 0; i < OBJECT_ENTRY_CAP; i++)
	{
		objList[i] = NULL;
	}
	
	objList[0] = &EmptyBuild;
	objList[1] = &DestroyBlock;
	objList[2] = &Block;
	objList[11] = &StartStand;
	objList[12] = &Build00;
	
	objList[50] = &Strange;
	
	objList[56] = &ToggleOpen;
	objList[57] = &ToggleClosed;
	
	objList[58] = &ButtonStand;

	objList[59] = &MoverOpen;

	objList[60] = &TestSpawner;

	objList[61] = &Player_Start_Location;

	objList[62] = &MoverClosed;
	objList[63] = &Empty;
}


