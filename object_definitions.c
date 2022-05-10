_sobject Lamp = {
	.entity_ID = 0,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = OBJECT,
	.light_bright = 1000
};

_sobject Build = {
	.entity_ID = 1,
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

_sobject GoToLevel1 = {
	.entity_ID = 1,
	.radius[X] = 25,
	.radius[Y] = 25,
	.radius[Z] = 25,
	.ext_dat = LDATA | LEVEL_CHNG | 0x80
};

_sobject GoToLevel2 = {
	.entity_ID = 2,
	.radius[X] = 25,
	.radius[Y] = 25,
	.radius[Z] = 25,
	.ext_dat = LDATA | LEVEL_CHNG | 0x80
};

_sobject GoToLevel3 = {
	.entity_ID = 3,
	.radius[X] = 25,
	.radius[Y] = 25,
	.radius[Z] = 25,
	.ext_dat = LDATA | LEVEL_CHNG | 0x80
};

_sobject GoToLevel4 = {
	.entity_ID = 4,
	.radius[X] = 25,
	.radius[Y] = 25,
	.radius[Z] = 25,
	.ext_dat = LDATA | LEVEL_CHNG | 0x80
};

_sobject WreathTrackData = {
	.entity_ID = 1,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = LDATA | TRACK_DATA | 0x0F
};

_sobject Empty;

_sobject * objList[64];


void	fill_obj_list(void)
{
	objList[0] = &Lamp;
	objList[1] = &Build;

	objList[63] = &Empty;
}
