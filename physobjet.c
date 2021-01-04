#include "physobjet.h"
#include "object_definitions.c"


_declaredObject * dWorldObjects; //In LWRAM - see lwram.c
_buildingObject * BuildingPayload;

unsigned short objNEW = 0;
unsigned short objDRAW[MAX_WOBJS];
unsigned short activeObjects[MAX_WOBJS];
short link_starts[8] = {-1, -1, -1, -1,
						-1, -1, -1, -1};
int trackTimers[16];
int activeTrack = -1;
int objUP = 0;
int total_building_payload = 0;

void	declare_object_at_cell(short pixX, short height, short pixY, int type, ANGLE xrot, ANGLE yrot, ANGLE zrot)
{
		if(objNEW < MAX_WOBJS)
		{
	dWorldObjects[objNEW].pos[X] = -(pixX * CELL_SIZE_INT)<<16;
	dWorldObjects[objNEW].pos[Z] = -(pixY * CELL_SIZE_INT)<<16;
	dWorldObjects[objNEW].pos[Y] = height<<16; //Vertical offset from ground
	dWorldObjects[objNEW].pix[X] = -(pixX);
	dWorldObjects[objNEW].pix[Y] = -(pixY);
	dWorldObjects[objNEW].type = *objList[type];
	dWorldObjects[objNEW].srot[X] = (xrot * 182); // deg * 182 = angle
	dWorldObjects[objNEW].srot[Y] = (yrot * 182);
	dWorldObjects[objNEW].srot[Z] = (zrot * 182);
	dWorldObjects[objNEW].link = link_starts[(dWorldObjects[objNEW].type.ext_dat & 0x7000)>>12]; //Set object's link to the current link of this type
	link_starts[(dWorldObjects[objNEW].type.ext_dat & 0x7000)>>12] = objNEW; //Set the current link of this type to this entry
	dWorldObjects[objNEW].more_data = 0;
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
	dWorldObjects[objNEW].srot[X] = 0;
	dWorldObjects[objNEW].srot[Y] = 0;
	dWorldObjects[objNEW].srot[Z] = 0;
	dWorldObjects[objNEW].more_data = 0;
	dWorldObjects[objNEW].link = link_starts[(dWorldObjects[objNEW].type.ext_dat & 0x7000)>>12]; //Set object's link to the current link of this type
	link_starts[(dWorldObjects[objNEW].type.ext_dat & 0x7000)>>12] = objNEW; //Set the current link of this type to this entry
	objNEW++;
		}
}

void	declarations(void)
{
	declare_object_at_cell(0, 0, 0, 62 /* Track Data */, 0, 0, 0);
/* 	for(int k = 0; k < 8; k++)
	{
		link_starts[k] = -1; //Re-set link starts to no links conidition
	} */

/* 	for(int i = 0; i < objNEW; i++)
	{
		dWorldObjects[i].link = link_starts[(dWorldObjects[i].type.ext_dat & 0x7000)>>12]; //Set object's link to the current link of this type
		link_starts[(dWorldObjects[i].type.ext_dat & 0x7000)>>12] = i; //Set the current link of this type to this entry
	} */
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
	objUP = 0;
	
	unsigned short * used_radius;

//Notice: Maximum collision tested & rendered items is MAX_PHYS_PROXY
	for(int i = 0; i < objNEW; i++){
		
		//jo_printf(0, 0, "(CTRL)"); //Debug ONLY
		
		difX = fxm(((ppos[X] * CELL_SIZE) + dWorldObjects[i].pos[X]), INV_CELL_SIZE)>>16; 
		difY = fxm(((ppos[Y] * CELL_SIZE) + dWorldObjects[i].pos[Z]), INV_CELL_SIZE)>>16; 
				////////////////////////////////////////////////////
				//Flush specific data for gate posts. Don't remember why.
				////////////////////////////////////////////////////
		dWorldObjects[i].type.ext_dat &= ((dWorldObjects[i].type.ext_dat & OTYPE) == GATE_P) ? 0xFFFD : 0xFFFF;
				////////////////////////////////////////////////////
		
		if((dWorldObjects[i].type.ext_dat & OTYPE) == LDATA)
		{ 		
				////////////////////////////////////////////////////
				//If the object type declared is LDATA (level data), use a different logic branch.
				////////////////////////////////////////////////////
				if(difX > -14 && difX < 14 && difY > -14 && difY < 14 &&
				(dWorldObjects[i].type.ext_dat & LDATA_TYPE) == LEVEL_CHNG)
				{
					// We've found a level change trigger close to the player.
					// If we are close enough to the level change trigger and it is enabled, change levels.
					int pos_difs[3] = {
										(you.pos[X] + dWorldObjects[i].pos[X]),
										you.pos[Y] + (dWorldObjects[i].pos[Y] - (main_map[
(-dWorldObjects[i].pix[X] + (main_map_x_pix * dWorldObjects[i].pix[Y]) + (main_map_total_pix>>1))
																			]<<16)),
										(you.pos[Z] + dWorldObjects[i].pos[Z])
										};	
					if(JO_ABS(pos_difs[X]) < (dWorldObjects[i].type.radius[X]<<16)
					&& JO_ABS(pos_difs[Y]) < (dWorldObjects[i].type.radius[Y]<<16)
					&& JO_ABS(pos_difs[Z]) < (dWorldObjects[i].type.radius[Z]<<16)
					//Enabling Booleans
					&& !(dWorldObjects[i].type.ext_dat & OBJPOP) && (dWorldObjects[i].type.ext_dat & 0x80))
					{
						//////////////////////////////////////////
						// Temporary, but will change levels.
						//////////////////////////////////////////
						dWorldObjects[i].type.ext_dat |= OBJPOP;
						pcm_play(snd_win, PCM_PROTECTED, 7);
						map_chg = false;
						p64MapRequest(dWorldObjects[i].type.entity_ID);
						///////////////////////////////////////////
						// More temporary stuff.
						///////////////////////////////////////////
						you.points = 0;
						//declare_object_at_cell(0, 0, 0, 62 /* Track Data */, 0, 0, 0);
					}
				}
		} else if(difX > -14 && difX < 14 && difY > -14 && difY < 14 && objUP < MAX_PHYS_PROXY)
			{
				////////////////////////////////////////////////////
				// If no radius was defined for the object, use the radius from the entity.
				// Must check if the entity is loaded, or else out of bounds access may occur.
				////////////////////////////////////////////////////
				if(dWorldObjects[i].type.radius[X] == 0 &&
					dWorldObjects[i].type.radius[Y] == 0 &&
					dWorldObjects[i].type.radius[Z] == 0
					&& entities[dWorldObjects[i].type.entity_ID].file_done == true)
				{
				used_radius = entities[dWorldObjects[i].type.entity_ID].radius;
				} else {
				used_radius = dWorldObjects[i].type.radius;
				}
					if((dWorldObjects[i].type.ext_dat & OTYPE) != BUILD)
					{
						
							////////////////////////////////////////////////////
							//If a non-building object was in rendering range, specify it as being populated, 
							//and give it matrix/bounding box parameters.
							////////////////////////////////////////////////////
						bound_box_starter.modified_box = &RBBs[objUP];
						bound_box_starter.x_location = dWorldObjects[i].pos[X];
						//Y location has to find the value of a pixel of the map and add it with object height off ground + Y radius
						bound_box_starter.y_location = dWorldObjects[i].pos[Y] - ((used_radius[Y])<<16)
						- (main_map[
						(-dWorldObjects[i].pix[X] + (main_map_x_pix * dWorldObjects[i].pix[Y]) + (main_map_total_pix>>1)) 
						]<<16);
						//
						bound_box_starter.z_location = dWorldObjects[i].pos[Z];
						bound_box_starter.x_rotation = dWorldObjects[i].srot[X];
						bound_box_starter.y_rotation = dWorldObjects[i].srot[Y];
						bound_box_starter.z_rotation = dWorldObjects[i].srot[Z];

						bound_box_starter.x_radius = used_radius[X]<<16;
						bound_box_starter.y_radius = used_radius[Y]<<16;
						bound_box_starter.z_radius = used_radius[Z]<<16;
								
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
						bound_box_starter.y_location = dWorldObjects[i].pos[Y] - ((used_radius[Y])<<16)
						- (main_map[
						(-dWorldObjects[i].pix[X] + (main_map_x_pix * dWorldObjects[i].pix[Y]) + (main_map_total_pix>>1)) 
						]<<16);
						//
						bound_box_starter.z_location = dWorldObjects[i].pos[Z];
						bound_box_starter.x_rotation = 0;
						bound_box_starter.y_rotation = 0;
						bound_box_starter.z_rotation = 0;
						
						bound_box_starter.x_radius = used_radius[X]<<16;
						bound_box_starter.y_radius = used_radius[Y]<<16;
						bound_box_starter.z_radius = used_radius[Z]<<16;
								
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
		} else if(difX > -18 && difX < 18 && difY > -18 && difY < 18 && objUP < MAX_PHYS_PROXY)
			{
				////////////////////////////////////////////////////
				// If no radius was defined for the object, use the radius from the entity.
				// Must check if the entity is loaded, or else out of bounds access may occur.
				////////////////////////////////////////////////////
				if(dWorldObjects[i].type.radius[X] == 0 &&
					dWorldObjects[i].type.radius[Y] == 0 &&
					dWorldObjects[i].type.radius[Z] == 0
					&& entities[dWorldObjects[i].type.entity_ID].file_done == true)
				{
				used_radius = entities[dWorldObjects[i].type.entity_ID].radius;
				} else {
				used_radius = dWorldObjects[i].type.radius;
				}
					if((dWorldObjects[i].type.ext_dat & OTYPE) != BUILD && dWorldObjects[i].type.light_bright != 0)
					{
					////////////////////////////////////////////////////
					//If a non-building light-emitting object is in this larger range, add its light data to the light list.
					//But do not flag it to render or be collision-tested.
					////////////////////////////////////////////////////
						bound_box_starter.modified_box = &RBBs[objUP];
						bound_box_starter.x_location = dWorldObjects[i].pos[X];
						//Y location has to find the value of a pixel of the map and add it with object height off ground + Y radius
						bound_box_starter.y_location = dWorldObjects[i].pos[Y] - ((used_radius[Y])<<16)
						- (main_map[
						(-dWorldObjects[i].pix[X] + (main_map_x_pix * dWorldObjects[i].pix[Y]) + (main_map_total_pix>>1)) 
						]<<16);
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
					} else if((dWorldObjects[i].type.ext_dat & OTYPE) == BUILD)
					{
							////////////////////////////////////////////////////
							// Generate valid matrix parameters for the building.
							////////////////////////////////////////////////////
						bound_box_starter.modified_box = &RBBs[objUP];
						bound_box_starter.x_location = dWorldObjects[i].pos[X];
						//Y location has to find the value of a pixel of the map and add it with object height off ground + Y radius
						bound_box_starter.y_location = dWorldObjects[i].pos[Y] - ((used_radius[Y])<<16)
						- (main_map[
						(-dWorldObjects[i].pix[X] + (main_map_x_pix * dWorldObjects[i].pix[Y]) + (main_map_total_pix>>1)) 
						]<<16);
						//
						bound_box_starter.z_location = dWorldObjects[i].pos[Z];
						bound_box_starter.x_rotation = 0;
						bound_box_starter.y_rotation = 0;
						bound_box_starter.z_rotation = 0;
						
						bound_box_starter.x_radius = used_radius[X]<<16;
						bound_box_starter.y_radius = used_radius[Y]<<16;
						bound_box_starter.z_radius = used_radius[Z]<<16;
								
						make2AxisBox(&bound_box_starter);

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
		
	//jo_printf(18, 6, "objUP:(%i)", objUP);
	//jo_printf(18, 7, "objNW:(%i)", objNEW);
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
		if(RBBs[i].status[2] == 'L' && lights_created < MAX_DYNAMIC_LIGHTS)
			{
				active_lights[lights_created].pop = 1;
				active_lights[lights_created].ambient_light = active_lights[0].ambient_light;
				active_lights[lights_created].bright = dWorldObjects[activeObjects[i]].type.light_bright;
				active_lights[lights_created].pos[X] = -RBBs[i].pos[X];
				active_lights[lights_created].pos[Y] = -(RBBs[i].pos[Y] + dWorldObjects[activeObjects[i]].type.light_y_offset);
				active_lights[lights_created].pos[Z] = -RBBs[i].pos[Z];
				lights_created++;
				//slPrintFX(0, slLocate(2, 6+i));
				// slPrintFX(active_lights[i].pos[X], slLocate(2, 7));
				// slPrintFX(active_lights[i].pos[Y], slLocate(2, 8));
				// slPrintFX(active_lights[i].pos[Z], slLocate(2, 9));
			}
	}
	
	// jo_printf(2, 10, "(%i) lights", lights_created);
	// jo_printf(2, 12, "(%i) obj", objUP);
	
}

void	add_to_track_timer(int index) //Careful with index -- This function does not internally "DWO" - so only DWO outside, not inside here.
{
	
	short trackedLDATA = link_starts[LDATA>>12];
	short track_select = 0;
	short object_track = (dWorldObjects[index].type.ext_dat & 0xF00)>>8; //Get the level data's track #

	while(trackedLDATA != -1){
		if( (dWorldObjects[trackedLDATA].type.ext_dat & TRACK_DATA) == TRACK_DATA)
		{//WE FOUND SOME TRACK DATA
			track_select = dWorldObjects[trackedLDATA].type.entity_ID & 0xF; 
			if(track_select == object_track && (activeTrack == trackedLDATA || activeTrack == -1))
			{
				activeTrack = trackedLDATA;
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
	dWorldObjects[obj_id2].type.ext_dat |= 2; //Set checked flag up
	
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
	VECTOR outV0;
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
	fenceA[Y] = -((dWorldObjects[posts[0]].pos[Y]) - (main_map[ (-dWorldObjects[posts[0]].pix[X] + (main_map_x_pix * dWorldObjects[posts[0]].pix[Y]) + (main_map_total_pix>>1)) ]<<16));
	fenceA[Z] = -dWorldObjects[posts[0]].pos[Z];
	
	fenceB[X] = fenceA[X];
	fenceB[Y] = fenceA[Y] + (used_radius[Y]<<16);
	fenceB[Z] = fenceA[Z];
	
	fenceC[X] = -dWorldObjects[posts[1]].pos[X];
	fenceC[Y] = -((dWorldObjects[posts[1]].pos[Y]) - (main_map[ (-dWorldObjects[posts[1]].pix[X] + (main_map_x_pix * dWorldObjects[posts[1]].pix[Y]) + (main_map_total_pix>>1)) ]<<16));
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
	project_to_segment(tgtRelPos, fenceA, fenceC, edgePrj0, outV0);
		//The first condition is if our projection is beyond the X or Z radius of the face, we want to stop now.
	if(JO_ABS(edgePrj0[X]) > JO_ABS(outV0[X]>>1) || JO_ABS(edgePrj0[Z]) > JO_ABS(outV0[Z]>>1) )
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
	
	cross_fixed(rminusb, sminusb, cross);
	
	cross[X] = cross[X]>>8;
	cross[Y] = cross[Y]>>8;
	cross[Z] = cross[Z]>>8;
	
	normalize(cross, faceNormal);
	//Then a collision detector.
	tDist = ptalt_plane(tgt->pos, faceNormal, centerFace);

	// slPrintFX(tDist, slLocate(0, 12));
	// slPrintFX(dWorldObjects[obj_id1].dist, slLocate(0, 13));
	// jo_printf(12, 14, "(%i)", tDist ^ dWorldObjects[obj_id1].dist);

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
		dWorldObjects[activeObjects[index]].srot[Y] += 182; //Spin
		
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
			if((dWorldObjects[index].type.ext_dat & 0x3) != 0) return; //Return if the gate is already flagged as passed, or checked.
																	//Interesting: You can flag only one entity as being checked, and it still exits early.
																	//Why: because flagOne and flagTwo are no longer equal; one is checked, the other isn't.
																	//We prefer an exit earlier, though -- so we flag both as checked, so it exits here.
	short trackedEntry = link_starts[GATE_P>>12];
	unsigned short flagOne = dWorldObjects[index].type.ext_dat & 0x7FFF;
	unsigned short flagTwo = dWorldObjects[trackedEntry].type.ext_dat & 0x7FFF;

	//Goal: Check every entity in the GATE_P link list until the LINK is -1. When it is -1, stop. If it is equal to the current object's index, continue.
		while(trackedEntry != -1){
				if(flagOne == flagTwo && trackedEntry != index)
				{
					has_entity_passed_between(index, trackedEntry, tgt);
				}
			trackedEntry = dWorldObjects[trackedEntry].link; //Retrieve the linked entry from the current tracked entry
			flagTwo = dWorldObjects[trackedEntry].type.ext_dat & 0x7FFF; //Start testing for the newly tracked entity
		}
}

void	gate_track_manager(void)
{
	//Goal: Make a timer when you first pass through a gate ring or gate in a track
	//The timer will reset, and reset all the gates in the track, when it exceeds the time setting by LDATA
	//Also:
	//If the player's speed goes lower than the setting of the track's LDATA, it will reset.
	short trackedLDATA = link_starts[LDATA>>12];
	short trackedPOST = link_starts[GATE_P>>12];
	short trackedRING = link_starts[GATE_R>>12];
	
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
	
	int num_track_dat = 0;
	static char complete_ldat = 0;
	
	// jo_printf(0, 15, "tim(%i)", (dWorldObjects[activeTrack].type.ext_dat & 0xF)<<17);
	// jo_printf(0, 16, "act(%i)", activeTrack);

	
	while(trackedLDATA != -1){
				//jo_printf(0, 0, "(GTMN)"); //Debug ONLY
		if( (dWorldObjects[trackedLDATA].type.ext_dat & TRACK_DATA) == TRACK_DATA)
		{//WE FOUND SOME TRACK DATA
		track_select = dWorldObjects[trackedLDATA].type.entity_ID & 0xF; //Get the level data's track #
		dWorldObjects[trackedLDATA].pix[X] = 0;
		dWorldObjects[trackedLDATA].pix[Y] = 0;
		trackedRING = link_starts[GATE_R>>12]; //Re-set this link pointer (so we can re-scan)
		trackedPOST = link_starts[GATE_P>>12]; //Re-set this link pointer (so we can re-scan)
		num_track_dat++;
		//jo_printf(1, 12, "ldats(%i)", num_track_dat);
		//jo_printf(1, 13, "track(%i)", track_select);
				if(activeTrack == -1 || (activeTrack == trackedLDATA)) // if active track.. or track released
					{
					// jo_printf(0, 17, "ldt(%i)", trackedLDATA);
					// jo_printf(0, 17, "ldt(%i)", dWorldObjects[trackedLDATA].more_data);
			while(trackedRING != -1){
				//jo_printf(0, 0, "(RING)"); //Debug ONLY
				object_track = (dWorldObjects[trackedRING].type.ext_dat & 0xF00)>>8; //Get object track to see if it matches the level data track
				
					if(track_select == object_track)
					{
						if(dWorldObjects[trackedRING].type.ext_dat & 0x1 && !(dWorldObjects[trackedLDATA].more_data & OBJPOP))
						{
							dWorldObjects[trackedLDATA].type.ext_dat |= OBJPOP; //will set the track data as ACTIVE 
							dWorldObjects[trackedLDATA].pix[X]++;
						}

					dWorldObjects[trackedLDATA].pix[Y]++;
				//Reset if track reset enabled
						if(track_reset[track_select] == true)
						{
						dWorldObjects[trackedRING].type.ext_dat &= 0xFFFE; 
						}
					}
			trackedRING = dWorldObjects[trackedRING].link;
			}

			while(trackedPOST != -1){
				//jo_printf(0, 0, "(POST)"); //Debug ONLY
				object_track = (dWorldObjects[trackedPOST].type.ext_dat & 0xF00)>>8; //Get object track to see if it matches the level data track

					if(track_select == object_track)
					{
						if(dWorldObjects[trackedPOST].type.ext_dat & 0x1 && !(dWorldObjects[trackedLDATA].more_data & OBJPOP))
						{
							dWorldObjects[trackedLDATA].type.ext_dat |= OBJPOP; //will set the track data as ACTIVE 
							dWorldObjects[trackedLDATA].pix[X]++;
						}
					dWorldObjects[trackedLDATA].pix[Y]++;
				//Reset if track reset enabled
						if(track_reset[track_select] == true)
						{
						dWorldObjects[trackedPOST].type.ext_dat &= 0xFFFE; 
						}
					}
			trackedPOST = dWorldObjects[trackedPOST].link;
			}
			track_reset[track_select] = false;
				//jo_printf(0, 0, "(LDAT)"); //Debug ONLY
			//Track completion logic
			if(dWorldObjects[trackedLDATA].pix[X] == dWorldObjects[trackedLDATA].pix[Y] && dWorldObjects[trackedLDATA].pix[X] != 0)
			{
				dWorldObjects[trackedLDATA].type.ext_dat &= UNPOP;	//Set track as inactive
				dWorldObjects[trackedLDATA].more_data |= OBJPOP;	//Set track as complete
				trackTimers[track_select] = 0;	//Re-set the track timer
				activeTrack = -1;	//Release active track
				you.points += 10 * dWorldObjects[trackedLDATA].pix[X];
				complete_ldat++;
				pcm_play(snd_cronch, PCM_PROTECTED, 7); //Sound
				slPrint("                           ", slLocate(0, 6));
				slPrint("                           ", slLocate(0, 7));
			}

			//Timer run & check
			if((dWorldObjects[trackedLDATA].type.ext_dat & OBJPOP) != 0)
			{
				trackTimers[track_select] -= delta_time;
					if(trackTimers[track_select] < 0) //If timer expired...
					{
						dWorldObjects[trackedLDATA].type.ext_dat &= UNPOP;
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
		} else if((dWorldObjects[trackedLDATA].type.ext_dat & LEVEL_CHNG) == LEVEL_CHNG)
		{
				//if(you.points <= 0x15)
				//{
					//If you haven't crossed all the tracks, disable the level changer.
				//	dWorldObjects[trackedLDATA].type.ext_dat &= 0xFF7F; 
				//} else {
					//If you have enough points and crossed all the tracks, enable the level changer.
				//	dWorldObjects[trackedLDATA].type.ext_dat |= 0x80;
				//}
		}


		trackedLDATA = dWorldObjects[trackedLDATA].link;
	}//while LDATA end
	
	//Completed all tracks
	if(complete_ldat == num_track_dat && link_starts[LDATA>>12] > -1)
	{
		pcm_play(snd_win, PCM_PROTECTED, 7);
		complete_ldat = 0;
		//map_chg = false;
		//p64MapRequest(1);
	}
	
			if(activeTrack != -1){
				slPrint("Find the other wreath!", slLocate(0, 6));
				slPrintFX(trackTimers[dWorldObjects[activeTrack].type.entity_ID & 0xF], slLocate(0, 7));
			}
			
	//slPrintHex(dWorldObjects[trackedLDATA].type.ext_dat, slLocate(13, 12));
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


