

_sobject BB00 = {
	.entity_ID = 0,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = GHOST,
	.light_bright = 0
};

_sobject BB01 = {
	.entity_ID = 1,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = GHOST,
	.light_bright = 0
};

_sobject BB02 = {
	.entity_ID = 2,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = GHOST,
	.light_bright = 0
};

_sobject BB03 = {
	.entity_ID = 3,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = GHOST,
	.light_bright = 0
};

_sobject BB04 = {
	.entity_ID = 4,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = GHOST,
	.light_bright = 0
};

_sobject Meme00 = {
	.entity_ID = 5,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = BUILD,
	.light_bright = 0
};

_sobject Meme01 = {
	.entity_ID = 6,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = BUILD,
	.light_bright = 0
};

_sobject Meme02 = {
	.entity_ID = 7,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = BUILD,
	.light_bright = 0
};

_sobject Ring00 = {
	.entity_ID = 8,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = GATE_R,
	.light_bright = 0
};

_sobject Post00 = {
	.entity_ID = 9,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = GATE_P,
	.light_bright = 0
};

_sobject Platf00 = {
	.entity_ID = 10,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = BUILD,
	.light_bright = 0
};

_sobject Build00 = {
	.entity_ID = 11,
	.radius[X] = 0,
	.radius[Y] = 0,
	.radius[Z] = 0,
	.ext_dat = BUILD,
	.light_bright = 0
};

_sobject Build01 = {
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

_sobject GoToLevel1 = {
	.entity_ID = 1,
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

//To make this into a usable system for game engine, I need to write another tool.
// The original source file for a level design consists of:
//	1. A list of object types, specifying the file-name to be used for this object type otherwise adhering to the type " _sobject "
//	2. A list of object declarations: Type, position, rotation.
//	Note that building objects declare sub-objects.
// At its most basic, I need the tool to write a binary file with the following:
//	1. The total number of entities, the total number of object types, and the total number of declared objects in the level
//	2. A list of entity file-names to be loaded for the level
//	3. A list of object types, listing the level-specific appropriate entityID.
//	4. An object declaration list which is an ordered list of the locations and rotations of all objects in level
// The game then needs to be prepared to:
//	1. Re-set the data pointers for meshes, re-set the entity list to zero, re-set the object list, and re-set the delcared object list.
//	2. Load the binary level data file from disc to a work area
//	3. Acquire the total total number of entities, number of object types, and total number of declared objects.
//	4. Load the entities from the file-names in order, according to the total number of entities.
//	5. Fill the object list according to the total number of object types.
//	6. Declare all objects from the level data file according to the total number of declared objects.

void	fill_obj_list(void)
{
	objList[0] = &BB00;
	objList[1] = &BB01;
	objList[2] = &BB02;
	objList[3] = &BB03;
	objList[4] = &BB04;
	objList[5] = &Meme00;
	objList[6] = &Meme01;
	objList[7] = &Meme02;
	objList[8] = &Ring00;
	objList[9] = &Post00;
	objList[10] = &Platf00;
	objList[11] = &Build00;
	objList[12] = &Build01;

	objList[63] = &Empty;
}
