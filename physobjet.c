#include <sl_def.h>
#include <SEGA_GFS.H>
#include "def.h"
#include "pcmsys.h"
#include "pcmstm.h"
#include "mymath.h"
#include "render.h"
#include "collision.h"
#include "ldata.h"
#include "hmap.h"
#include "object_col.h"
#include "menu.h"
#include "minimap.h"
#include "particle.h"

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
int objUP = 0;
int total_building_payload = 0;

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

_declaredObject * get_first_in_object_list(unsigned short object_type_specification)
{
	short first_object_id_num = link_starts[(object_type_specification & ETYPE)>>12];
	if(first_object_id_num >= 0)
	{
	return (_declaredObject *)&dWorldObjects[first_object_id_num];
	} else {
	//There's no objects of this type, so just point to the next new object.
	//It's safe, I guess? If there are open object slots....
	return (_declaredObject *)&dWorldObjects[objNEW];
	}
}

void	align_object_to_object(int index1, int index2)
{
	//Note that we only need the X/Z vector, or the XY of the map location.
	int posDif[XYZ] = {((dWorldObjects[index1].pix[X] - dWorldObjects[index2].pix[X]) * CELL_SIZE)>>8, 0,
					((dWorldObjects[index1].pix[Y] - dWorldObjects[index2].pix[Y]) * CELL_SIZE)>>8};
	accurate_normalize(posDif, posDif);
	dWorldObjects[index1].rot[Y] = slAtan(posDif[Z], posDif[X]);
	if((dWorldObjects[index1].type.ext_dat & ETYPE) == GATE_P) dWorldObjects[index1].more_data |= GATE_POST_ALIGNED;
}

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
		//Contention: Building-type is both a model-type and object-type.
		//Smartest to use the model type.
		if(entities[dWorldObjects[objNEW].type.entity_ID].type == MODEL_TYPE_BUILDING)
		{
			//Specific for building-type objects, place its entity ID in the ext_dat.
			dWorldObjects[objNEW].type.clone_ID = dWorldObjects[objNEW].type.entity_ID;
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
		if((dWorldObjects[objNEW].type.ext_dat & ETYPE) != LDATA && dWorldObjects[objNEW].type.radius[X] == 0 &&
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
		dWorldObjects[objNEW].pix[X] = root_object->pix[X];
		dWorldObjects[objNEW].pix[Y] = root_object->pix[Y];
		dWorldObjects[objNEW].type = *objList[building_item->object_type];
		dWorldObjects[objNEW].rot[X] = 0;
		dWorldObjects[objNEW].rot[Y] = 0;
		dWorldObjects[objNEW].rot[Z] = 0;
		dWorldObjects[objNEW].more_data = 0;
		dWorldObjects[objNEW].link = link_starts[(dWorldObjects[objNEW].type.ext_dat & 0x7000)>>12]; //Set object's link to the current link of this type
		link_starts[(dWorldObjects[objNEW].type.ext_dat & 0x7000)>>12] = objNEW; //Set the current link of this type to this entry
		////////////////////////////////////////////////////
		// If no radius was defined for the object, use the radius from the entity.
		// Must check if the entity is loaded, or else out of bounds access may occur.
		////////////////////////////////////////////////////
		if((dWorldObjects[objNEW].type.ext_dat & ETYPE) != LDATA && dWorldObjects[objNEW].type.radius[X] == 0 &&
			dWorldObjects[objNEW].type.radius[Y] == 0 && dWorldObjects[objNEW].type.radius[Z] == 0 &&
			entities[dWorldObjects[objNEW].type.entity_ID].file_done)
		{
			dWorldObjects[objNEW].type.radius[X] = entities[dWorldObjects[objNEW].type.entity_ID].radius[X];
			dWorldObjects[objNEW].type.radius[Y] = entities[dWorldObjects[objNEW].type.entity_ID].radius[Y];
			dWorldObjects[objNEW].type.radius[Z] = entities[dWorldObjects[objNEW].type.entity_ID].radius[Z];
		}
		
		dWorldObjects[objNEW].pos[X] = (root_object->pos[X] + ((int)building_item->pos[X]<<16));
		dWorldObjects[objNEW].pos[Y] = (root_object->pos[Y] + ((int)building_item->pos[Y]<<16));
		dWorldObjects[objNEW].pos[Z] = (root_object->pos[Z] + ((int)building_item->pos[Z]<<16));
		
		objNEW++;
	}
}

void	post_ldata_init_building_object_search(void)
{
	
	for(int i = 0; i < objNEW; i++)
	{
		if((dWorldObjects[i].type.ext_dat & ETYPE) == BUILD 
		&& !(dWorldObjects[i].more_data & BUILD_PAYLOAD_LOADED)
		&& entities[dWorldObjects[i].type.entity_ID].file_done == true)
		{
			/////////////////////////////////////////////////////
			// This object is a building. 
			// If this building has not yet been checked for items registered to it,
			// check the building payload list to see if there are any items assigned to its entity ID.
			// If there are any, register them in the declared object list.
			// After that, flag this building object's "more data" with something to say
			// its items have already been registered.
			/////////////////////////////////////////////////////
			for(int b = 0; b < total_building_payload; b++)
			{
				declare_building_object(&dWorldObjects[i], &BuildingPayload[b]);
			}
			//nbg_sprintf(1, 6, "tbp(%i)", total_building_payload);
						
			dWorldObjects[i].more_data |= BUILD_PAYLOAD_LOADED;
		}
	}
}

//
// It might be time to add a console / event-viewer to report goings-on of the game in text.
// Reason: One way to implement subtitles.
// Of course an easier alternative is just implement a subtitle system,
// and then use it for event reporting.
//
/**

Okey, what's next in the gameplay pipe?
Slide Hop is to have three "things to do". These aren't just the primary things to do, they are it. These' the things.

1 - Gates
a. Passing gates for discovery - done.
b. Silent timer after discovery - done.
c. cleaing gates by timer - done.
d. option for minimum speed track or gate - not done
e. allow rings in gate series - not tested
f. represent gate progress in menu - done
g. gate guide - done
h. gates show up on minimap - done
i. gate ring model - not done
j. gate post model - not done

2 - Seven Rings
a. Item types - done
b. 7 Rings with 7 unique item types - done
c. rings show up on minimap - done
d. represent ring progress in menu - done
e. 7 rings models - done enough
f. 7 rings sound effects - not done
g. timed lap 2 with rings - not done, maybe not needed
h. manager for items, manager for 7 ring items - done.

3 - CTF

a. flag stand - test model OK
b. flag stand shield - test model OK
c. goal stand - test model OK
d. jump on goal stand to unshield - done
e. flag - test model OK
f. carrying flag - done
g. time limit with flag - done
h. delivering flag - done
i. represent flag progress in menu - done
j. on-screen guide - done

4 - other things
a. particle effect system ? - kind of important
	1. collision for particles will be a challenge. a solved one, but a challenge
	2. particles need multiple types: no gravity, gravity, collision, no collision, etc
	3. particles are otherwise just billboard sprites
b. destruction of entity into particles ? - kind of important
c. shrink effect on entity ? - less but still kind of important
d. i am absolutely going to have to figure out texture cutting (in addition to the current tiling system)
e. HUD event system is done
f. 3D pad support + 3D pad menu? - pretty important, but low priority
g. streaming image system ? - for memes.. -- non-essential, would rather test levels

**/

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

	//////////////////////////////////////////////////////////////////////
	// **TESTING**
	//////////////////////////////////////////////////////////////////////
		return;
	}		//Just in case.
	static int difX = 0;
	static int difY = 0;
	static int difH = 0;
	static int position_difference[XYZ] = {0,0,0};
	objUP = 0; //Should we start this at -1, because -1 will mean there are no objects in scene?

//Notice: Maximum collision tested & rendered items is MAX_PHYS_PROXY
	for(int i = 0; i < objNEW; i++){
		
		//nbg_sprintf(0, 0, "(VDP1_BASE_CMDCTRL)"); //Debug ONLY
		
		difX = fxm(JO_ABS((ppos[X] * CELL_SIZE) + dWorldObjects[i].pos[X]) - (dWorldObjects[i].type.radius[X]<<16), INV_CELL_SIZE)>>16; 
		difY = fxm(JO_ABS((ppos[Y] * CELL_SIZE) + dWorldObjects[i].pos[Z]) - (dWorldObjects[i].type.radius[Z]<<16), INV_CELL_SIZE)>>16; 
		difH = JO_ABS(you.pos[Y] + dWorldObjects[i].pos[Y]);
		
		if((dWorldObjects[i].type.ext_dat & ETYPE) == LDATA)
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

					} else if((dWorldObjects[i].type.ext_dat & LDATA_TYPE) == LEVEL_CHNG)
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
							//pcm_play(snd_win, PCM_PROTECTED, 5);
							start_adx_stream(stmsnd[stm_win], 5);
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
				//Exit rendering for collected items or inactive objects
				if((dWorldObjects[i].type.ext_dat & ETYPE) == ITEM && (dWorldObjects[i].type.ext_dat & ITEM_COLLECTED)) continue;
				if((dWorldObjects[i].type.ext_dat & ETYPE) == OBJECT && (dWorldObjects[i].type.ext_dat & OBJECT_DISABLED)) continue;
				
				if(entities[dWorldObjects[i].type.entity_ID].type != MODEL_TYPE_BUILDING)
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
					RBBs[objUP].boxID = i;
					////////////////////////////////////////////////////
					//Set the box status. This branch of the logic dictates the box is:
					// 1. Render-able
					// 2. Collidable
					// 3. May or may not emit light
					////////////////////////////////////////////////////
					RBBs[objUP].status[0] = 'R';
					RBBs[objUP].status[1] = ((dWorldObjects[i].type.ext_dat & ETYPE) == GHOST) ? 'N' : 'C';
					RBBs[objUP].status[2] = (dWorldObjects[i].type.light_bright != 0) ? 'L' : 'N';
					//Bit 15 of ext_dat is a flag that will tell the system if the object is on or not.
					dWorldObjects[i].type.ext_dat |= OBJPOP;
					//This array is meant as a list where iterative searches find the entity type drawn.
					objDRAW[objUP] = dWorldObjects[i].type.entity_ID;
					//This array is meant on a list where iterative searches can find the right object in the entire declared list.
					activeObjects[objUP] = i;
					//This tells you how many objects were updated.
					objUP++; 
				} else if(entities[dWorldObjects[i].type.entity_ID].type == MODEL_TYPE_BUILDING)
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
					RBBs[objUP].boxID = i;
					
					////////////////////////////////////////////////////
					//Set the box status. 
					//There isn't really a bound box for buildings.
					//They only need to be rendered, and in a special way, too.
					////////////////////////////////////////////////////
					RBBs[objUP].status[0] = 'R';
					RBBs[objUP].status[1] = ((dWorldObjects[i].type.ext_dat & ETYPE) == GHOST) ? 'N' : 'C';
					RBBs[objUP].status[2] = (dWorldObjects[i].type.light_bright != 0) ? 'L' : 'N';
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
				if(dWorldObjects[i].type.light_bright != 0)
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
					RBBs[objUP].boxID = i;
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
		
	// nbg_sprintf(12, 5, "objUP:(%i)", objUP);
	// nbg_sprintf(12, 6, "objNW:(%i)", objNEW);
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

	
	// nbg_sprintf(2, 10, "(%i) lights", lights_created);
	// nbg_sprintf(2, 12, "(%i) obj", objUP);
	
}

//I hate this function.
void	add_to_track_timer(int index, int index2) 
{
	
	short trackedLDATA = link_starts[LDATA>>12];
	short ldata_track = 0;
	short object_track = 0;

	if(!(dWorldObjects[index].more_data & GATE_DISCOVERED))
	{
		dWorldObjects[index].more_data |= GATE_DISCOVERED;
		add_position_to_minimap(dWorldObjects[index].pix[X], dWorldObjects[index].pix[Y], 0x83E0);
		if(index2 >= 0) 
		{
			dWorldObjects[index2].more_data |= GATE_DISCOVERED;
			add_position_to_minimap(dWorldObjects[index2].pix[X], dWorldObjects[index2].pix[Y], 0x83E0);
		}
		pcm_play(snd_khit, PCM_PROTECTED, 5);
		you.points += 1;
		return;
	}

	while(trackedLDATA != -1){
		if( (dWorldObjects[trackedLDATA].type.ext_dat & LDATA_TYPE) == TRACK_DATA)
		{//WE FOUND SOME TRACK DATA
			object_track = (dWorldObjects[index].type.ext_dat & 0xF00)>>8; 
			ldata_track = dWorldObjects[trackedLDATA].type.entity_ID; 
	//	nbg_sprintf(2, 10, "(%i)otr", object_track);
	//	nbg_sprintf(2, 12, "(%i)trs", ldata_track);
			//Only add if the track numbers match, the active track is set to this track or is not set, and the track is discovered
			if(ldata_track == object_track && (dWorldObjects[trackedLDATA].type.ext_dat & TRACK_DISCOVERED))
			{
				//Gate flag processing
				dWorldObjects[index].dist = 0;
				dWorldObjects[index].type.ext_dat |= GATE_PASSED;
				add_position_to_minimap(dWorldObjects[index].pix[X], dWorldObjects[index].pix[Y], 0xFC00);
				if(index2 >= 0)
				{
					dWorldObjects[index2].type.ext_dat |= GATE_PASSED;
					add_position_to_minimap(dWorldObjects[index2].pix[X], dWorldObjects[index2].pix[Y], 0xFC00);
				}
				dWorldObjects[trackedLDATA].dist += dWorldObjects[trackedLDATA].type.light_y_offset<<16;
				dWorldObjects[trackedLDATA].more_data &= TRACK_NO_GUIDE;
				dWorldObjects[trackedLDATA].more_data |= (dWorldObjects[index].type.ext_dat & GATE_LINK_NUMBER)<<4;
				pcm_play(snd_button, PCM_PROTECTED, 5);
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
	// If the object is in the wrong direction to the other object, return.
	// Otherwise, flag the posts has having been checked this frame, then continue.
	//////////////////
	if(obj_id1 == obj_id2) return;
	if(dWorldObjects[obj_id1].pix[X] <= dWorldObjects[obj_id2].pix[X]) return;
	if(dWorldObjects[obj_id1].pix[Y] <= dWorldObjects[obj_id2].pix[Y]) return;
	if(entities[dWorldObjects[obj_id1].type.entity_ID].file_done != true) return;
	//Flag as checked this frame
	dWorldObjects[obj_id1].type.ext_dat |= GATE_POST_CHECKED; 
	dWorldObjects[obj_id2].type.ext_dat |= GATE_POST_CHECKED; 
	
	static POINT fenceA;
	static POINT fenceB;
	static POINT fenceC;
	static POINT fenceD;
	VECTOR rminusb = {0, 0, 0};
	VECTOR sminusb = {0, 0, 0};
	VECTOR cross = {0, 0, 0};
	VECTOR used_normal = {0, 0, 0};
	VECTOR fabs_norm = {0, 0, 0};
	int tDist = 0;
	int dominant_axis = 0;
	//Order the objects so the face always has the same normal
	
	//Extrapolate a quad out of the pix given
	//	0 - 1 // B - D
	//	3 - 2 // A - C
	fenceA[X] = -dWorldObjects[obj_id1].pos[X];
	fenceA[Y] = -dWorldObjects[obj_id1].pos[Y];
	fenceA[Z] = -dWorldObjects[obj_id1].pos[Z];
	
	fenceB[X] = fenceA[X];
	fenceB[Y] = fenceA[Y] - (dWorldObjects[obj_id1].type.radius[Y]<<16);
	fenceB[Z] = fenceA[Z];
	
	fenceC[X] = -dWorldObjects[obj_id2].pos[X];
	fenceC[Y] = -dWorldObjects[obj_id2].pos[Y];
	fenceC[Z] = -dWorldObjects[obj_id2].pos[Z];
	
	fenceD[X] = fenceC[X];
	fenceD[Y] = fenceC[Y] - (dWorldObjects[obj_id2].type.radius[Y]<<16);
	fenceD[Z] = fenceC[Z];

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
	
	normalize(cross, used_normal);

	//////////////////////////////////////////////////////////////
	// Grab the absolute normal used for finding the dominant axis
	//////////////////////////////////////////////////////////////
	fabs_norm[X] = JO_ABS(used_normal[X]);
	fabs_norm[Y] = JO_ABS(used_normal[Y]);
	fabs_norm[Z] = JO_ABS(used_normal[Z]);
	FIXED max_axis = JO_MAX(JO_MAX((fabs_norm[X]), (fabs_norm[Y])), (fabs_norm[Z]));
	dominant_axis = ((fabs_norm[X]) == max_axis) ? N_Xp : dominant_axis;
	dominant_axis = ((fabs_norm[Y]) == max_axis) ? N_Yp : dominant_axis;
	dominant_axis = ((fabs_norm[Z]) == max_axis) ? N_Zp : dominant_axis;
	//	0 - 1 // B - D
	//	3 - 2 // A - C
	//////////////////////////////////////////////////////////////
	// Collision Test Method: Chirality Check
	// This first tests, line by line, if the player is inside the shape on at least two axis.
	// Then the final axis is checked with a point-to-plane distance check.
	// The benefits of this is that the chirality check will exit early a lot of the time.
	//////////////////////////////////////////////////////////////
 	if(edge_wind_test(fenceA, fenceB, tgt->pos, dominant_axis) < 0)
	{
		if(edge_wind_test(fenceB, fenceD, tgt->pos, dominant_axis) < 0)
		{
			if(edge_wind_test(fenceD, fenceC, tgt->pos, dominant_axis) < 0)
			{
				if(edge_wind_test(fenceC, fenceA, tgt->pos, dominant_axis) < 0)
				{
					tDist = realpt_to_plane(you.pos, used_normal, fenceA);
					if(dWorldObjects[obj_id1].dist != 0 && (tDist ^ dWorldObjects[obj_id1].dist) < 0)
					{
						add_to_track_timer(obj_id1, obj_id2);
						//spr_sprintf(150,112, "bip");
					}
				}
			}
		}
	} 
	
	// int ab = edge_wind_test(fenceA, fenceB, tgt->pos, dominant_axis);
	// int bb = edge_wind_test(fenceB, fenceD, tgt->pos, dominant_axis);
	// int cb = edge_wind_test(fenceD, fenceC, tgt->pos, dominant_axis);
	// int db = edge_wind_test(fenceC, fenceA, tgt->pos, dominant_axis);

	// nbg_sprintf(0 + (obj_id1 * 6),6, "(%i)", ab>>12);
	// nbg_sprintf(0 + (obj_id1 * 6),7, "(%i)", bb>>12);
	// nbg_sprintf(0 + (obj_id1 * 6),8, "(%i)", cb>>12);
	// nbg_sprintf(0 + (obj_id1 * 6),9, "(%i)", db>>12);

	//nbg_sprintf(2,6 + obj_id1, "(%i)", tDist);
	dWorldObjects[obj_id1].dist = tDist;
}

/////////
//
// Something about gate posts is inefficient and slowing the program down.
// It's a time like this where profiler would come in super handy.
// Unfortunately, I'm going to have to scope the issue manually.
// Regardless there's room for  optimization, here or there.
//
//
/////////
void	test_gate_posts(int index, _boundBox * tgt)
{

			if((dWorldObjects[index].type.ext_dat & GATE_PASSED) != 0) return; //Return if the gate is already flagged as passed.
															
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
				if(!(dWorldObjects[index].more_data & GATE_POST_ALIGNED) && ((flagOne & 0xFF0) == (flagTwo & 0xFF0)))
				{
					align_object_to_object(index, trackedEntry);
					if(entities[dWorldObjects[index].type.entity_ID].type == MODEL_TYPE_BUILDING)
					{
						//Why do we flip this? Because coordinates are madness.
						dWorldObjects[index].rot[Y] = -dWorldObjects[index].rot[Y];
						generate_rotated_entity_for_object(index);
					}
					// nbg_sprintf(0, 10, "o1id(%i)", index);
					// nbg_sprintf(10, 10, "o2id(%i)", trackedEntry);
					// nbg_sprintf(3, 12, "data0(%x)", posDif[X]);
					// nbg_sprintf(5, 13, "rot0(%i)", dWorldObjects[index].rot[Y]);
					// nbg_sprintf(3, 14, "data1(%x)", posDif[Z]);
					// nbg_sprintf(5, 15, "rot1(%i)", dWorldObjects[trackedEntry].rot[Y]);
					
				}
			}
			trackedEntry = dWorldObjects[trackedEntry].link; //Retrieve the declared entity ID of the next gate post from linked list
			flagTwo = dWorldObjects[trackedEntry].type.ext_dat & 0x7FFF; //Get the data of the next post (but ignore the POP?)
			// Explanation: The POP of two pieces of a gate may not always match, but all of the other data should.
			// This includes: Is it checked yet, is it passed yet, is it the same track, is it the same set of linked posts.
		}
}


void	test_gate_ring(int index, _boundBox * tgt)
{
	//
	//Objective / Concept:
	// Test if player has passed through a ring,
	// based on a bound box object from a dWorldObjects entry.
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
		add_to_track_timer(activeObjects[index], -1);
		dWorldObjects[activeObjects[index]].type.ext_dat |= GATE_PASSED; //Flag gate as passed.
		}
	
	dWorldObjects[activeObjects[index]].dist = tDist;
}


void	item_by_type_processing(_declaredObject * item, unsigned short type)
{
	if(type == ITEM_TYPE_PTADR)
	{
		item->type.ext_dat |= ITEM_COLLECTED;
		pcm_play(snd_clack, PCM_SEMI, 6);
		you.points += 1;
	}
	if(type == ITEM_TYPE_RING1)
	{
		item->type.ext_dat |= ITEM_COLLECTED;
		add_position_to_minimap(item->pix[X], item->pix[Y], 0xB3E0);
		pcm_play(snd_ring1, PCM_SEMI, 6);
		you.points += 2;
		start_hud_event(RING1_EVENT);
	}
	if(type == ITEM_TYPE_RING2)
	{
		item->type.ext_dat |= ITEM_COLLECTED;
		add_position_to_minimap(item->pix[X], item->pix[Y], 0xE7E0);
		pcm_play(snd_ring2, PCM_SEMI, 6);
		you.points += 4;
		start_hud_event(RING2_EVENT);
	}
	if(type == ITEM_TYPE_RING3)
	{
		item->type.ext_dat |= ITEM_COLLECTED;
		add_position_to_minimap(item->pix[X], item->pix[Y], 0xCC0C);
		pcm_play(snd_ring3, PCM_SEMI, 6);
		you.points += 8;
		start_hud_event(RING3_EVENT);
	}
	if(type == ITEM_TYPE_RING4)
	{
		item->type.ext_dat |= ITEM_COLLECTED;
		add_position_to_minimap(item->pix[X], item->pix[Y], 0x819F);
		pcm_play(snd_ring4, PCM_SEMI, 6);
		you.points += 16;
		start_hud_event(RING4_EVENT);
	}
	if(type == ITEM_TYPE_RING5)
	{
		item->type.ext_dat |= ITEM_COLLECTED;
		add_position_to_minimap(item->pix[X], item->pix[Y], 0xFFEC);
		pcm_play(snd_ring5, PCM_SEMI, 6);
		you.points += 32;
		start_hud_event(RING5_EVENT);
	}
	if(type == ITEM_TYPE_RING6)
	{
		item->type.ext_dat |= ITEM_COLLECTED;
		add_position_to_minimap(item->pix[X], item->pix[Y], 0x83F3);
		pcm_play(snd_ring6, PCM_SEMI, 6);
		you.points += 64;
		start_hud_event(RING6_EVENT);
	}
	if(type == ITEM_TYPE_RING7)
	{
		item->type.ext_dat |= ITEM_COLLECTED;
		add_position_to_minimap(item->pix[X], item->pix[Y], 0x801F);
		pcm_play(snd_ring7, PCM_SEMI, 6);
		you.points += 128;
		start_hud_event(RING7_EVENT);
	}
	if(type == ITEM_TYPE_FLAG)
	{
		//Multiple things that could happen with a flag.
		if(item->type.ext_dat & FLAG_OPEN && !(item->type.ext_dat & FLAG_RETURN))
		{
			item->type.ext_dat |= FLAG_GRABBED;
			item->more_data |= ITEM_MDATA_SNAP_COLLISION;
		} 

		if(item->type.ext_dat & FLAG_GRABBED && !(item->type.ext_dat & FLAG_RETURN))
		{
			item->pos[X] = -pl_RBB.pos[X];
			item->pos[Y] = -pl_RBB.pos[Y] - (item->type.radius[Y]<<15);
			item->pos[Z] = -pl_RBB.pos[Z];
			item->rot[X] = pl_RBB.boxRot[X];
			item->rot[Y] = pl_RBB.boxRot[Y];
			item->rot[Z] = pl_RBB.boxRot[Z];
			item->pix[X] = -you.cellPos[X];
			item->pix[Y] = -you.cellPos[Y];
		}
		
	}
	
}


//Function will check collision with a ITEM-type object and flag entities that have been collided with for removal.
//It will play a sound, and add to your points too.
void	item_collision(int index, _boundBox * tgt)
{

	static int rel_pos[XYZ];
	if(dWorldObjects[activeObjects[index]].type.ext_dat & ITEM_COLLECTED) return;
	
	rel_pos[X] = JO_ABS(tgt->pos[X] + RBBs[index].pos[X])>>16;
	rel_pos[Y] = JO_ABS(tgt->pos[Y] + RBBs[index].pos[Y])>>16;
	rel_pos[Z] = JO_ABS(tgt->pos[Z] + RBBs[index].pos[Z])>>16;
	
	dWorldObjects[activeObjects[index]].dist = slSquart( (rel_pos[X] * rel_pos[X]) + (rel_pos[Y] * rel_pos[Y]) + (rel_pos[Z] * rel_pos[Z]) );
	
	if(dWorldObjects[activeObjects[index]].dist < dWorldObjects[activeObjects[index]].type.radius[X]
	|| dWorldObjects[activeObjects[index]].more_data & ITEM_MDATA_SNAP_COLLISION)
	{
		item_by_type_processing(&dWorldObjects[activeObjects[index]], GET_ITEM_TYPE(dWorldObjects[activeObjects[index]].type.ext_dat));
	}
	
}

void	subtype_collision_logic(_declaredObject * someOBJECTdata, _boundBox * stator, _boundBox * mover)
{
	if(stator->collisionID != mover->boxID) return; //If these objects aren't colliding with each other, exit.
	unsigned short otype = GET_OBJECT_TYPE(someOBJECTdata->type.ext_dat);
	if(otype == FORCEFIELD_TOUCH && !(someOBJECTdata->type.ext_dat & FF_TIMER_STARTED))
	{
		someOBJECTdata->dist += 32768; //Add to the timer.
		someOBJECTdata->type.ext_dat |= FF_TIMER_STARTED;
		pcm_play(snd_ffield1, PCM_PROTECTED, 6);
	} else if(otype == CRUSH_BLOCK_SLOW)
	{
		if(you.sanics > (4<<16))
		{
		someOBJECTdata->type.ext_dat |= OBJECT_DISABLED;
		pcm_play(snd_khit, PCM_PROTECTED, 6);
		}
	}
	
}

void	track_data_manage_rings(_declaredObject * someLDATA, _declaredObject * someRINGdata,
		unsigned short * discovery, short ldata_track)
{
			while(someRINGdata != &dWorldObjects[objNEW])
			{
				//nbg_sprintf(0, 0, "(RING)"); //Debug ONLY
				short object_track = (someRINGdata->type.ext_dat & 0xF00)>>8; //Get object track to see if it matches the level data track
					if(ldata_track == object_track)
					{
						int gNum = someRINGdata->type.ext_dat & GATE_LINK_NUMBER;
						int gGuide = (someLDATA->more_data & TRACK_GUIDE_NUMBER)>>8;
						// Track Discovery Checking
						if(!(someLDATA->type.ext_dat & TRACK_DISCOVERED))
						{
							*discovery &= someRINGdata->more_data;
							if(someRINGdata->more_data) someLDATA->pix[X]++;
						}
						//This is a counter that adds up the total # of gates in a track.
						someLDATA->pix[Y]++;
						//Reset if track reset enabled
						if(someLDATA->type.ext_dat & TRACK_RESET)
						{
							add_position_to_minimap(someRINGdata->pix[X], someRINGdata->pix[Y], 0x83E0);
							someRINGdata->type.ext_dat &= GATE_UNPASSED; 
						} else if(someRINGdata->type.ext_dat & GATE_PASSED)
						{
							someLDATA->type.ext_dat |= TRACK_ACTIVE; 
							someLDATA->pix[X]++;
						} else if(someLDATA->type.ext_dat & TRACK_ACTIVE && (gNum>>4) == gGuide)
						{
							add_to_sprite_list(someRINGdata->pos, someRINGdata->pix, flagIconTexno /*texno*/, 2<<6 /*colorbank*/, 0 /*mesh Bool*/, SPRITE_TYPE_UNSCALED_BILLBOARD, 0 /*no clip*/, 3000);
						}

						someLDATA->more_data &= TRACK_NO_CHECK;
						someLDATA->more_data |= (someRINGdata->type.ext_dat & GATE_LINK_NUMBER);
					}
			someRINGdata = step_linked_object_list(someRINGdata);
			}
}

void	track_data_manage_posts(_declaredObject * someLDATA, _declaredObject * somePOSTdata,
		unsigned short * discovery, short ldata_track)
{
			while(somePOSTdata != &dWorldObjects[objNEW])
			{
				//nbg_sprintf(0, 0, "(POST)"); //Debug ONLY
				short object_track = (somePOSTdata->type.ext_dat & 0xF00)>>8; //Get object track to see if it matches the level data track
				////////////////////////////////////////////////////
				// Flush the "checked collision yet" marker for gate posts.
				somePOSTdata->type.ext_dat &= GATE_UNCHECKED;
					if(ldata_track == object_track)
					{
						int gNum = somePOSTdata->type.ext_dat & GATE_LINK_NUMBER;
						int gGuide = (someLDATA->more_data & TRACK_GUIDE_NUMBER)>>8;
						int lastCheck = someLDATA->more_data & TRACK_LAST_CHECKED;
						// Track Discovery Checking
						if(!(someLDATA->type.ext_dat & TRACK_DISCOVERED))
						{
							*discovery &= somePOSTdata->more_data;
							if(somePOSTdata->more_data & TRACK_DISCOVERED && lastCheck != gNum) someLDATA->pix[X]++;

						}
						//The "Y" pix of a track data level data is the total number of gates in the track.
						//To complete the track, X must equal Y.
						if(lastCheck != gNum || someLDATA->pix[Y] == 0)
						{
							someLDATA->pix[Y]++;
						}
						//Reset if track reset enabled
						if(someLDATA->type.ext_dat & TRACK_RESET)
						{
							add_position_to_minimap(somePOSTdata->pix[X], somePOSTdata->pix[Y], 0x83E0);
							somePOSTdata->type.ext_dat &= GATE_UNPASSED; 
						} else if(somePOSTdata->type.ext_dat & GATE_PASSED && lastCheck != gNum)
						{
							someLDATA->type.ext_dat |= TRACK_ACTIVE; 
							someLDATA->pix[X]++;
							if(gGuide == (gNum>>4))
							{
								someLDATA->more_data &= TRACK_NO_GUIDE;
								gGuide = (gGuide >= 15) ? 0 : gGuide+1;
								someLDATA->more_data |= (gGuide)<<8;
							}
						} else if(someLDATA->type.ext_dat & TRACK_ACTIVE && (gNum>>4) == gGuide)
						{
							add_to_sprite_list(somePOSTdata->pos, somePOSTdata->pix, flagIconTexno /*texno*/, 2<<6 /*colorbank*/, 0 /*mesh Bool*/, SPRITE_TYPE_UNSCALED_BILLBOARD, 0 /*no clip*/, 3000);
						}
						someLDATA->more_data &= TRACK_NO_CHECK;
						someLDATA->more_data |= (somePOSTdata->type.ext_dat & GATE_LINK_NUMBER);
					}
			somePOSTdata = step_linked_object_list(somePOSTdata);
			}
}

void	manage_track_data(_declaredObject * someLDATA)
{
	
	short ldata_track = 0;
	static unsigned short discovery;
	discovery = 0;
	////////////////////////////////////////////////////////////////////////////////
	//
	// Level data, track data manager section
	// It's messy.
	//
	////////////////////////////////////////////////////////////////////////////////
	discovery |= TRACK_DISCOVERED; // Flag the track as discovered. This is used for checking the tracks discovery later.
	ldata_track = someLDATA->type.entity_ID; //Get the level data's track #
	someLDATA->pix[X] = 0; //Re-set the passed/to-pass counters (pix x and pix y) of the track level data.
	someLDATA->pix[Y] = 0; //We do this every time because we count them up every time.
	_declaredObject * somePOSTdata = get_first_in_object_list(GATE_P);
	_declaredObject * someRINGdata = get_first_in_object_list(GATE_R);
	//nbg_sprintf(1, 13, "track(%i)", ldata_track);
	if(!(someLDATA->type.ext_dat & TRACK_COMPLETE)) // if track isn't complete
	{
				// nbg_sprintf(0, 17, "ldt(%i)", trackedLDATA);
				// nbg_sprintf(0, 17, "ldt(%i)", someLDATA->more_data);
		track_data_manage_rings(someLDATA, someRINGdata, &discovery, ldata_track);
		track_data_manage_posts(someLDATA, somePOSTdata, &discovery, ldata_track);
	
		someLDATA->type.ext_dat &= TRACK_CLEAR_RESET;
			
		//Timer run & check
		if((someLDATA->type.ext_dat & TRACK_ACTIVE))
		{
			someLDATA->dist -= delta_time;
			slPrintFX(someLDATA->dist, slLocate(0, 7));
				if(someLDATA->dist < 0) //If timer expired...
				{
					someLDATA->type.ext_dat &= TRACK_INACTIVE;
					someLDATA->type.ext_dat |= TRACK_RESET; //Reset tracks; timer expired
					someLDATA->dist = 0;
					//Sound stuff
					pcm_play(snd_alarm, PCM_PROTECTED, 5);
					//Clear screen in this zone
			slPrint("                           ", slLocate(0, 6));
			slPrint("                           ", slLocate(0, 7));
				}
				
			//Drawing Gate Guide
			int gateToGuideTo = (someLDATA->more_data & TRACK_GUIDE_NUMBER)>>8;
			if(gateToGuideTo >= someLDATA->pix[Y])
			{
				//In case the gate-guide number is higher than the number of gates in the track ...
				//Set the gate-to-guide to to zero.
				someLDATA->more_data &= TRACK_NO_GUIDE;
			}
				
		}
			
		//Track completion logic
		if(someLDATA->pix[X] == someLDATA->pix[Y] && someLDATA->pix[Y] != 0 && someLDATA->type.ext_dat & TRACK_ACTIVE)
		{
			someLDATA->type.ext_dat &= TRACK_INACTIVE;	//Set track as inactive
			someLDATA->type.ext_dat |= TRACK_COMPLETE;	//Set track as complete
			you.points += 10 * someLDATA->pix[X];
			//pcm_play(snd_win, PCM_PROTECTED, 5);
			start_adx_stream(stmsnd[stm_win], 5);
			slPrint("                           ", slLocate(0, 6));
			slPrint("                           ", slLocate(0, 7));
		}

		//Discovery count-down & track discovery
		if(discovery & TRACK_DISCOVERED && !(someLDATA->type.ext_dat & TRACK_DISCOVERED))
		{
			someLDATA->dist -= delta_time;
			if(someLDATA->dist < 0 && someLDATA->pix[Y] != 0)
			{
				pcm_play(snd_cronch, PCM_PROTECTED, 5); //Sound
				someLDATA->type.ext_dat |= TRACK_DISCOVERED;
				someLDATA->pix[X] = 0;
				someLDATA->dist = 0;
			}
		} else if(!(someLDATA->type.ext_dat & TRACK_DISCOVERED))
		{
			someLDATA->dist = 2<<16; //Set track discovery timer
		}
	}

	_declaredObject * someOBJECTdata = get_first_in_object_list(OBJECT);
	while(someOBJECTdata != &dWorldObjects[objNEW] && someLDATA->type.ext_dat & TRACK_RESET)
	{
		//If the track data's series and object data's series match, reset the object.
		if(ldata_track == someOBJECTdata->type.clone_ID)
		{
			someOBJECTdata->type.ext_dat &= OBJECT_RESET;
		}
		
		someOBJECTdata = step_linked_object_list(someOBJECTdata);
	}
}

void	run_an_item_manager(_declaredObject * someLDATA)
{
	//aight so wtf am i doing lol
	_declaredObject * someITEMdata = get_first_in_object_list(ITEM);
	_declaredObject * someOBJECTdata = get_first_in_object_list(OBJECT);
	
	unsigned short manager_series = someLDATA->type.entity_ID;
	unsigned short item_series;
	unsigned short item_type;
	unsigned short manager_type = someLDATA->type.ext_dat & ITEM_CONDITION_TYPES;
	unsigned short * edata;
	//Zero out some counters
	someLDATA->rot[X] = 0;
	someLDATA->rot[Y] = 0;
	
	while(someITEMdata != &dWorldObjects[objNEW])
	{
		edata = &someITEMdata->type.ext_dat;
		item_series = someITEMdata->type.clone_ID;
		item_type = GET_ITEM_TYPE(*edata);
		if(item_series == manager_series)
		{
			someLDATA->rot[X]++;
			if(*edata & ITEM_COLLECTED)
			{
				someLDATA->rot[Y]++;
				if(manager_type == MANAGER_7RINGS)
				{
					someLDATA->more_data |= (1<<((item_type>>4)-1));
				}
			}
			if(manager_type == MANAGER_7RINGS)
			{
				//Make the rings spin
				someITEMdata->rot[Y] += 182;
			}
			
			if(manager_type == MANAGER_CTF && item_type == ITEM_TYPE_FLAG)
			{
				if(someLDATA->type.ext_dat & CTF_FLAG_OPEN)
				{
					*edata |= FLAG_OPEN;
				}
				if(someLDATA->type.ext_dat & CTF_FLAG_TAKEN)
				{
					someITEMdata->dist = approximate_distance(someLDATA->pos, someITEMdata->pos)>>16;
					 if(someITEMdata->dist < someLDATA->type.radius[X])
					 {
						 *edata |= ITEM_COLLECTED;
						 someITEMdata->more_data = 0;
						 someLDATA->type.ext_dat |= CTF_FLAG_CAPTURED;
					 }
					 if(someLDATA->dist < 0)
					 {
						*edata |= FLAG_RETURN;
					 }
				}
				if(!(someLDATA->type.ext_dat & CTF_FLAG_TAKEN) && *edata & FLAG_GRABBED)
				{
					//Start a timer. The clone ID of the CTF manager is used as the seconds count.
					someLDATA->dist += someLDATA->type.clone_ID<<16;
					someLDATA->type.ext_dat |= CTF_FLAG_TAKEN;
					pcm_play(snd_ftake, PCM_PROTECTED, 5);
				}
			}
			
			if(manager_type == MANAGER_RETURN_PT)
			{
				if(item_type == ITEM_TYPE_FLAG && *edata & FLAG_RETURN)
				{
					*edata &= ITEM_NO_FLAGS;
					someITEMdata->pix[X] = someLDATA->pix[X];
					someITEMdata->pix[Y] = someLDATA->pix[Y];
					someITEMdata->pos[X] = someLDATA->pos[X];
					someITEMdata->pos[Y] = someLDATA->pos[Y];
					someITEMdata->pos[Z] = someLDATA->pos[Z];
					someITEMdata->rot[X] = someLDATA->rot[X];
					someITEMdata->rot[Y] = someLDATA->rot[Y];
					someITEMdata->rot[Z] = someLDATA->rot[Z];
					someITEMdata->more_data = 0;
					//pcm_play(snd_freturn, PCM_PROTECTED, 6);
					start_adx_stream(stmsnd[stm_freturn], 5);
				}
			}
		
		}
		
		someITEMdata = step_linked_object_list(someITEMdata);
	}
	
	while(someOBJECTdata != &dWorldObjects[objNEW])
	{
		edata = &someOBJECTdata->type.ext_dat;
		item_series = someOBJECTdata->type.clone_ID;
		item_type = GET_OBJECT_TYPE(someOBJECTdata->type.ext_dat);
		
		if(item_series == manager_series)
		{
			someLDATA->rot[X]++;
			if(manager_type == MANAGER_CTF && item_type == FORCEFIELD_REMOTE
			&& someLDATA->type.ext_dat & CTF_FLAG_OPEN && !(*edata & OBJECT_DISABLED))
			{
				*edata |= OBJECT_DISABLED;
				add_position_to_minimap(someOBJECTdata->pix[X], someOBJECTdata->pix[Y], 0xFFEC);
				add_position_to_minimap((someLDATA->pos[X]>>16) / CELL_SIZE_INT, (someLDATA->pos[Z]>>16) / CELL_SIZE_INT, 0xFFEC);
			}
		}
		
		someOBJECTdata = step_linked_object_list(someOBJECTdata);
	}
	
	
	
	
	if(manager_type == MANAGER_COLLECT_ALL)
	{
		if(someLDATA->rot[X] == someLDATA->rot[Y])
		{
			someLDATA->type.ext_dat |= ITEM_MANAGER_INACTIVE;
			someLDATA->type.ext_dat |= COLLECT_ALL_COMPLETE;
			pcm_play(snd_cronch, PCM_PROTECTED, 6);
		}
	} else if(manager_type == MANAGER_7RINGS)
	{
		if((someLDATA->more_data & 0x7F) == 0x7F)
		{
			someLDATA->type.ext_dat |= ITEM_MANAGER_INACTIVE;
			someLDATA->type.ext_dat |= COLLECT_ALL_COMPLETE;
			//pcm_play(snd_alarm, PCM_PROTECTED, 6);
			start_hud_event(RINGS_ALL_EVENT);
		}
	} else if(manager_type == MANAGER_CTF)
	{
		if(!(someLDATA->type.ext_dat & CTF_FLAG_OPEN))
		{
			//Not a great solution since it can't abstract to a different target.
			//But right now that doesn't matter.
			someLDATA->more_data = approximate_distance(someLDATA->pos, you.wpos)>>16;
			if(someLDATA->more_data < someLDATA->type.radius[X])
			{
			someLDATA->type.ext_dat |= CTF_FLAG_OPEN;
			pcm_play(snd_ffield1, PCM_PROTECTED, 6);
			}
		}
		if(someLDATA->type.ext_dat & CTF_FLAG_CAPTURED)
		{
			someLDATA->type.ext_dat |= ITEM_MANAGER_INACTIVE;
			start_adx_stream(stmsnd[stm_win], 5);
		}
		if(someLDATA->type.ext_dat & CTF_FLAG_TAKEN)
		{
			add_to_sprite_list(someLDATA->pos, someLDATA->pix, flagIconTexno /*texno*/, 2<<6 /*colorbank*/, 0 /*mesh Bool*/, SPRITE_TYPE_UNSCALED_BILLBOARD, 0 /*no clip*/, 3000);
			if(someLDATA->dist < 0)
			{
				someLDATA->type.ext_dat &= CLEAR_MANAGER_FLAGS;
				someLDATA->type.ext_dat |= CTF_FLAG_OPEN;
			}
			someLDATA->dist -= delta_time;
		}
	}
	
}

void	manage_object_data(void)
{
	//Objects are particular.
	//They can be part of item managers, or not.
	//This only manages the type of object data that are not managed by other means.
	//Items should also have a loop like this eventually, but not yet.
	
	_declaredObject * someOBJECTdata = get_first_in_object_list(OBJECT);
	
	while(someOBJECTdata != &dWorldObjects[objNEW])
	{
		unsigned short * edata = &someOBJECTdata->type.ext_dat;
		
		if(GET_OBJECT_TYPE(*edata) == FORCEFIELD_TOUCH)
		{
			if(*edata & FF_TIMER_STARTED)
			{
				if(someOBJECTdata->dist < 0)
				{
					*edata |= OBJECT_DISABLED;
					*edata |= FF_RESET_STARTED;
					someOBJECTdata->dist += 2<<16;
				}
				someOBJECTdata->dist -= delta_time;
			}
			if(*edata & FF_RESET_STARTED)
			{
				if(someOBJECTdata->dist < 0)
				{
					*edata &= OBJECT_RESET;
					someOBJECTdata->dist = 0;
					pcm_play(snd_ffield2, PCM_PROTECTED, 6);
				}
				someOBJECTdata->dist -= delta_time;
			}
		}
		
		someOBJECTdata = step_linked_object_list(someOBJECTdata);
	}
	
}


void	ldata_manager(void)
{
	_declaredObject * someLDATA = get_first_in_object_list(LDATA);
	
	//More to do:
	//Minimum speed reset [maybe not for now]
	//What is this though?
	//A control loop for a specific level data type, as "track data". A track is a series of rings or gates that make a... track.
	//This keeps an eye on what's going on in each track.
	//The collision math is ran separately, in a more key spot in the entire physics structure.
	//This is a purpose-built function, but can be viewed as a method of game state tracking through linked lists.
	
	// nbg_sprintf(0, 15, "tim(%i)", (dWorldObjects[activeTrack].type.ext_dat & 0xF)<<17);
	// nbg_sprintf(0, 16, "act(%i)", activeTrack);

	
	while(someLDATA != &dWorldObjects[objNEW])
	{
				//nbg_sprintf(0, 0, "(GTMN)"); //Debug ONLY
		if( (someLDATA->type.ext_dat & LDATA_TYPE) == TRACK_DATA && !(someLDATA->more_data & TRACK_COMPLETE))
		{
			manage_track_data(someLDATA);
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
		} else if((someLDATA->type.ext_dat & LDATA_TYPE) == ITEM_MANAGER && !(someLDATA->type.ext_dat & ITEM_MANAGER_INACTIVE))
		{
			run_an_item_manager(someLDATA);
		}
		someLDATA = step_linked_object_list(someLDATA);
	}//while LDATA end
			
	manage_object_data();
	//slPrintHex(someLDATA->type.ext_dat, slLocate(13, 12));
	//nbg_sprintf(13, 12, "ac_trk(%i)", activeTrack);
			
	// slPrintHex(dWorldObjects[5].dist, slLocate(0, 15));
	// slPrintHex(dWorldObjects[6].dist, slLocate(0, 16));
				// slPrintFX(trackTimers[0], slLocate(0, 5));
				// slPrintFX(trackTimers[1], slLocate(0, 6));
	// slPrintHex(dWorldObjects[5].pix[X], slLocate(0, 7));
	// slPrintHex(dWorldObjects[5].pix[Y], slLocate(0, 8));
	// slPrintHex(dWorldObjects[6].pix[X], slLocate(13, 7));
	// slPrintHex(dWorldObjects[6].pix[Y], slLocate(13, 8));
	
}


