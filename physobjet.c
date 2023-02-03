#include "jo/jo.h"
#include "def.h"
#include "pcmsys.h"
#include "pcmstm.h"
#include "mymath.h"
#include "render.h"
#include "collision.h"
#include "ldata.h"
#include "hmap.h"
#include "object_col.h"

#include "physobjet.h"
#include "object_definitions.c"


_declaredObject * dWorldObjects; //In LWRAM - see lwram.c
_buildingObject * BuildingPayload;

unsigned short objNEW = 0; //objNEW is the total number of declared objects
unsigned short objDRAW[MAX_WOBJS]; //objDRAW is a list of the delcared objects that will be drawn
unsigned short activeObjects[MAX_WOBJS]; //activeObjects is a list of the declared objects that have some code running for them
// Setting the link starts as -1 is what also sets that the last object in the list will link to -1.
short link_starts[8] = {-1, -1, -1, -1,
						-1, -1, -1, -1};
int trackTimers[16];
int activeTrack = -1;
int objUP = 0;
int total_building_payload = 0;

void	declare_object_at_cell(short pixX, short height, short pixY, int type, ANGLE xrot, ANGLE yrot, ANGLE zrot, short more_data)
{
		if(objNEW < MAX_WOBJS)
		{
	dWorldObjects[objNEW].pos[X] = -(pixX * CELL_SIZE_INT)<<16;
	dWorldObjects[objNEW].pos[Z] = -(pixY * CELL_SIZE_INT)<<16;
	dWorldObjects[objNEW].pos[Y] = height<<16; //Vertical offset from ground
	dWorldObjects[objNEW].pix[X] = -(pixX);
	dWorldObjects[objNEW].pix[Y] = -(pixY);
	dWorldObjects[objNEW].type = *objList[type];
	dWorldObjects[objNEW].rot[X] = (xrot * 182); // deg * 182 = angle
	dWorldObjects[objNEW].rot[Y] = (yrot * 182);
	dWorldObjects[objNEW].rot[Z] = (zrot * 182);
		if((dWorldObjects[objNEW].type.ext_dat & OTYPE) == BUILD)
		{
			//Specific for building-type objects, place its entity ID in the ext_dat.
			dWorldObjects[objNEW].type.ext_dat |= ((dWorldObjects[objNEW].type.entity_ID)<<4);
			if((xrot | yrot | zrot) != 0)
			{
			//For build-type objects, if any rotation is applied, we can't use the same polygon data anymore.
			//This is because the collision system discards planes for collision early based on their normal.
			//This will not work if the polygon is rotated without the normal also being rotated with it.
			//So, to facilitate rotating a BUILD-type object, we must create new PDATA with the rotation built in to it.
			//The following function jumps to such a thing.
			generate_rotated_entity_for_object(objNEW);
			}
		}
		////////////////////////////////////////////////////
		// If no radius was defined for the object, use the radius from the entity.
		// Must check if the entity is loaded, or else out of bounds access may occur.
		////////////////////////////////////////////////////
		if((dWorldObjects[objNEW].type.ext_dat & OTYPE) != LDATA && dWorldObjects[objNEW].type.radius[X] == 0 &&
			dWorldObjects[objNEW].type.radius[Y] == 0 && dWorldObjects[objNEW].type.radius[Z] == 0 &&
			entities[dWorldObjects[objNEW].type.entity_ID].file_done)
		{
			dWorldObjects[objNEW].type.radius[X] = entities[dWorldObjects[objNEW].type.entity_ID].radius[X];
			dWorldObjects[objNEW].type.radius[Y] = entities[dWorldObjects[objNEW].type.entity_ID].radius[Y];
			dWorldObjects[objNEW].type.radius[Z] = entities[dWorldObjects[objNEW].type.entity_ID].radius[Z];
		}
	dWorldObjects[objNEW].link = link_starts[(dWorldObjects[objNEW].type.ext_dat & 0x7000)>>12]; //Set object's link to the current link of this type
	link_starts[(dWorldObjects[objNEW].type.ext_dat & 0x7000)>>12] = objNEW; //Set the current link of this type to this entry
	dWorldObjects[objNEW].more_data |= more_data;
	objNEW++;
		}
}

void	declare_building_object(_declaredObject * root_object, _buildingObject * building_item)
{
	//If the root object does not possess the entity ID of the item's root entity, do not add it.
		if(objNEW < MAX_WOBJS && root_object->type.entity_ID == building_item->root_entity)
		{
	int root_object_y = (entities[root_object->type.entity_ID].radius[Y]<<16);
	dWorldObjects[objNEW].pos[X] = (root_object->pos[X] + ((int)building_item->pos[X]<<16));
	dWorldObjects[objNEW].pos[Y] = (root_object->pos[Y] + ((int)building_item->pos[Y]<<16)) - root_object_y;
	dWorldObjects[objNEW].pos[Z] = (root_object->pos[Z] + ((int)building_item->pos[Z]<<16));
	
	dWorldObjects[objNEW].pix[X] = root_object->pix[X];
	dWorldObjects[objNEW].pix[Y] = root_object->pix[Y];
	dWorldObjects[objNEW].type = *objList[building_item->object_type];
	dWorldObjects[objNEW].rot[X] = 0;
	dWorldObjects[objNEW].rot[Y] = 0;
	dWorldObjects[objNEW].rot[Z] = 0;
	dWorldObjects[objNEW].more_data = 0;
	dWorldObjects[objNEW].link = link_starts[(dWorldObjects[objNEW].type.ext_dat & 0x7000)>>12]; //Set object's link to the current link of this type
	link_starts[(dWorldObjects[objNEW].type.ext_dat & 0x7000)>>12] = objNEW; //Set the current link of this type to this entry
	objNEW++;
		}
}

void	declarations(void)
{

/* level00 ?*/
//Will replace.

// declare_object_at_cell((300 / 40) + 1, -(343), -(1060 / 40), 12 /*build01*/, 0, 0, 0, 0);
// declare_object_at_cell(-(300 / 40) + 1, -300, (-1100 / 40), 61 /*start location*/, 0, 0, 0, 0);

// declare_object_at_cell( (420 / 40) + 1, -277,  -(1180/40), 9 /*post00*/, 0, 0, 0, 0);
// declare_object_at_cell( (180 / 40) + 1, -277,  -(940/40), 9 /*post00*/, 0, 0, 0, 0);

//declare_object_at_cell((220 / 40) + 1, -280, (-1060 / 40), 15 /*ADX sound trigger*/, 40, 40, 40, 7 | (7<<8) /* sound num & vol */);

/* level01? */
// Will replace.

// declare_object_at_cell(-(632 / 40) + 1, -170, -(632 / 40), 11 /*build00*/, 0, 0, 0, 0);
// declare_object_at_cell(-(897 / 40) + 1, -375,  (856 / 40), 12 /*build01*/, 0, 0, 0, 0);
														 
// declare_object_at_cell(-(366 / 40) + 1, -145, -(531 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell((612 / 40) + 1, -150,  -(653 / 40), 10 /*platf00*/, 0, 0, 0, 0);

// declare_object_at_cell((61 / 40) + 1, -275,  -(714/40), 8 /*ring00*/, 0, 0, 0, 0);
// declare_object_at_cell((918 / 40) + 1, -341,  (224/40), 8 /*ring00*/, 90, 0, 0, 0);
// declare_object_at_cell(-(856 / 40) + 1, -502, (652/40), 8 /*ring00*/, 0, 0, 0, 0);

// declare_object_at_cell( (42 / 40) + 1, -182,  (81/40), 9 /*post00*/, 0, 0, 0, 0);
// declare_object_at_cell( (42 / 40) + 1, -182, (244/40), 9 /*post00*/, 0, 180, 0, 0);
// declare_object_at_cell( (42 / 40) + 1, -178, (488/40), 9 /*post00*/, 0, 0, 0, 0);
// declare_object_at_cell( (42 / 40) + 1, -178, (652/40), 9 /*post00*/, 0, 180, 0, 0);
// declare_object_at_cell( (531 / 40) + 1, -210,(407/40), 9 /*post00*/, 0, 90, 0, 0);
// declare_object_at_cell( (694 / 40) + 1, -265,(407/40), 9 /*post00*/, 0, -90, 0, 0);
// declare_object_at_cell(-(611 / 40) + 1, -210,(407/40), 9 /*post00*/, 0, -90, 0, 0);
// declare_object_at_cell(-(815 / 40) + 1, -297,(407/40), 9 /*post00*/, 0, 90, 0, 0);

// declare_object_at_cell(-2 + 1, -115, -14, 0 /*bb00*/,0,0,0, 0);
 // declare_object_at_cell(5 + 1, -150,  -5, 1 /*bb01*/,0,0,0, 0);
// declare_object_at_cell(17 + 1, -380,  -2, 2 /*bb02*/,0,0,0, 0);
// declare_object_at_cell(-9 + 1, -255,  -2, 3 /*bb03*/,0,0,0, 0);
 // declare_object_at_cell(0 + 1, -352,  22, 4 /*bb04*/,0,0,0, 0);

// declare_object_at_cell(-5 + 1, -210,  -8, 5 /*meme00*/,0,0,0, 0);
// declare_object_at_cell(20 + 1, -205, -18, 6 /*meme01*/,0,0,0, 0);
// declare_object_at_cell(22 + 1, -430,  22, 7 /*meme02*/,0,0,0, 0);

// declare_object_at_cell(-(632 / 40) + 1, -219, (-632 / 40), 61 /*start location*/, 0, 0, 0, 0);

/* level2? */
// Likely will keep, with modification.

// declare_object_at_cell((1680 / 40) + 1, -30, -(1680 / 40), 61 /*start location*/, 0, 0, 0, 0);

// declare_object_at_cell((1080 / 40) + 1, -338, -(1440 / 40), 11 /*bridge1*/, 0, 0, 0, 0);

// declare_object_at_cell(-(560 / 40) + 1, -177, -(320 / 40), 12 /*greece01*/, 0, 0, 0, 0);

// declare_object_at_cell(-(800 / 40) + 1, -177, -(320 / 40), 13 /*greece02*/, 0, 180, 0, 0);
// declare_object_at_cell(-(320 / 40) + 1, -177, -(160 / 40), 13 /*greece02*/, 0, 90, 0, 0);

// declare_object_at_cell(-(1560 / 40) + 1, -201, -(2880 / 40), 13 /*greece02*/, 0, 45, 0, 0);
// declare_object_at_cell(-(1280 / 40) + 1, -201, -(3160 / 40), 13 /*greece02*/, 0, 45, 0, 0);
// declare_object_at_cell(-(1200 / 40) + 1, -201, -(3240 / 40), 13 /*greece02*/, 0, 45, 0, 0);
// declare_object_at_cell(-(920 / 40) + 1, -201, -(3520 / 40), 13 /*greece02*/, 0, 45, 0, 0);

// declare_object_at_cell(-(320 / 40) + 1, -177, -(320 / 40), 14 /*greece03*/, 0, 0, 0, 0);

// declare_object_at_cell(-(1440 / 40) + 1, -200, -(3040 / 40), 15 /*greece04*/, 0, 0, 0, 0);
// declare_object_at_cell(-(1080 / 40) + 1, -200, -(3400 / 40), 15 /*greece04*/, 0, 0, 0, 0);

// declare_object_at_cell((1040 / 40) + 1, -146, -(240 / 40), 16 /*overhang*/, 0, 135, 0, 0);
// declare_object_at_cell((80 / 40) + 1, -209, (3160 / 40), 16 /*overhang*/, 0, 0, 0, 0);
// declare_object_at_cell(-(560 / 40) + 1, -238, (3320 / 40), 16 /*overhang*/, 0, 0, 0, 0);
// declare_object_at_cell((1560 / 40) + 1, -316, -(2960 / 40), 16 /*overhang*/, 0, 90, 0, 0);
// declare_object_at_cell(-(640 / 40) + 1, -190, -(2240 / 40), 16 /*overhang*/, 0, -45, 15, 0);
// declare_object_at_cell(-(400 / 40) + 1, -180, -(2600 / 40), 16 /*overhang*/, 0, -45, 15, 0);

// declare_object_at_cell((760 / 40) + 1, -167, -(80 / 40), 17 /*pier1*/, 0, 45, 0, 0);
// declare_object_at_cell((1200 / 40) + 1, -97, (2280 / 40), 17 /*pier1*/, 0, 45, 0, 0);

// declare_object_at_cell((1680 / 40) + 1, -65, -(1320 / 40), 18 /*tunnel1*/, 0, 0, 0, 0);

// declare_object_at_cell(-(1280 / 40) + 1, -299, (1600 / 40), 19 /*tunnel2*/, 0, -58, 0, 0);

// declare_object_at_cell((1320 / 40) + 1, -133, (840 / 40), 21 /*wall1*/, 0, 140, 0, 0);
// declare_object_at_cell(-(1400 / 40) + 1, -262, (2600 / 40), 21 /*wall1*/, 0, 180, 0, 0);
// declare_object_at_cell(-(320 / 40) + 1, -226, (720 / 40), 21 /*wall1*/, 0, 180, 0, 0);
// declare_object_at_cell(-(960 / 40) + 1, -208, (360 / 40), 21 /*wall1*/, 0, 40, 0, 0);
// declare_object_at_cell((320 / 40) + 1, -397, -(680 / 40), 21 /*wall1*/, 0, 40, 0, 0);
// declare_object_at_cell((640 / 40) + 1, -369, -(2520 / 40), 21 /*wall1*/, 0, 0, 0, 0);

// declare_object_at_cell(-(1360 / 40) + 1, -205, (3280 / 40), 22 /*build00?*/, 0, 45, 0, 0);


/* level3 ? */
//Likely will replace

// declare_object_at_cell(-(20 / 40) + 1, -120, (940 / 40), 61 /*start location*/, 0, 0, 0, 0);

// declare_object_at_cell((1420 / 40) + 1, -63,  -(1260 / 40), 11 /*build00*/, 0, -20, 7, 0);
// declare_object_at_cell((420 / 40) + 1, -352,  -(60 / 40), 11 /*build00*/, 0, 0, 0, 0);


// declare_object_at_cell((1770 / 40) + 1, -78,  -(1300 / 40), 12 /*build01*/, 0, 0, 0, 0);

// declare_object_at_cell((360 / 40) + 1, -99,  -(1220 / 40), 12 /*build01*/, 0, 0, 0, 0);
// declare_object_at_cell((660 / 40) + 1, -71,  -(1220 / 40), 12 /*build01*/, 0, 0, 0, 0);

// declare_object_at_cell((2340 / 40) + 1, -44,  -(1180 / 40), 13 /*build02*/, 0, 0, 0, 0);

// declare_object_at_cell(-(20 / 40) + 1, -60,  (1300 / 40), 10 /*block01*/, 0, 0, 0, 0);
// declare_object_at_cell((420 / 40) + 1, -435,  -(60 / 40), 10 /*block01?*/, 0, 0, 0, 0);

// declare_object_at_cell((1260 / 40) + 1, -128,  (1180 / 40), 12 /*build02*/, 0, 0, 0, 0);
// declare_object_at_cell((1620 / 40) + 1, -103,  (1020 / 40), 12 /*build02*/, 0, 0, 0, 0);

// declare_object_at_cell((660 / 40) + 1, -97,  (1300 / 40), 14 /*build03*/, 0, 0, 0, 0);
// declare_object_at_cell(-(620 / 40) + 1, -97,  (1300 / 40), 14 /*build03*/, 0, 0, 0, 0);

/* level 4 ? */
// Hell Run level. I like it.

// declare_object_at_cell((460 / 40) + 1, -340, -(4820 / 40), 61 /*start location*/, 0, 0, 0, 0);

// declare_object_at_cell((140 / 40) + 1, -211,  -(1020 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell((180 / 40) + 1, -197,  -(480 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell((220 / 40) + 1, -351,  (1060 / 40), 10 /*platf00*/, 0, 0, 0, 0);

// declare_object_at_cell((260 / 40) + 1, -270,  -(4340 / 40), 23 /*bridge2*/, 0, 30, 0, 0);
// declare_object_at_cell((300 / 40) + 1, -426,  (860 / 40), 23 /*bridge2*/, 0, 30, 0, 0);
// declare_object_at_cell((300 / 40) + 1, -426,  (1260 / 40), 23 /*bridge2*/, 0, -30, 0, 0);

// declare_object_at_cell((140 / 40) + 1, -245, -(4140 / 40), 16 /*overhang*/, 0, 115, 0, 0);
// declare_object_at_cell((20 / 40) + 1, -289, -(3100 / 40), 16 /*overhang*/, 0, 90, 0, 0);

// declare_object_at_cell((260 / 40) + 1, -340, (2100 / 40), 16 /*overhang*/, 0, 90, 0, 0);

// declare_object_at_cell((180 / 40) + 1, -201, -(1220 / 40), 16 /*overhang*/, 20, 0, 0, 0);
// declare_object_at_cell((100 / 40) + 1, -201, -(1220 / 40), 16 /*overhang*/, 20, 0, 0, 0);

// declare_object_at_cell((220 / 40) + 1, -152, -(740 / 40), 16 /*overhang*/, 14, 0, 0, 0);
// declare_object_at_cell((140 / 40) + 1, -152, -(740 / 40), 16 /*overhang*/, 14, 0, 0, 0);

// declare_object_at_cell(-(140 / 40) + 1, -381, (3340 / 40), 24 /*OBSTCL1*/, 0, 0, 0, 0);
// declare_object_at_cell((60 / 40) + 1, -381, -(2300 / 40), 24 /*OBSTCL1*/, 0, 0, 0, 0);
// declare_object_at_cell((300 / 40) + 1, -383, (220 / 40), 24 /*OBSTCL1*/, 0, 0, 0, 0);

// declare_object_at_cell((100 / 40) + 1, -389, -(2300 / 40), 9 /*KYOOB*/, 0, 0, 0, 0);
// declare_object_at_cell((260 / 40) + 1, -393, (220 / 40), 9 /*KYOOB*/, 0, 0, 0, 0);

// declare_object_at_cell(-(20 / 40) + 1, -394, -(1900 / 40), 13 /*greece02*/, 0, 0, 0, 0);
// declare_object_at_cell((340 / 40) + 1, -310, -(300 / 40), 13 /*greece02*/, 0, 180, 0, 0);

// declare_object_at_cell(-(20 / 40) + 1, -398, (4420 / 40), 13 /*greece02*/, 0, 45, 0, 0);
// declare_object_at_cell(-(300 / 40) + 1, -398, (4700 / 40), 13 /*greece02*/, 0, 225, 0, 0);

// declare_object_at_cell(-(180 / 40) + 1, -398, (4540 / 40), 15 /*greece04*/, 0, 0, 0, 0);

// declare_object_at_cell(-(140 / 40) + 1, -296, (2940 / 40), 21 /*wall1*/, 0, 0, 60, 0);
// declare_object_at_cell(-(140 / 40) + 1, -277, (3820 / 40), 21 /*wall1*/, -60, -90, 0, 0);

/* level5 ? */
//Like this level, will keep.

declare_object_at_cell(-(860 / 40) + 1, -330, (460 / 40), 61 /*start location*/, 0, 0, 0, 0);

declare_object_at_cell((740 / 40) + 1, -148, (740 / 40), 16 /*overhang*/, 0, 45, 0, 0);
declare_object_at_cell(-(1180 / 40) + 1, -126, (1020 / 40), 16 /*overhang*/, 0, 45, 0, 0);
declare_object_at_cell((1740 / 40) + 1, -399, (1780 / 40), 16 /*overhang*/, 0, 45, 0, 0);

declare_object_at_cell(-(500 / 40) + 1, -334,  (580 / 40), 10 /*platf00*/, 0, 0, 0, 0);
declare_object_at_cell(-(340 / 40) + 1, -346,  (380 / 40), 10 /*platf00*/, 0, 0, 0, 0);
declare_object_at_cell((1020 / 40) + 1, -196,  (460 / 40), 10 /*platf00*/, 0, 0, 0, 0);

declare_object_at_cell((860 / 40) + 1, -22, -(700 / 40), 21 /*wall1*/, 0, -135, 0, 0);
declare_object_at_cell((340 / 40) + 1, -181, -(1340 / 40), 21 /*wall1*/, 75, 90, 0, 0);

declare_object_at_cell((1380 / 40) + 1, -284, (1380 / 40), 23 /*bridge2*/, 0, -135, 0, 0);
declare_object_at_cell((660 / 40) + 1, -173, -(900 / 40), 23 /*bridge2*/, 0, -45, 0, 0);

declare_object_at_cell(-(420 / 40) + 1, -164, -(1020 / 40), 17 /*pier1*/, 0, 120, 0, 0);

declare_object_at_cell(-(860 / 40) + 1, -310, -(100 / 40), 20 /*tunnl3*/, 0, 90, 0, 0);

declare_object_at_cell((300 / 40) + 1, -321, -(20 / 40), 14 /*greece03*/, 0, 0, 0, 0);

declare_object_at_cell(-(60 / 40) + 1, -370, (180 / 40), 25 /*float01*/, 0, 0, 0, 0);

}

//I'm not sure if this whole system is ideal.
//But if I really do end up limited to 256 objects, really.. honestly... it should be okay, it's not logically intensive, and not intense on the bus either.
// Yet for collectibles, on these big maps, is that enough?
// .. Yeah. Yeah, it probably is. Even if it's 512.

void	object_control_loop(int ppos[XY])
{	
	if(ldata_ready != true) 
	{
	//////////////////////////////////////////////////////////////////////
	// **TESTING**
	//////////////////////////////////////////////////////////////////////
	// ldata_ready = true;
	// declare_object_at_cell(-8, 0, -8, 26 /*House*/, 0, 0, 0);
	// declare_object_at_cell(8, 0, -8, 27 /*House*/, 0, 0, 0);
	// declare_object_at_cell(8, 0, 8, 28 /*House*/, 0, 0, 0);
	// declare_object_at_cell(-8, 0, 8, 29 /*House*/, 0, 0, 0);
	//////////////////////////////////////////////////////////////////////
	// **TESTING**
	//////////////////////////////////////////////////////////////////////
		return;
	}		//Just in case.
	static int difX = 0;
	static int difY = 0;
	static int difH = 0;
	static int position_difference[XYZ] = {0,0,0};
	objUP = 0;

//Notice: Maximum collision tested & rendered items is MAX_PHYS_PROXY
	for(int i = 0; i < objNEW; i++){
		
		//jo_printf(0, 0, "(VDP1_BASE_CMDCTRL)"); //Debug ONLY
		
		difX = fxm(JO_ABS((ppos[X] * CELL_SIZE) + dWorldObjects[i].pos[X]) - (dWorldObjects[i].type.radius[X]<<16), INV_CELL_SIZE)>>16; 
		difY = fxm(JO_ABS((ppos[Y] * CELL_SIZE) + dWorldObjects[i].pos[Z]) - (dWorldObjects[i].type.radius[Z]<<16), INV_CELL_SIZE)>>16; 
		difH = JO_ABS(you.pos[Y] + dWorldObjects[i].pos[Y]);
		
		if((dWorldObjects[i].type.ext_dat & OTYPE) == LDATA)
		{ 		
				////////////////////////////////////////////////////
				//If the object type declared is LDATA (level data), use a different logic branch.
				////////////////////////////////////////////////////
				if(difH < HEIGHT_CULLING_DIST && difX < CELL_CULLING_DIST_MED && difY < CELL_CULLING_DIST_MED) 
				{
					//Get the position difference. This is uniquely used for level data collision.
					//For now, at least.
					position_difference[X] = JO_ABS(you.pos[X] + dWorldObjects[i].pos[X]);
					position_difference[Y] = JO_ABS(you.pos[Y] + dWorldObjects[i].pos[Y]);
					position_difference[Z] = JO_ABS(you.pos[Z] + dWorldObjects[i].pos[Z]);
					
					// slPrintFX(position_difference[X], slLocate(2, 7));
					// slPrintFX(position_difference[Y], slLocate(2, 8));
					// slPrintFX(position_difference[Z], slLocate(2, 9));
					
					if((dWorldObjects[i].type.ext_dat & LDATA_TYPE) == SOUND_TRIG && !(dWorldObjects[i].type.ext_dat & OBJPOP))
					{	
						// "360" is a magic number to convert the rotation angle into the literal size.
						// This is used for radius of collision with the trigger.
						if(position_difference[X] < (dWorldObjects[i].rot[X] * 360)
						&& position_difference[Y] < (dWorldObjects[i].rot[Y] * 360)
						&& position_difference[Z] < (dWorldObjects[i].rot[Z] * 360))
						{
							if((dWorldObjects[i].type.ext_dat & SDTRIG_PCM) == SDTRIG_PCM)
							{
								pcm_play(dWorldObjects[i].more_data & 0xFF, PCM_PROTECTED, dWorldObjects[i].more_data>>8);
								dWorldObjects[i].type.ext_dat |= OBJPOP;
							} else {
								start_adx_stream((Sint8*)"TESTSND.ADX", dWorldObjects[i].more_data>>8);
								dWorldObjects[i].type.ext_dat |= OBJPOP;
							}
						}

					}
					if((dWorldObjects[i].type.ext_dat & LDATA_TYPE) == LEVEL_CHNG)
					{
						// We've found a level change trigger close to the player.
						// If we are close enough to the level change trigger and it is enabled, change levels.
						
						if(position_difference[X] < (dWorldObjects[i].type.radius[X]<<16)
						&& position_difference[Y] < (dWorldObjects[i].type.radius[Y]<<16)
						&& position_difference[Z] < (dWorldObjects[i].type.radius[Z]<<16)
						//Enabling Booleans
						&& !(dWorldObjects[i].type.ext_dat & OBJPOP) && (dWorldObjects[i].type.ext_dat & 0x80))
						{
							//////////////////////////////////////////
							// Temporary, but will change levels.
							//////////////////////////////////////////
							dWorldObjects[i].type.ext_dat |= OBJPOP;
							pcm_play(snd_win, PCM_PROTECTED, 7);
							map_chg = false;
							//p64MapRequest(dWorldObjects[i].type.entity_ID);
							///////////////////////////////////////////
							// More temporary stuff.
							///////////////////////////////////////////
							you.points = 0;
						}
					}
				}
		} else if(difX < CELL_CULLING_DIST_MED && difY < CELL_CULLING_DIST_MED && difH < HEIGHT_CULLING_DIST && objUP < MAX_PHYS_PROXY)
			{

				if((dWorldObjects[i].type.ext_dat & OTYPE) != BUILD)
				{
					////////////////////////////////////////////////////
					//If a non-building object was in rendering range, specify it as being populated, 
					//and give it matrix/bounding box parameters.
					////////////////////////////////////////////////////
					bound_box_starter.modified_box = &RBBs[objUP];
					bound_box_starter.x_location = dWorldObjects[i].pos[X];
					//Y location has to find the value of a pixel of the map and add it with object height off ground + Y radius
					bound_box_starter.y_location = dWorldObjects[i].pos[Y];/* - ((used_radius[Y])<<16)
					- (main_map[
					(-dWorldObjects[i].pix[X] + (main_map_x_pix * dWorldObjects[i].pix[Y]) + (main_map_total_pix>>1)) 
					]<<(MAP_V_SCALE));*/
					//
					bound_box_starter.z_location = dWorldObjects[i].pos[Z];
					bound_box_starter.x_rotation = dWorldObjects[i].rot[X];
					bound_box_starter.y_rotation = dWorldObjects[i].rot[Y];
					bound_box_starter.z_rotation = dWorldObjects[i].rot[Z];

					bound_box_starter.x_radius = dWorldObjects[i].type.radius[X]<<16;
					bound_box_starter.y_radius = dWorldObjects[i].type.radius[Y]<<16;
					bound_box_starter.z_radius = dWorldObjects[i].type.radius[Z]<<16;
							
					make2AxisBox(&bound_box_starter);

						////////////////////////////////////////////////////
						//Set the box status. This branch of the logic dictates the box is:
						// 1. Render-able
						// 2. Collidable
						// 3. May or may not emit light
						////////////////////////////////////////////////////
						RBBs[objUP].status[0] = 'R';
						RBBs[objUP].status[1] = 'C';
						RBBs[objUP].status[2] = (dWorldObjects[i].type.light_bright != 0) ? 'L' : 'N';
					//Bit 15 of ext_dat is a flag that will tell the system if the object is on or not.
					dWorldObjects[i].type.ext_dat |= OBJPOP;
					//This array is meant as a list where iterative searches find the entity type drawn.
					objDRAW[objUP] = dWorldObjects[i].type.entity_ID;
					//This array is meant on a list where iterative searches can find the right object in the entire declared list.
					activeObjects[objUP] = i;
					//This tells you how many objects were updated.
					objUP++; 
					} else if((dWorldObjects[i].type.ext_dat & OTYPE) == BUILD)
				{
						////////////////////////////////////////////////////
						// Generate valid matrix parameters for the building.
						////////////////////////////////////////////////////
					bound_box_starter.modified_box = &RBBs[objUP];
					bound_box_starter.x_location = dWorldObjects[i].pos[X];
					//Y location has to find the value of a pixel of the map and add it with object height off ground + Y radius
					bound_box_starter.y_location = dWorldObjects[i].pos[Y];/* - ((used_radius[Y])<<16)
					- (main_map[
					(-dWorldObjects[i].pix[X] + (main_map_x_pix * dWorldObjects[i].pix[Y]) + (main_map_total_pix>>1)) 
					]<<(MAP_V_SCALE));*/
					//
					bound_box_starter.z_location = dWorldObjects[i].pos[Z];
					bound_box_starter.x_rotation = 0;
					bound_box_starter.y_rotation = 0;
					bound_box_starter.z_rotation = 0;
					
					bound_box_starter.x_radius = dWorldObjects[i].type.radius[X]<<16;
					bound_box_starter.y_radius = dWorldObjects[i].type.radius[Y]<<16;
					bound_box_starter.z_radius = dWorldObjects[i].type.radius[Z]<<16;
							
					make2AxisBox(&bound_box_starter);
					
					/////////////////////////////////////////////////////
					// This object is a building. 
					// If this building has not yet been checked for items registered to it,
					// check the building payload list to see if there are any items assigned to its entity ID.
					// If there are any, register them in the declared object list.
					// After that, flag this building object's "more data" with something to say
					// its items have already been registered.
					/////////////////////////////////////////////////////
					if(dWorldObjects[i].more_data == 0 &&
						entities[dWorldObjects[i].type.entity_ID].file_done == true)
					{
						for(int b = 0; b < total_building_payload; b++)
						{
							declare_building_object(&dWorldObjects[i], &BuildingPayload[b]);
						}
						dWorldObjects[i].more_data = 1;
					}
						////////////////////////////////////////////////////
						//Set the box status. 
						//There isn't really a bound box for buildings.
						//They only need to be rendered, and in a special way, too.
						////////////////////////////////////////////////////
						RBBs[objUP].status[0] = 'R';
						RBBs[objUP].status[1] = 'C';
						RBBs[objUP].status[2] = 'N';
					//Bit 15 of ext_dat is a flag that will tell the system if the object is on or not.
					dWorldObjects[i].type.ext_dat |= OBJPOP;
					//This array is meant as a list where iterative searches find the entity type drawn.
					objDRAW[objUP] = dWorldObjects[i].type.entity_ID;
					//This array is meant on a list where iterative searches can find the right object in the entire declared list.
					activeObjects[objUP] = i;
					//This tells you how many objects were updated.
					objUP++; 
				}
			////////////////////////////////////////////////////
			// Object in render-range end stub
			////////////////////////////////////////////////////
		} else if(difX < CELL_CULLING_DIST_LONG && difY < CELL_CULLING_DIST_LONG && difH < HEIGHT_CULLING_DIST && objUP < MAX_PHYS_PROXY)
			{
				if((dWorldObjects[i].type.ext_dat & OTYPE) != BUILD && dWorldObjects[i].type.light_bright != 0)
				{
				////////////////////////////////////////////////////
				//If a non-building light-emitting object is in this larger range, add its light data to the light list.
				//But do not flag it to render or be collision-tested.
				////////////////////////////////////////////////////
					bound_box_starter.modified_box = &RBBs[objUP];
					bound_box_starter.x_location = dWorldObjects[i].pos[X];
					//Y location has to find the value of a pixel of the map and add it with object height off ground + Y radius
					bound_box_starter.y_location = dWorldObjects[i].pos[Y];/* - ((used_radius[Y])<<16)
					- (main_map[
					(-dWorldObjects[i].pix[X] + (main_map_x_pix * dWorldObjects[i].pix[Y]) + (main_map_total_pix>>1)) 
					]<<(MAP_V_SCALE));*/
					//
					bound_box_starter.z_location = dWorldObjects[i].pos[Z];
					make2AxisBox(&bound_box_starter);
						////////////////////////////////////////////////////
						//Set the box status. This branch of the logic dictates the box is:
						// 1. Not render-able
						// 2. Not collide-able
						// 3. May emit light
						////////////////////////////////////////////////////
						RBBs[objUP].status[0] = 'N';
						RBBs[objUP].status[1] = 'N';
						RBBs[objUP].status[2] = 'L';
					//Bit 15 of ext_dat is a flag that will tell the system if the object is on or not.
					dWorldObjects[i].type.ext_dat |= OBJPOP;
					//This array is meant as a list where iterative searches find the entity type drawn.
					objDRAW[objUP] = dWorldObjects[i].type.entity_ID;
					//This array is meant on a list where iterative searches can find the right object in the entire declared list.
					activeObjects[objUP] = i;
					//This tells you how many objects were updated.
					objUP++; 
				}
			////////////////////////////////////////////////////
			// Object in light-range end stub
			////////////////////////////////////////////////////
				} else {
					////////////////////////////////////////////////////
					//If the declared object was not in range, specify it as being unpopulated.
					////////////////////////////////////////////////////
					activeObjects[objUP] = 256;
					dWorldObjects[i].type.ext_dat &= UNPOP; //Axe bit 15 but keep all other data.
				}
			////////////////////////////////////////////////////
			//Object control loop end stub
			////////////////////////////////////////////////////
		}
		
	flush_boxes(objUP);
		
	// jo_printf(12, 5, "objUP:(%i)", objUP);
	// jo_printf(12, 6, "objNW:(%i)", objNEW);
	////////////////////////////////////////////////////
	//Object control function end stub
	////////////////////////////////////////////////////
}

void	light_control_loop(void)
{
	//First, purge the light list.
	for(int p = 0; p < MAX_DYNAMIC_LIGHTS; p++)
	{
		active_lights[p].pop = 0;
	}
	//Then, check if any active object, up to the light source limit, should emit a light. If it does, add it to the light list,
	//given the light_y_offset of the light's source alongside the object's location.
	unsigned char lights_created = 0;
	
	
	//There is something whacky going on here and I don't like it.
	// I have had a problem with this code for a long, long time.
	// I don't know what's going on...
	
	///////////////////////////
	// Test light
	// DO NOT leave active when pushing live!!!
	///////////////////////////
	// active_lights[0].bright = 2000;
	// active_lights[0].pos[X] = -(160<<16);
	// active_lights[0].pos[Y] = 60<<16;
	// active_lights[0].pos[Z] = (100<<16);
	// active_lights[0].pop = 1;
	
	for(int i = 0; i < MAX_PHYS_PROXY; i++)
	{
		if(RBBs[i].status[2] == 'L')
			{
				if(lights_created < MAX_DYNAMIC_LIGHTS)
				{
					active_lights[lights_created].pop = 1;
					active_lights[lights_created].ambient_light = active_lights[0].ambient_light;
					active_lights[lights_created].bright = dWorldObjects[activeObjects[i]].type.light_bright;
					active_lights[lights_created].pos[X] = -RBBs[i].pos[X];
					active_lights[lights_created].pos[Y] = -(RBBs[i].pos[Y] + dWorldObjects[activeObjects[i]].type.light_y_offset);
					active_lights[lights_created].pos[Z] = -RBBs[i].pos[Z];
					lights_created++;
				} else {
					///////////////////////////
					// If the light list is full, but there are lights closer to you than the ones in the list,
					// replace one in the list with the nearer light.
					///////////////////////////
					for(int j = 0; j < MAX_DYNAMIC_LIGHTS; j++)
					{
						POINT newpos = {-active_lights[j].pos[X], -active_lights[j].pos[Y], -active_lights[j].pos[Z]};
						POINT ngpos = {-you.pos[X], -you.pos[Y], -you.pos[Z]};
						int dist0 = approximate_distance(newpos, ngpos);
						int dist1 = approximate_distance(RBBs[i].pos, ngpos);
						if(dist1 < dist0)
						{
							active_lights[j].ambient_light = active_lights[0].ambient_light;
							active_lights[j].bright = dWorldObjects[activeObjects[i]].type.light_bright;
							active_lights[j].pos[X] = -RBBs[i].pos[X];
							active_lights[j].pos[Y] = -(RBBs[i].pos[Y] + dWorldObjects[activeObjects[i]].type.light_y_offset);
							active_lights[j].pos[Z] = -RBBs[i].pos[Z];
						}
					}
				}
				//slPrintFX(0, slLocate(2, 6+));
				//slPrintFX(active_lights[i].pos[X], slLocate(2, 7 + (i * 3)));
				//slPrintFX(active_lights[i].pos[Y], slLocate(2, 8 + (i * 3)));
				//slPrintFX(active_lights[i].pos[Z], slLocate(2, 9 + (i * 3)));
			}
	}
	
		// slPrintFX(active_lights[0].pos[X], slLocate(2, 7));
		// slPrintFX(active_lights[0].pos[Y], slLocate(2, 8));
		// slPrintFX(active_lights[0].pos[Z], slLocate(2, 9));
		
		// slPrintFX(active_lights[1].pos[X], slLocate(2, 7+3));
		// slPrintFX(active_lights[1].pos[Y], slLocate(2, 8+3));
		// slPrintFX(active_lights[1].pos[Z], slLocate(2, 9+3));

	
	// jo_printf(2, 10, "(%i) lights", lights_created);
	// jo_printf(2, 12, "(%i) obj", objUP);
	
}

//I hate this function.
void	add_to_track_timer(int index) 
{
	
	short trackedLDATA = link_starts[LDATA>>12];
	short track_select = 0;
	short object_track = 0;

	while(trackedLDATA != -1){
		if( (dWorldObjects[trackedLDATA].type.ext_dat & LDATA_TYPE) == TRACK_DATA)
		{//WE FOUND SOME TRACK DATA
			object_track = (dWorldObjects[index].type.ext_dat & 0xF00)>>8; //Get the level data's track #
			track_select = dWorldObjects[trackedLDATA].type.entity_ID & 0xF; 
			if(track_select == object_track && (activeTrack == track_select || activeTrack == -1))
			{
				activeTrack = dWorldObjects[trackedLDATA].type.entity_ID & 0xF;
				trackTimers[object_track] += (dWorldObjects[trackedLDATA].type.ext_dat & 0xF)<<17;
				pcm_play(snd_button, PCM_PROTECTED, 7);
				break;
			}
		}//PAST TRACK DATA
			trackedLDATA = dWorldObjects[trackedLDATA].link;
	}
}

void	has_entity_passed_between(short obj_id1, short obj_id2, _boundBox * tgt)
{	
	//////////////////
	// If the gate has no pair, return.
	// If the entity has yet to be loaded, return.
	// Otherwise, flag the posts has having been checked this frame, then continue.
	//////////////////
	if(obj_id1 == obj_id2) return;
	if(entities[dWorldObjects[obj_id1].type.entity_ID].file_done != true) return;
	
	dWorldObjects[obj_id1].type.ext_dat |= 2; 
	dWorldObjects[obj_id2].type.ext_dat |= 2; 
	
	//////////////////
	// By default, use the entity radius.
	// If a radius was manually set, use that instead.
	//////////////////
	unsigned short * used_radius = &entities[dWorldObjects[obj_id1].type.entity_ID].radius[X];
	if(dWorldObjects[obj_id1].type.radius[Y] > 0) used_radius = &dWorldObjects[obj_id1].type.radius[X];
	POINT fenceA;
	POINT fenceB;
	POINT fenceC;
	POINT fenceD;
	POINT tgtRelPos = {0, 0, 0};
	VECTOR rminusb = {0, 0, 0};
	VECTOR sminusb = {0, 0, 0};
	VECTOR cross = {0, 0, 0};
	VECTOR faceNormal = {0, 0, 0};
	VECTOR edgePrj0 = {0, 0, 0};
	POINT centerFace = {0, 0, 0};
	short posts[2];
	FIXED radius1;
	FIXED radius2;
	FIXED bigRadius;
	int tDist = 0;
	
		posts[0] = (obj_id1 > obj_id2) ? obj_id1 : obj_id2;
		posts[1] = (obj_id1 > obj_id2) ? obj_id2 : obj_id1; //Order the objects so the face always has the same normal
	
	//Extrapolate a quad out of the pix given
	//	0 - 1 // B - D
	//	3 - 2 // A - C
	fenceA[X] = -dWorldObjects[posts[0]].pos[X];
	fenceA[Y] = -((dWorldObjects[posts[0]].pos[Y]) - (main_map[ (-dWorldObjects[posts[0]].pix[X] + (main_map_x_pix * dWorldObjects[posts[0]].pix[Y]) + (main_map_total_pix>>1)) ]<<(MAP_V_SCALE)));
	fenceA[Z] = -dWorldObjects[posts[0]].pos[Z];
	
	fenceB[X] = fenceA[X];
	fenceB[Y] = fenceA[Y] + (used_radius[Y]<<16);
	fenceB[Z] = fenceA[Z];
	
	fenceC[X] = -dWorldObjects[posts[1]].pos[X];
	fenceC[Y] = -((dWorldObjects[posts[1]].pos[Y]) - (main_map[ (-dWorldObjects[posts[1]].pix[X] + (main_map_x_pix * dWorldObjects[posts[1]].pix[Y]) + (main_map_total_pix>>1)) ]<<(MAP_V_SCALE)));
	fenceC[Z] = -dWorldObjects[posts[1]].pos[Z];
	
	fenceD[X] = fenceC[X];
	fenceD[Y] = fenceC[Y] + (used_radius[Y]<<16);
	fenceD[Z] = fenceC[Z];

		//Start math for projection
	centerFace[X] = (fenceA[X] + fenceC[X] + fenceB[X] + fenceD[X])>>2;
	centerFace[Y] = (fenceA[Y] + fenceC[Y] + fenceB[Y] + fenceD[Y])>>2;
	centerFace[Z] = (fenceA[Z] + fenceC[Z] + fenceB[Z] + fenceD[Z])>>2;
		//Get a relative position
	tgtRelPos[X] = tgt->pos[X] - centerFace[X];
	tgtRelPos[Y] = tgt->pos[Y] - centerFace[Y];
	tgtRelPos[Z] = tgt->pos[Z] - centerFace[Z];
		//If you are farther away from the face in X and Z than the face's big radius, you can't possibly have collided with it.
		//Note: Potential for this to be an incorrect conclusion on things moving really fast with small fences.
		radius1 = JO_ABS(fenceA[X] - fenceC[X]);
		radius2 = JO_ABS(fenceA[Z] - fenceC[Z]);
		bigRadius =  (radius1 > radius2) ? radius1 : radius2;
	if(JO_ABS(tgtRelPos[X]) > bigRadius || JO_ABS(tgtRelPos[Z]) > bigRadius)
	{
		//Data cleanup to prevent errant positive detection when culling logic is passed,
		dWorldObjects[obj_id1].dist = 0; //when previously it may noy have been.
		return;
	};
		//Project to a horizontal edge of the face.
		//More specifically, this projects to the vector of that edge,
		//but since the projection is using a relative point rather than an absolute point,
		//it's already a projection in relation to the center of the face.
	POINT vfAfC = {fenceA[X] - fenceC[X], fenceA[Y] - fenceC[Y], fenceA[Z] - fenceC[Z]};
	VECTOR unitRelPos;
	VECTOR unitvfAfC;
	normalize(tgtRelPos, unitRelPos);
	normalize(vfAfC, unitvfAfC);

	line_intersection_function(tgt->pos, unitRelPos, fenceA, unitvfAfC, edgePrj0);
	
	
		//The first condition is if our projection is beyond the X or Z radius of the face, we want to stop now.
	if(JO_ABS(edgePrj0[X]) > JO_ABS(vfAfC[X]>>1) || JO_ABS(edgePrj0[Z]) > JO_ABS(vfAfC[Z]>>1) )
	{
		//Data cleanup to prevent errant positive detection when culling logic is passed,
		dWorldObjects[obj_id1].dist = 0; //when previously it may noy have been.
		return;
	};
		//Now if we are above or below the Y radius of the face, defined by the radius of the objects, we want to stop.
/* 	if( JO_ABS((tgtRelPos[Y]) - (edgePrj0[Y])) > (dWorldObjects[posts[0]].type.radius[Y]<<16) )
	{
		//Data cleanup to prevent errant positive detection when culling logic is passed,
		dWorldObjects[activeObjects[index]].dist = 0; //when previously it may noy have been.
		return;
	}; */
	
	//Then we do the math to find the normal of this area.
	//Makes a vector from point 3 to point 1.
	rminusb[X] = (fenceA[X] - fenceD[X]);
	rminusb[Y] = (fenceA[Y] - fenceD[Y]);
	rminusb[Z] = (fenceA[Z] - fenceD[Z]);
	//Makes a vector from point 2 to point 0.
	sminusb[X] = (fenceC[X] - fenceB[X]);
	sminusb[Y] = (fenceC[Y] - fenceB[Y]);
	sminusb[Z] = (fenceC[Z] - fenceB[Z]);
	
	fxcross(rminusb, sminusb, cross);
	
	cross[X] = cross[X]>>8;
	cross[Y] = cross[Y]>>8;
	cross[Z] = cross[Z]>>8;
	
	normalize(cross, faceNormal);
	//Then a collision detector.
	tDist = ptalt_plane(tgt->pos, faceNormal, centerFace);

	// slPrintFX(tDist, slLocate(0, 12));
	// slPrintFX(dWorldObjects[obj_id1].dist, slLocate(0, 13));
	// jo_printf(12, CELL_CULLING_DIST_MED, "(%i)", tDist ^ dWorldObjects[obj_id1].dist);

//Some way to check if the sign is different, also a safety to ensure at least 1 frame of checking has passed
	if( (tDist ^ dWorldObjects[obj_id1].dist) < 0 && dWorldObjects[obj_id1].dist != 0) 
	{
			dWorldObjects[obj_id1].dist = 0;
			dWorldObjects[posts[0]].type.ext_dat |= 0x1;
			dWorldObjects[posts[1]].type.ext_dat |= 0x1;
			add_to_track_timer(posts[0]);
		return;
	} else {
			dWorldObjects[obj_id1].dist = tDist;
		return;
	}
	
}

		//Presently function is unused so is technically incomplete (doesn't return or point to useful data).
		//For AI pathing, you.. uhh.. find a way.
/* void	walk_map_between_objects(short obj_id1, short obj_id2)
{
		//arrays below contain the in-order X and Y coordinates of cells that draw a line between the two objects.
		short pixXs[64];
		short pixYs[64];
		
		short cellDif[XY];
		cellDif[X] = JO_ABS(JO_ABS(dWorldObjects[obj_id1].pix[X]) - JO_ABS(dWorldObjects[obj_id2].pix[X]));
		cellDif[Y] = JO_ABS(JO_ABS(dWorldObjects[obj_id1].pix[Y]) - JO_ABS(dWorldObjects[obj_id2].pix[Y]));
		
		short totalCell = 0;
		short newCell[XY] = {dWorldObjects[obj_id1].pix[X], dWorldObjects[obj_id1].pix[Y]};
		Sint8 AddBool[XY];
		
		AddBool[X] = (dWorldObjects[obj_id1].pix[X] > dWorldObjects[obj_id2].pix[X]) ? -1 : 1;
		AddBool[Y] = (dWorldObjects[obj_id1].pix[Y] > dWorldObjects[obj_id2].pix[Y]) ? -1 : 1;
		
		//Less Strict Version. Simpler, faster code, but doesn't result in a "touch any cell" pathway.
		//It is a logical copy of how Jo Engine draws lines in the background layer.
		//This method makes more sense there since it is more visibly pleasing.
  		short error = 0;
		short e2 = 0;
		error = (cellDif[X] == cellDif[Y]) ? 0 : (cellDif[X] > cellDif[Y] ? cellDif[X] : -cellDif[Y])>>1;
    for (;;)
    {
				//First cell is where we started [0]
        pixXs[totalCell] = newCell[X];
		pixYs[totalCell] = newCell[Y];
		totalCell++;
				//Last cell is where we end up at
		if (newCell[X] == dWorldObjects[obj_id2].pix[X] && newCell[Y] == dWorldObjects[obj_id2].pix[Y]) break;
		e2 = error;
		
			error -= (e2 > -cellDif[X]) ? cellDif[Y] : 0;
			newCell[X] += (e2 > -cellDif[X]) ? AddBool[X] : 0;

			error += (e2 < cellDif[Y]) ? cellDif[X] : 0;
			newCell[Y] += (e2 < cellDif[Y]) ? AddBool[Y] : 0;
    } 
	
}
 */
//Function will check collision with a ITEM-type object and flag entities that have been collided with for removal.
//It will play a sound, and add to your points too.
void	run_item_collision(int index, _boundBox * tgt)
{
		VECTOR relPos = {0, 0, 0};
		FIXED realDist = 0;
		
			if( !(dWorldObjects[activeObjects[index]].type.ext_dat & 8) ){ //Check if root entity still exists
	//Root entity collision test
		relPos[X] = RBBs[index].pos[X] + tgt->pos[X];
		relPos[Y] = RBBs[index].pos[Y] + tgt->pos[Y];
		relPos[Z] = RBBs[index].pos[Z] + tgt->pos[Z];
		//ABS your too-big-check you idiot
		if( JO_ABS(relPos[X]) < (SQUARE_MAX) && JO_ABS(relPos[Y]) < (SQUARE_MAX) && JO_ABS(relPos[Z]) < (SQUARE_MAX)) //If too far away, don't test
		{
		
		realDist = slSquartFX( fxm(relPos[X], relPos[X]) + fxm(relPos[Y], relPos[Y]) + fxm(relPos[Z], relPos[Z]) ); //a^2 + b^2 = c^2
		dWorldObjects[activeObjects[index]].rot[Y] += 18; //Spin
		
			if(realDist < (dWorldObjects[activeObjects[index]].type.radius[Y]<<16) ) //Explicit radius collision test
				{
			dWorldObjects[activeObjects[index]].type.ext_dat |= 8; //Remove root entity from stack object
			dWorldObjects[activeObjects[index]].type.light_bright = 0; //Remove brightness
			you.points++;
			pcm_play(snd_click, PCM_SEMI, 7);
				}
		}
			}//Root entity check end
	
}

void	test_gate_ring(int index, _boundBox * tgt)
{
	//
	//Objective / Concept:
	// Test if player has passed through a ring,
	// based on a bound box object from a dWorldObject entry.
	// The radius of the ring being the span of the largest axis of the object.
	//The test method is a dot product test on the plane and whether or not we are within the largest radius distance.
	//
			if((dWorldObjects[activeObjects[index]].type.ext_dat & 0x1) != 0) return; //Return if the gate is already flagged as passed.
	VECTOR tgtRelPos = {tgt->pos[X] + RBBs[index].pos[X], tgt->pos[Y] + RBBs[index].pos[Y], tgt->pos[Z] + RBBs[index].pos[Z]}; 
		//Don't forget to ABS your junk, nut lover
			if(JO_ABS(tgtRelPos[X]) >= (SQUARE_MAX) || JO_ABS(tgtRelPos[Y]) >= (SQUARE_MAX) || JO_ABS(tgtRelPos[Z]) >= (SQUARE_MAX)) return; //Exit if distance is large
			
	POINT negPos = {-tgt->pos[X], -tgt->pos[Y], -tgt->pos[Z]}; //coordinate systems bruh
	
	int radiaSizX = RBBs[index].brad[X];
	int radiaSizY = RBBs[index].brad[Y];
	int radiaSizZ = RBBs[index].brad[Z];
	
	int leastRadius = JO_MIN(radiaSizX, JO_MIN(radiaSizY, radiaSizZ));
	int largeRadius = JO_MAX(radiaSizX, JO_MAX(radiaSizY, radiaSizZ));
	
	int tDist = 0;
	int tRelDist = 0;
	
	tRelDist = slSquartFX(fxdot(tgtRelPos, tgtRelPos)); 
	
	if(leastRadius == radiaSizX)
	{
		tDist = realpt_to_plane(negPos, RBBs[index].UVX, RBBs[index].pos);
	} else if(leastRadius == radiaSizY)
	{
		
		tDist = realpt_to_plane(negPos, RBBs[index].UVY, RBBs[index].pos);
		
	} else if(leastRadius == radiaSizZ)
	{
		
		tDist = realpt_to_plane(negPos, RBBs[index].UVZ, RBBs[index].pos);
	}

		if( fxm(tDist, dWorldObjects[activeObjects[index]].dist) < 1 && tRelDist < largeRadius)
		{	//If the sign of tDist varies from the sign of old object dist, and we are within the radius...
			//we've passed the plane of the gate's span, and are close enough to have done so within it.
		add_to_track_timer(activeObjects[index]);
		dWorldObjects[activeObjects[index]].type.ext_dat |= 0x1; //Flag gate as passed.
		}
	
	dWorldObjects[activeObjects[index]].dist = tDist;
}


void	test_gate_posts(int index, _boundBox * tgt)
{

			if((dWorldObjects[index].type.ext_dat & 0x1) != 0) return; //Return if the gate is already flagged as passed.
															
	short trackedEntry = link_starts[GATE_P>>12];
	unsigned short flagOne = dWorldObjects[index].type.ext_dat & 0x7FFF;
	unsigned short flagTwo = dWorldObjects[trackedEntry].type.ext_dat & 0x7FFF;

	//Goal: Check every entity in the GATE_P link list until the LINK is -1. When it is -1, stop. If it is equal to the current object's index, continue.
		while(trackedEntry != -1){
			//Do nothing, and continue to next entry if the entries are identical.
			if(trackedEntry != index)
			{				
				if(flagOne == flagTwo)
				{
					has_entity_passed_between(index, trackedEntry, tgt);
					
				}
				
				/////////////////
				// Rotation-orientation segment
				// This code body will automatically rotate posts' to "face" each other.
				// You can disable this code segment by flagging the object's "ext_dat" with 0x2 when declaring it.
				// This will rotate meshes such that the Z+ direction of the mesh will face the other gate post.
				/////////////////
				if(!(dWorldObjects[index].more_data & 0x1) && ((flagOne & 0xFF0) == (flagTwo & 0xFF0)))
				{
					
					//Orienting post 1
					//First, get vector to post 2.
					//Note that we only need the X/Z vector, or the XY of the map location.
					int posDif[XYZ] = {((dWorldObjects[index].pix[X] - dWorldObjects[trackedEntry].pix[X]) * CELL_SIZE)>>8, 0,
									((dWorldObjects[index].pix[Y] - dWorldObjects[trackedEntry].pix[Y]) * CELL_SIZE)>>8};
					accurate_normalize(posDif, posDif);
					dWorldObjects[index].rot[Y] = slAtan(posDif[Z], posDif[X]);
					dWorldObjects[index].more_data |= 0x1;
					
					// jo_printf(0, 10, "o1id(%i)", index);
					// jo_printf(10, 10, "o2id(%i)", trackedEntry);
					// jo_printf(3, 12, "data0(%x)", posDif[X]);
					// jo_printf(5, 13, "rot0(%i)", dWorldObjects[index].rot[Y]);
					// jo_printf(3, 14, "data1(%x)", posDif[Z]);
					// jo_printf(5, 15, "rot1(%i)", dWorldObjects[trackedEntry].rot[Y]);
					
				}
			}
			trackedEntry = dWorldObjects[trackedEntry].link; //Retrieve the declared entity ID of the next gate post from linked list
			flagTwo = dWorldObjects[trackedEntry].type.ext_dat & 0x7FFF; //Get the data of the next post (but ignore the POP?)
			// Explanation: The POP of two pieces of a gate may not always match, but all of the other data should.
			// This includes: Is it checked yet, is it passed yet, is it the same track, is it the same set of linked posts.
		}
}

//Idea:
//Make function that handles going through the linked lists.
_declaredObject * step_linked_object_list(_declaredObject * previous_entry)
{
	//In case the object is the last in the list, its link will be -1.
	//So do not try to go deeper in the list.
	//When we reach the last entry, return a safe, known memory address. In this case, the address of a new object.
		if(previous_entry->link >= 0)
		{
	return (_declaredObject *)&dWorldObjects[previous_entry->link];
		} else {
	return (_declaredObject *)&dWorldObjects[objNEW];
		}
}

_declaredObject * get_first_in_object_list(short object_type_specification)
{
	short first_object_id_num = link_starts[(object_type_specification & OTYPE)>>12];
	if(first_object_id_num >= 0)
	{
	return (_declaredObject *)&dWorldObjects[first_object_id_num];
	} else {
	//There's no objects of this type, so just point to the next new object.
	//It's safe, I guess? If there are open object slots....
	return (_declaredObject *)&dWorldObjects[objNEW];
	}
}

void	gate_track_manager(void)
{
	//Goal: Make a timer when you first pass through a gate ring or gate in a track
	//The timer will reset, and reset all the gates in the track, when it exceeds the time setting by LDATA
	//Also:
	//If the player's speed goes lower than the setting of the track's LDATA, it will reset.
	_declaredObject * someLDATA = get_first_in_object_list(LDATA);
	_declaredObject * somePOSTdata = get_first_in_object_list(GATE_P);
	_declaredObject * someRINGdata = get_first_in_object_list(GATE_R);
	
	static char track_reset[16] = 	{0, 0, 0, 0,
									0, 0, 0, 0,
									0, 0, 0, 0,
									0, 0, 0, 0};
	
	//More to do:
	//Minimum speed reset [maybe not for now]
	//First and last gate logic
	//What is this though?
	//A control loop for a specific level data type, as "track data". A track is a series of rings or gates that make a... track.
	//This keeps an eye on what's going on in each track.
	//The collision math is ran separately, in a more key spot in the entire physics structure.
	//This is a purpose-built function, but can be viewed as a method of game state tracking through linked lists.
	
	short track_select;
	short object_track;
	
	int num_track_dat =  0;
	static char complete_tracks = 0;
	
	// jo_printf(0, 15, "tim(%i)", (dWorldObjects[activeTrack].type.ext_dat & 0xF)<<17);
	// jo_printf(0, 16, "act(%i)", activeTrack);

	
	while(someLDATA != &dWorldObjects[objNEW]){
				//jo_printf(0, 0, "(GTMN)"); //Debug ONLY
		if( (someLDATA->type.ext_dat & LDATA_TYPE) == TRACK_DATA)
		{
		////////////////////////////////////////////////////////////////////////////////
		//
		// Level data, track data manager section
		// It's messy.
		//
		////////////////////////////////////////////////////////////////////////////////
		track_select = someLDATA->type.entity_ID & 0xF; //Get the level data's track #
		someLDATA->pix[X] = 0; //Re-set the passed/to-pass counters (pix x and pix y) of the track level data.
		someLDATA->pix[Y] = 0; //We do this every time because we count them up every time.
		somePOSTdata = get_first_in_object_list(GATE_P); //Re-set this link pointer (so we can re-scan)
		someRINGdata = get_first_in_object_list(GATE_R); //Re-set this link pointer (so we can re-scan)
		num_track_dat++;
		//jo_printf(1, 12, "ldats(%i)", num_track_dat);
		//jo_printf(1, 13, "track(%i)", track_select);
				if(activeTrack == -1 || (activeTrack == track_select)) // if active track.. or track released
					{
					// jo_printf(0, 17, "ldt(%i)", trackedLDATA);
					// jo_printf(0, 17, "ldt(%i)", someLDATA->more_data);
			while(someRINGdata != &dWorldObjects[objNEW]){
				//jo_printf(0, 0, "(RING)"); //Debug ONLY
				object_track = (someRINGdata->type.ext_dat & 0xF00)>>8; //Get object track to see if it matches the level data track
					if(track_select == object_track)
					{
						//Special magic numbers checking, i guess?
						if(someRINGdata->type.ext_dat & 0x1 && !(someLDATA->more_data & OBJPOP))
						{
							someLDATA->type.ext_dat |= OBJPOP; //will set the track data as ACTIVE 
							//I forget why I set this?
							someLDATA->pix[X]++;
						}
					
					//I still forget why I set this?
					someLDATA->pix[Y]++;
				//Reset if track reset enabled
						if(track_reset[track_select] == true)
						{
						//What even is track reset?
						someRINGdata->type.ext_dat &= 0xFFFE; 
						}
					}
			someRINGdata = step_linked_object_list(someRINGdata);
			}

			while(somePOSTdata != &dWorldObjects[objNEW]){
				//jo_printf(0, 0, "(POST)"); //Debug ONLY
				object_track = (somePOSTdata->type.ext_dat & 0xF00)>>8; //Get object track to see if it matches the level data track
				////////////////////////////////////////////////////
				// Flush the "checked collision yet" marker for gate posts.
				somePOSTdata->type.ext_dat &= 0xFFFD;
					if(track_select == object_track)
					{
						if(somePOSTdata->type.ext_dat & 0x1 && !(someLDATA->more_data & OBJPOP))
						{
							someLDATA->type.ext_dat |= OBJPOP; //will set the track data as ACTIVE 
							// The "X" pix of track data level data is the number of passed gates in the track.
							someLDATA->pix[X]++;
						}
					//The "Y" pix of a track data level data is the total number of gates in the track.
					//To complete the track, X must equal Y.
					someLDATA->pix[Y]++;
				//Reset if track reset enabled
						if(track_reset[track_select] == true)
						{
						somePOSTdata->type.ext_dat &= 0xFFFE; 
						}
					}
			somePOSTdata = step_linked_object_list(somePOSTdata);
			}
			track_reset[track_select] = false;
				//jo_printf(0, 0, "(LDAT)"); //Debug ONLY
			//Track completion logic
			if(someLDATA->pix[X] == someLDATA->pix[Y] && someLDATA->pix[X] != 0)
			{
				someLDATA->type.ext_dat &= UNPOP;	//Set track as inactive
				someLDATA->more_data |= OBJPOP;	//Set track as complete
				trackTimers[track_select] = 0;	//Re-set the track timer
				activeTrack = -1;	//Release active track
				you.points += 10 * someLDATA->pix[X];
				complete_tracks++;
				pcm_play(snd_cronch, PCM_PROTECTED, 7); //Sound
				slPrint("                           ", slLocate(0, 6));
				slPrint("                           ", slLocate(0, 7));
			}

			//Timer run & check
			if((someLDATA->type.ext_dat & OBJPOP) != 0)
			{
				trackTimers[track_select] -= delta_time;
					if(trackTimers[track_select] < 0) //If timer expired...
					{
						someLDATA->type.ext_dat &= UNPOP;
						track_reset[track_select] = true; //Reset tracks; timer expired
						trackTimers[track_select] = 0;
						activeTrack = -1; //Release active track
						//Sound stuff
						pcm_play(snd_alarm, PCM_PROTECTED, 7);
						//Clear screen in this zone
				slPrint("                           ", slLocate(0, 6));
				slPrint("                           ", slLocate(0, 7));
					}
			}
					}//if active track \ track end
		////////////////////////////////
		/// Track data manager end stub
		////////////////////////////////
		} else if((someLDATA->type.ext_dat & LDATA_TYPE) == LEVEL_CHNG)
		{
				//if(you.points <= 0x15)
				//{
					//If you haven't crossed all the tracks, disable the level changer.
				//	someLDATA->type.ext_dat &= 0xFF7F; 
				//} else {
					//If you have enough points and crossed all the tracks, enable the level changer.
					someLDATA->type.ext_dat |= 0x80;
			//	}
		}
		someLDATA = step_linked_object_list(someLDATA);
	}//while LDATA end
	
	//Completed all tracks, but only do anything if there are actually any tracks
	if(complete_tracks == num_track_dat && (num_track_dat > 0) && link_starts[LDATA>>12] > -1)
	{
		pcm_play(snd_win, PCM_PROTECTED, 7);
		complete_tracks = 0;
		//map_chg = false;
		//p64MapRequest(1);
	}
	
			if(activeTrack != -1){
				slPrint("Find the other wreath!", slLocate(0, 6));
				slPrintFX(trackTimers[activeTrack], slLocate(0, 7));
			}
			
	//slPrintHex(someLDATA->type.ext_dat, slLocate(13, 12));
	//jo_printf(13, 12, "ac_trk(%i)", activeTrack);
			
	// slPrintHex(dWorldObjects[5].dist, slLocate(0, 15));
	// slPrintHex(dWorldObjects[6].dist, slLocate(0, 16));
				// slPrintFX(trackTimers[0], slLocate(0, 5));
				// slPrintFX(trackTimers[1], slLocate(0, 6));
	// slPrintHex(dWorldObjects[5].pix[X], slLocate(0, 7));
	// slPrintHex(dWorldObjects[5].pix[Y], slLocate(0, 8));
	// slPrintHex(dWorldObjects[6].pix[X], slLocate(13, 7));
	// slPrintHex(dWorldObjects[6].pix[Y], slLocate(13, 8));
	
}


