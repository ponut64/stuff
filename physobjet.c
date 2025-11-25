#include <sl_def.h>
#include <SEGA_GFS.H>
#include "def.h"
#include "pcmsys.h"
#include "pcmstm.h"
#include "mymath.h"
#include "render.h"
#include "collision.h"
#include "ldata.h"
#include "menu.h"
#include "minimap.h"
#include "particle.h"
#include "sound.h"


#include "physobjet.h"
#include "object_definitions.c"

#include "pathing.c"
#include "actor.c"

_declaredObject * dWorldObjects; //In LWRAM - see lwram.c
_buildingObject * BuildingPayload;
_declaredObject emptyObject;

unsigned short objNEW = 0; //objNEW is the total number of declared objects
unsigned short objPREP[MAX_WOBJS + MAX_PHYS_PROXY]; //objPREP is a list of the delcared objects that will be drawn
unsigned short objDRAW[MAX_WOBJS + MAX_PHYS_PROXY];
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

void	purge_object_list(void)
{
	for(int i = 0; i < objNEW; i++)
	{
		dWorldObjects[i] = emptyObject;
	}
	objNEW = 0;
}

void	declare_object_at_cell(short posX, short height, short posZ, short type, ANGLE xrot, ANGLE yrot, ANGLE zrot, unsigned short more_data, unsigned short eeOrData)
{
		if(objNEW < MAX_WOBJS)
		{
			dWorldObjects[objNEW].pos[X] = -(posX<<16);
			dWorldObjects[objNEW].pos[Z] = -(posZ<<16);
			dWorldObjects[objNEW].pos[Y] = height<<16; //Vertical offset from ground
			dWorldObjects[objNEW].curSector = INVALID_SECTOR;
			dWorldObjects[objNEW].type = *objList[type];
			dWorldObjects[objNEW].type.ext_dat |= eeOrData;
			if((dWorldObjects[objNEW].type.ext_dat & ETYPE) != BUILD)
			{
				//why? idfk
			xrot = -xrot;
			yrot = -yrot;
			zrot = -zrot;
			}
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
		dWorldObjects[objNEW].curSector = building_item->sector;
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
		
		
		////////////////////////////////////////////////////
		// Rotation Inheritance
		// Objects inserted as items from a sector or building processor are loaded with a normal indicating the direction the item should face.
		// In this case, the Z+ axis will be more or less aligned with the direction of the normal, allowing only Y and X rotation in order to do this.
		// The processor "favors" Y rotation, that is, it shouldn't ever flip an object upside down in order to face a direction.
		// Note that because a single direction is being fed into a two-axis rotation, it is imperfect. 
		////////////////////////////////////////////////////
		int domain = solve_domain(building_item->normal[X], building_item->normal[Z]);

		int sin_y = slSin(fxAtan2(building_item->normal[Z]<<1, building_item->normal[X]<<1));
		int cos_y = slCos(fxAtan2(building_item->normal[Z]<<1, building_item->normal[X]<<1));

		switch(domain)
		{
		case(0):
		dWorldObjects[objNEW].rot[Y] = sin_y + cos_y + 32767;
		break;
		case(1):
		dWorldObjects[objNEW].rot[Y] = -sin_y + cos_y;
		break;
		case(2):
		dWorldObjects[objNEW].rot[Y] = sin_y - cos_y + 32767;
		break;
		case(3):
		dWorldObjects[objNEW].rot[Y] = -sin_y + cos_y;
		break;
		case(4):
		dWorldObjects[objNEW].rot[Y] = 32767;
		break;
		case(5):
		dWorldObjects[objNEW].rot[Y] = 0;
		break;
		case(6):
		dWorldObjects[objNEW].rot[Y] = -16384;
		break;
		case(7):
		dWorldObjects[objNEW].rot[Y] = 16384;
		break;
		default:
		break;
		}
		
		domain = solve_domain(building_item->normal[Y], building_item->normal[X]);
		dWorldObjects[objNEW].rot[X] = 0; //Init Zero
		
		int sin_x = slSin(fxAtan2(building_item->normal[Y]<<1, building_item->normal[X]<<1));
		int cos_x = slCos(fxAtan2(building_item->normal[Y]<<1, building_item->normal[X]<<1));

		switch(domain)
		{
		case(0):
		dWorldObjects[objNEW].rot[X] = sin_x + cos_x;
		break;
		case(1):
		dWorldObjects[objNEW].rot[X] = -sin_x - cos_x;
		break;
		case(2):
		dWorldObjects[objNEW].rot[X] = -sin_x + cos_x;
		break;
		case(3):
		dWorldObjects[objNEW].rot[X] = sin_x + cos_x;
		break;
		default:
		break;
		}
		
		int domain2 = solve_domain(building_item->normal[Y], building_item->normal[Z]);
		
		sin_x = slSin(fxAtan2(building_item->normal[Y]<<1, building_item->normal[Z]<<1));
		cos_x = slCos(fxAtan2(building_item->normal[Y]<<1, building_item->normal[Z]<<1));

		switch(domain2)
		{
		case(0):
		dWorldObjects[objNEW].rot[X] += sin_x + cos_x;
		break;
		case(1):
		dWorldObjects[objNEW].rot[X] += -sin_x - cos_x;
		break;
		case(2):
		dWorldObjects[objNEW].rot[X] += -sin_x + cos_x;
		break;
		case(3):
		dWorldObjects[objNEW].rot[X] += sin_x + cos_x;
		break;
		default:
		break;
		}
		
		int align_domains = domain & domain2;
		
		if(align_domains == 6) dWorldObjects[objNEW].rot[X] = 16384;
		if(align_domains == 7) dWorldObjects[objNEW].rot[X] = -16384;
		
			//nbg_sprintf_decimal(5, 12, building_item->normal[X]);
			//nbg_sprintf_decimal(5, 13, building_item->normal[Y]);
			//nbg_sprintf_decimal(5, 14, building_item->normal[Z]);
		
		objNEW++;
	}
}


void	object_control_loop(void)
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
	static int position_difference[XYZ] = {0,0,0};
	objUP = 0; //Should we start this at -1, because -1 will mean there are no objects in scene?
	static _declaredObject * obj;
	//nbg_sprintf(5, 10, "size(%i)", sizeof(xdata));
//Notice: Maximum collision tested & rendered items is MAX_PHYS_PROXY
	for(int i = 0; i < objNEW; i++)
	{
		obj = &dWorldObjects[i];
		//nbg_sprintf(0, 0, "(VDP1_BASE_CMDCTRL)"); //Debug ONLY
		
		//Since we are transitioning to a sector-based engine, all objects need a valid sector.
		//So we have to make sure everything with an invalid sector is given a valid one.
		if(obj->curSector == INVALID_SECTOR)
		{
			obj->curSector = broad_phase_sector_finder(obj->pos, levelPos, &sectors[obj->curSector]);
		}
		
		
		int etype = obj->type.ext_dat & ETYPE;
		unsigned short * obj_edat = (&obj->type.ext_dat);
		if(etype == SPAWNER && sectorIsVisible[obj->curSector])
		{
			if((*obj_edat) & SPAWNER_DISABLED) continue;
			if((*obj_edat) & SPAWNER_ACTIVE) continue;
			
			create_actor_from_spawner(obj, i);
			
		} else if(etype == LDATA)
		{ 		
				////////////////////////////////////////////////////
				//If the object type declared is LDATA (level data), use a different logic branch.
				////////////////////////////////////////////////////
				if(sectorIsVisible[obj->curSector]) 
				{
					//Get the position difference. This is uniquely used for level data collision.
					//For now, at least.
					position_difference[X] = JO_ABS(you.pos[X] + obj->pos[X]);
					position_difference[Y] = JO_ABS(you.pos[Y] + obj->pos[Y]);
					position_difference[Z] = JO_ABS(you.pos[Z] + obj->pos[Z]);
					
					// slPrintFX(position_difference[X], slLocate(2, 7));
					// slPrintFX(position_difference[Y], slLocate(2, 8));
					// slPrintFX(position_difference[Z], slLocate(2, 9));
					
					if(((*obj_edat) & LDATA_TYPE) == EVENT_TRIG && !((*obj_edat) & OBJPOP))
					{	
						if(position_difference[X] < (obj->type.radius[X]<<16)
						&& position_difference[Y] < (obj->type.radius[Y]<<16)
						&& position_difference[Z] < (obj->type.radius[Z]<<16))
						{
							if(((*obj_edat) & TRIGGER_TYPE) == TRIGGER_TYPE_PCM)
							{
								pcm_play(obj->more_data & MDAT_NUMBER, PCM_PROTECTED, obj->more_data>>8);
								(*obj_edat) |= OBJPOP;
							} else if(((*obj_edat) & TRIGGER_TYPE) == TRIGGER_TYPE_ADX)
							{
								start_adx_stream(stmsnd[obj->more_data & MDAT_NUMBER], obj->more_data>>8);
								(*obj_edat) |= OBJPOP;
							} else if(((*obj_edat) & TRIGGER_TYPE) == TRIGGER_TYPE_HUD)
							{
								start_hud_event(obj->more_data & MDAT_NUMBER);
							}
						}
					} else if(((*obj_edat) & LDATA_TYPE) == LEVEL_CHNG)
					{
						// We've found a level change trigger close to the player.
						// If we are close enough to the level change trigger and it is enabled, change levels.
						
						if(position_difference[X] < (obj->type.radius[X]<<16)
						&& position_difference[Y] < (obj->type.radius[Y]<<16)
						&& position_difference[Z] < (obj->type.radius[Z]<<16)
						//Enabling Booleans
						&& !((*obj_edat) & OBJPOP) && ((*obj_edat) & 0x80)) //what is 0x80??
						{
							//////////////////////////////////////////
							// Will change levels *(insofar as the uncommented code)
							//////////////////////////////////////////
							(*obj_edat) |= OBJPOP;
							//p64MapRequest(dWorldObjects[i].type.entity_ID);
						}
					} else if(((*obj_edat) & LDATA_TYPE) == MOVER_TARGET)
					{
							//nbg_sprintf(20, 16, "((typed))");
							
							//Send Time to Delay Count
							obj->type.effectTimeCount += delta_time;
							
							if(position_difference[X] < (obj->type.radius[X]<<16)
							&& position_difference[Y] < (obj->type.radius[Y]<<16)
							&& position_difference[Z] < (obj->type.radius[Z]<<16)
							//Enabling Booleans
							&& !((*obj_edat) & OBJPOP))
							{
								//so for a need to have a simple test, we'll just send it to the opposing trigger - but how?
								//well, first, we'll see if it is already active. If it is, we won't do anything.
								unsigned int moverTriggerLink = (obj->more_data & 0xFF00)>>8;
								_declaredObject * dwa = &dWorldObjects[moverTriggerLink];
								
								//nbg_sprintf(20, 16, "bif(%i)", moverTriggerLink);
								//Do not move until activation delay is satisfied.
							if(dwa->type.ext_dat & OBJPOP || obj->type.effectTimeCount <= obj->type.effectTimeLimit) continue;
	
							//Further, we want to delineate these by their trigger type.
							//If it is proximity, we let it through. If it is by action, we wait for a button.
							//If it is remote, it may not be activated here.
							//This is, by the way, the trigger type for the sensor being collision-tested.
								int trigger_type = obj->type.ext_dat & MOVER_TARGET_TYPE;
			if(trigger_type == MOVER_TARGET_PROX || (trigger_type == MOVER_TARGET_ACTION && is_key_pressed(you.actionKeyDef)))
								{
									dwa->type.ext_dat |= OBJPOP;
									dwa->type.effectTimeCount = 0;
									obj->type.effectTimeCount = 0;
									if(obj->type.effect != 0) pcm_play(obj->type.effect, PCM_SEMI, 5);
									//We also need to handle a case where the target waypoint is out of bounds.
							//In such case, the waypoint will evaluate to be in invalid sector; we need give it a valid one.
									dwa->curSector = (dwa->curSector == INVALID_SECTOR) ? obj->curSector : dwa->curSector;
									//nbg_sprintf(20, 16, "(triggered)");
								} else if(trigger_type == MOVER_TARGET_ACTION)
								{
									//Display a pop-up indicating that an action is available.
									start_hud_event(UsePrompt);
								}
							} else if((*obj_edat) & OBJPOP)
							{
								//In case where the POP flag is high, the mover should be active.
								//So uh, let's uh, move it?
								//This'll be interesting... (should really be a break-out function for this stuff)
								_declaredObject * dwa = &dWorldObjects[obj->more_data & 0xFF];
								
								//Do not move until time has passed
								if(obj->type.effectTimeCount <= obj->type.effectTimeLimit &&
								obj->type.ext_dat & MOVER_TARGET_DELAYED) continue;
								
								//First, we need to get the delta between the mover's current position, and the trigger's.
								static int dTrig[3];
								dTrig[X] = (obj->pos[X] - dwa->pos[X]);
								dTrig[Y] = (obj->pos[Y] - dwa->pos[Y]);
								dTrig[Z] = (obj->pos[Z] - dwa->pos[Z]);
								
								
								//Quick check: Are we within acceptable distance of the trigger already? If so, halt.
								if(JO_ABS(dTrig[X]) < (4<<16) && JO_ABS(dTrig[Y]) < (4<<16) && JO_ABS(dTrig[Z]) < (4<<16))
								{
									obj->type.ext_dat &= UNPOP;
									obj->type.effectTimeCount = 0;
									pcm_cease(obj->type.entity_ID);
									if(obj->type.clone_ID != 0) pcm_play(obj->type.clone_ID, PCM_SEMI, 6);
									//In case this activator was set as <Return>, we need to do something strange.
									//We need to go to the opposing trigger of this pair, and activate it.
									//The delay for the return will be according to the settings of the other point,
									//which may or may not be set to also return.
									//Door pairs will thusly have only the OPEN waypoint of the door marked as <Return>.
									if(obj->type.ext_dat & MOVER_TARGET_RETURN)
									{
										unsigned int moverTriggerLink = (obj->more_data & 0xFF00)>>8;
										//(note the shifting use of "dwa")
										dwa = &dWorldObjects[moverTriggerLink];
										dwa->type.effectTimeCount = 0;
										dwa->type.ext_dat |= OBJPOP;
									}
									continue;
								}
								if(obj->type.entity_ID != 0) pcm_play(obj->type.entity_ID, PCM_FWD_LOOP, 5);
								
								//A unit difference will be used to scale the movement.
								static int unitD[3];
								dTrig[X]>>=4;
								dTrig[Y]>>=4;
								dTrig[Z]>>=4;
								quick_normalize(dTrig, unitD);
								int speed_set = (obj->type.ext_dat & MOVER_TARGET_RATE)+1;
								RBBs[dwa->bbnum].velocity[X] = fxm(fxm(delta_time, speed_set<<21), unitD[X]);
								RBBs[dwa->bbnum].velocity[Y] = fxm(fxm(delta_time, speed_set<<21), unitD[Y]);
								RBBs[dwa->bbnum].velocity[Z] = fxm(fxm(delta_time, speed_set<<21), unitD[Z]);
								
								dwa->pos[X] += RBBs[dwa->bbnum].velocity[X];
								dwa->pos[Y] += RBBs[dwa->bbnum].velocity[Y];
								dwa->pos[Z] += RBBs[dwa->bbnum].velocity[Z];
							}
					}
				}
		} else if((sectorIsVisible[obj->curSector] && objUP < MAX_PHYS_PROXY))
			{
				//Exit rendering for collected items or inactive objects
				if(etype == ITEM && ((*obj_edat) & ITEM_COLLECTED)) continue;
				if(etype == OBJECT && ((*obj_edat) & OBJECT_DISABLED)) continue;
				if(etype == BUILD && ((*obj_edat) & OBJECT_DISABLED)) continue;

				if(entities[dWorldObjects[i].type.entity_ID].type != MODEL_TYPE_BUILDING)
				{
					////////////////////////////////////////////////////
					//If a non-building object was in rendering range, specify it as being populated, 
					//and give it matrix/bounding box parameters.
					////////////////////////////////////////////////////
					bound_box_starter.modified_box = &RBBs[objUP];
					bound_box_starter.x_location = dWorldObjects[i].pos[X];
					bound_box_starter.y_location = dWorldObjects[i].pos[Y];
					bound_box_starter.z_location = dWorldObjects[i].pos[Z];
					bound_box_starter.x_rotation = dWorldObjects[i].rot[X];
					bound_box_starter.y_rotation = dWorldObjects[i].rot[Y];
					bound_box_starter.z_rotation = dWorldObjects[i].rot[Z];

					bound_box_starter.x_radius = dWorldObjects[i].type.radius[X]<<16;
					bound_box_starter.y_radius = dWorldObjects[i].type.radius[Y]<<16;
					bound_box_starter.z_radius = dWorldObjects[i].type.radius[Z]<<16;
							
					makeBoundBox(&bound_box_starter, EULER_OPTION_XZY);
					RBBs[objUP].boxID = i;
					obj->bbnum = objUP;
					////////////////////////////////////////////////////
					//Set the box status. This branch of the logic dictates the box is:
					// 1. Render-able
					// 2. Collidable
					// 3. May or may not emit light
					////////////////////////////////////////////////////
					RBBs[objUP].status[0] = 'R';
					RBBs[objUP].status[1] = (((*obj_edat) & ETYPE) == GHOST) ? 'N' : 'C';
					RBBs[objUP].status[2] = (dWorldObjects[i].type.light_bright != 0) ? 'L' : 'N';
					////////////////////////////////////////////////////
					//	Effect Processor
					////////////////////////////////////////////////////
					object_effects(i, objUP);
					//Bit 15 of ext_dat is a flag that will tell the system if the object is on or not.
					(*obj_edat) |= OBJPOP;
					//This array is meant as a list where iterative searches find the entity type drawn.
					objPREP[objUP] = dWorldObjects[i].type.entity_ID;
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
					bound_box_starter.y_location = dWorldObjects[i].pos[Y];
					bound_box_starter.z_location = dWorldObjects[i].pos[Z];
					bound_box_starter.x_rotation = 0;
					bound_box_starter.y_rotation = 0;
					bound_box_starter.z_rotation = 0;
					
					bound_box_starter.x_radius = dWorldObjects[i].type.radius[X]<<16;
					bound_box_starter.y_radius = dWorldObjects[i].type.radius[Y]<<16;
					bound_box_starter.z_radius = dWorldObjects[i].type.radius[Z]<<16;
							
					makeBoundBox(&bound_box_starter, EULER_OPTION_XZY);
					RBBs[objUP].boxID = i;
					obj->bbnum = objUP;
					////////////////////////////////////////////////////
					//Set the box status. 
					//There isn't really a bound box for buildings.
					//They only need to be rendered, and in a special way, too.
					////////////////////////////////////////////////////
					RBBs[objUP].status[0] = 'R';
					RBBs[objUP].status[1] = (((*obj_edat) & ETYPE) == GHOST) ? 'N' : 'C';
					RBBs[objUP].status[2] = (dWorldObjects[i].type.light_bright != 0) ? 'L' : 'N';
					//Bit 15 of ext_dat is a flag that will tell the system if the object is on or not.
					(*obj_edat) |= OBJPOP;
					//This array is meant as a list where iterative searches find the entity type drawn.
					objPREP[objUP] = dWorldObjects[i].type.entity_ID;
					//This array is meant on a list where iterative searches can find the right object in the entire declared list.
					activeObjects[objUP] = i;
					//This tells you how many objects were updated.
					objUP++; 
			////////////////////////////////////////////////////
			// Object in render-range end stub
			////////////////////////////////////////////////////
				}
				} else {
				////////////////////////////////////////////////////
				//If the declared object was not in range, specify it as being unpopulated.
				////////////////////////////////////////////////////
				activeObjects[objUP] = 256;
				obj->bbnum = -1;
				(*obj_edat) &= UNPOP; //Axe bit 15 but keep all other data.
				////////////////////////////////////////////////////
				//If the declared object had a collision-approved type, re-set some collision parameters.
				////////////////////////////////////////////////////
				dWorldObjects[i].dist = 0;
			}
			////////////////////////////////////////////////////
			//Object control loop end stub
			////////////////////////////////////////////////////
		}
		
		manage_actors();
		
	// nbg_sprintf(12, 6, "objUP:(%i)", objUP);
	// nbg_sprintf(12, 7, "objNW:(%i)", objNEW);
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

void	has_entity_passed_between(short obj_id1, short obj_id2, _boundBox * tgt)
{	
	//////////////////
	// If the gate has no pair, return.
	// If the entity has yet to be loaded, return.
	// If the object is in the wrong direction to the other object, return.
	// Otherwise, flag the posts has having been checked this frame, then continue.
	//////////////////
	if(obj_id1 == obj_id2) return;
	//
	// I want* a way to calculate the chirality of object 1 to object 2. It's an optimization thing.
	// Domain is not acceptable. >X or <Y alone is not acceptable. Clockwise or anticlockwise is not acceptable.
	// Then what the hell is the rule?! If it's not CW or CCW, HUH?!
	//
	
	if(entities[dWorldObjects[obj_id1].type.entity_ID].file_done != true) return;
	
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
	// A - B
	// D - C
	//Extrapolate a quad out of the pos given
	fenceA[X] = -dWorldObjects[obj_id1].pos[X];
	fenceA[Y] = -dWorldObjects[obj_id1].pos[Y] + (dWorldObjects[obj_id1].type.radius[Y]<<16);
	fenceA[Z] = -dWorldObjects[obj_id1].pos[Z];
	
	fenceD[X] = fenceA[X];
	fenceD[Y] = fenceA[Y] - (dWorldObjects[obj_id1].type.radius[Y]<<17);
	fenceD[Z] = fenceA[Z];
	
	fenceB[X] = -dWorldObjects[obj_id2].pos[X];
	fenceB[Y] = -dWorldObjects[obj_id2].pos[Y] + (dWorldObjects[obj_id1].type.radius[Y]<<16);
	fenceB[Z] = -dWorldObjects[obj_id2].pos[Z];
	
	fenceC[X] = fenceB[X];
	fenceC[Y] = fenceB[Y] - (dWorldObjects[obj_id2].type.radius[Y]<<17);
	fenceC[Z] = fenceB[Z];

	//Makes a vector from point 3 to point 1.
	rminusb[X] = (fenceA[X] - fenceC[X]);
	rminusb[Y] = (fenceA[Y] - fenceC[Y]);
	rminusb[Z] = (fenceA[Z] - fenceC[Z]);
	//Makes a vector from point 2 to point 0.
	sminusb[X] = (fenceB[X] - fenceD[X]);
	sminusb[Y] = (fenceB[Y] - fenceD[Y]);
	sminusb[Z] = (fenceB[Z] - fenceD[Z]);
	
	fxcross(rminusb, sminusb, cross);
	
	cross[X] = cross[X]>>4;
	cross[Y] = cross[Y]>>4;
	cross[Z] = cross[Z]>>4;
	
	quick_normalize(cross, used_normal);


	//////////////////////////////////////////////////////////////
	// Grab the absolute normal used for finding the dominant axis
	//////////////////////////////////////////////////////////////
	fabs_norm[X] = JO_ABS(cross[X]);
	fabs_norm[Y] = JO_ABS(cross[Y]);
	fabs_norm[Z] = JO_ABS(cross[Z]);
	FIXED max_axis = JO_MAX(JO_MAX((fabs_norm[X]), (fabs_norm[Y])), (fabs_norm[Z]));
	dominant_axis = ((fabs_norm[X]) == max_axis) ? N_Xp : dominant_axis;
	dominant_axis = ((fabs_norm[Y]) == max_axis) ? N_Yp : dominant_axis;
	dominant_axis = ((fabs_norm[Z]) == max_axis) ? N_Zp : dominant_axis;
	//Use cross as the center of the span
	//Ugly, but it's what I did...
	cross[X] = (fenceA[X] + fenceB[X] + fenceC[X] + fenceD[X])>>2;
	cross[Y] = (fenceA[Y] + fenceB[Y] + fenceC[Y] + fenceD[Y])>>2;
	cross[Z] = (fenceA[Z] + fenceB[Z] + fenceC[Z] + fenceD[Z])>>2;

	//////////////////////////////////////////////////////////////
	// Collision Test Method: Chirality Check
	// This first tests, line by line, if the player is inside the shape on at least two axis.
	// Then the final axis is checked with a point-to-plane distance check.
	// The benefits of this is that the chirality check will exit early a lot of the time.
	//////////////////////////////////////////////////////////////
 	if(edge_wind_test(fenceA, fenceB, fenceC, fenceD, tgt->pos, dominant_axis, 16))
	{
		tDist = ptalt_plane(tgt->pos, used_normal, cross);
		if(dWorldObjects[obj_id1].dist != 0 && (tDist ^ dWorldObjects[obj_id1].dist) < 0)
		{
			//Statement with no effect
		}
	} 
					
	// int ab = edge_wind_test(fenceA, fenceB, tgt->pos, dominant_axis,16);
	// int bb = edge_wind_test(fenceB, fenceD, tgt->pos, dominant_axis,16);
	// int cb = edge_wind_test(fenceD, fenceC, tgt->pos, dominant_axis,16);
	// int db = edge_wind_test(fenceC, fenceA, tgt->pos, dominant_axis,16);

	// nbg_sprintf(0 + (obj_id1 * 5), 6, "(%i)", ab>>10);
	// nbg_sprintf(0 + (obj_id1 * 5), 7, "(%i)", bb>>10);
	// nbg_sprintf(0 + (obj_id1 * 5), 8, "(%i)", cb>>10);
	// nbg_sprintf(0 + (obj_id1 * 5), 9, "(%i)", db>>10);

	//nbg_sprintf(2,6 + obj_id1, "(%i)", tDist);
	dWorldObjects[obj_id1].dist = tDist;
}

//Goal:
//Look through the linked list of the type at "reference".
//If you can find an inactive entry of the same type as <reference>, copy <reference> to <inactive>.
//If you cannot, make a new entry.
void	create_dynamic_entity(_declaredObject * reference)
{
	unsigned short etype = GET_ETYPE(reference->type.ext_dat);
	_declaredObject * someDOJdata = get_first_in_object_list(etype);

	while(someDOJdata != &dWorldObjects[objNEW])
	{
		int garbage = someDOJdata->garbage;
		
		if(garbage)
		{
			//Why does this link method work?
			//The memory addresses have not changed. "Link" is an array index.
			//Therefore, it is only necessary that the new object maintain the same link; it will always be linked *to* correctly.
			reference->garbage = 0;
			reference->link = someDOJdata->link;
			*someDOJdata = *reference;
			
			return;
		}
		
		someDOJdata = step_linked_object_list(someDOJdata);
	}
	//Safety
	if(objNEW >= MAX_WOBJS) return;
	
	if(someDOJdata == &dWorldObjects[objNEW])
	{
		reference->garbage = 0;
		*someDOJdata = *reference;
		//We have to manage link here.
		someDOJdata->link = link_starts[(dWorldObjects[objNEW].type.ext_dat & 0x7000)>>12]; 
		link_starts[(someDOJdata->type.ext_dat & 0x7000)>>12] = objNEW; 
		objNEW++;
		return;
	}

	
}

void	item_by_type_processing(_declaredObject * item, unsigned short type)
{
	//empty
	
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
	
	if(dWorldObjects[activeObjects[index]].dist < dWorldObjects[activeObjects[index]].type.radius[X])
	{
		item_by_type_processing(&dWorldObjects[activeObjects[index]], GET_ITEM_TYPE(dWorldObjects[activeObjects[index]].type.ext_dat));
	}
	
}

void	subtype_collision_logic(_declaredObject * someOBJECTdata, _boundBox * stator, _boundBox * mover)
{
	if(stator->collisionID != mover->boxID) return; //If these objects aren't colliding with each other, exit.
	switch(someOBJECTdata->type.ext_dat & ETYPE)
	{
		case(OBJECT):
		//Type determinants for player-collided-with-object
		
		break;
		
		case(SPAWNER):
		//Type determinants for player-collided-with-actor.
		
		
		break;
		
	}
	//unsigned short otype = GET_OBJECT_TYPE(someOBJECTdata->type.ext_dat);
	
}

void	run_an_item_manager(_declaredObject * someLDATA)
{
	//I need to abstract entering stuff into this list.
	//Switch case statments will help.
	//aight so wtf am i doing lol
	_declaredObject * someITEMdata = get_first_in_object_list(ITEM);
	_declaredObject * someOBJECTdata = get_first_in_object_list(OBJECT);
	
	//unsigned short manager_series = someLDATA->type.entity_ID;
	//unsigned short manager_type = someLDATA->type.ext_dat & ITEM_CONDITION_TYPES;
	//unsigned short * edata;
	//Zero out some counters
	someLDATA->rot[X] = 0;
	someLDATA->rot[Y] = 0;
	
	while(someITEMdata != &dWorldObjects[objNEW])
	{
		//edata = &someITEMdata->type.ext_dat;
		
		someITEMdata = step_linked_object_list(someITEMdata);
	}
	
	while(someOBJECTdata != &dWorldObjects[objNEW])
	{
		//edata = &someOBJECTdata->type.ext_dat;
		
		someOBJECTdata = step_linked_object_list(someOBJECTdata);
	}
	
}

void	manage_object_data(void)
{
	//Objects are particular.
	//They can be part of item managers, or not.
	//This only manages the type of object data that are not managed by other means.
	//Items should also have a loop like this eventually, but not yet.
	
	_declaredObject * someOBJECTdata = get_first_in_object_list(OBJECT); 
	//unsiged short otype = GET_OBJECT_TYPE(someOBJECTdata->type.ext_dat);
	
	while(someOBJECTdata != &dWorldObjects[objNEW])
	{

		//First thing that's going here for this engine -
		//Buttons.
		//Buttons have a player-sized collision box for interactivity in front of them.
		//Key note: in front of. The rotation of the entity is important here.
		//
		
		if(sectorIsVisible[someOBJECTdata->curSector])
		{
			if((someOBJECTdata->type.ext_dat & OBJECT_TYPE) == REMOTE_ACTIVATOR)
			{
			
			//we need to find the bound box which is being used to draw this object
			//we are using a pointer-matching scheme from the boxID (which is the object number)
			//note that using pointers to match might not be portable
			_boundBox * bb = &RBBs[someOBJECTdata->bbnum];
				if(&dWorldObjects[bb->boxID] == someOBJECTdata)
				{
					//Cleared, matching boxes.
					//So now what we need to do is check if the player is in a box forward of the object.
					//"Forward" being the ascribed boxes Z axis.
					//1 - Get the Z axis. 2 - Get radius 3 - Multiply Z radius by Z axis 4 - Add box location to this
					int interact_point[3];
					interact_point[X] = fxm(bb->UVZ[X], bb->radius[Z]) - bb->pos[X];
					interact_point[Y] = fxm(bb->UVZ[Y], bb->radius[Z]) - bb->pos[Y];
					interact_point[Z] = fxm(bb->UVZ[Z], bb->radius[Z]) - bb->pos[Z];
					//We want to put this at the floor of the object, so we could push it down by radius Y.
					
					someOBJECTdata->dist = approximate_distance(you.pos, interact_point);
					
					
					if(someOBJECTdata->dist < (84<<16))
					{
						if(someOBJECTdata->type.ext_dat & REMOTE_ACT_USABLE)
						{
							_declaredObject * dwo = &dWorldObjects[someOBJECTdata->more_data];
							if((dwo->type.ext_dat & LDATA) == LDATA && (dwo->type.ext_dat & LDATA_TYPE) == MOVER_TARGET)
							{
								_declaredObject * dwa = &dWorldObjects[(dwo->more_data & 0xFF00)>>8];
								if(!(dwo->type.ext_dat & OBJPOP))
								{
								start_hud_event(UsePrompt);
									//Then if the button is pressed, we need to trigger the object flagged by the activator.
									//(A timer should be set to limit button activations to once in like, a half-second or so)
									if(is_key_struck(you.actionKeyDef))
									{
											if((dwo->type.ext_dat & MOVER_TARGET_TYPE) == MOVER_TARGET_CALLBACK)
											{
												dwo->type.light_bright+=1;
												dwa->type.light_bright = dwo->type.light_bright+1;
												if(dwo->type.light_bright & 1) {
													dwo->type.ext_dat |= OBJPOP;
													//Safety, such that it can be recalled when already moving.
													dwa->type.ext_dat &= UNPOP;
													dwa->type.effectTimeCount = 0;
												} else {
													dwa->type.ext_dat |= OBJPOP;
													//Safety, such that it can be recalled when already moving.
													dwo->type.ext_dat &= UNPOP;
													dwo->type.effectTimeCount = 0;
												}
											} else {
											dwo->type.ext_dat |= OBJPOP;
											//Safety, such that it can be recalled when already moving.
											dwa->type.ext_dat &= UNPOP;
											dwa->type.effectTimeCount = 0;
											}
											if(!(someOBJECTdata->type.ext_dat & REMOTE_ACT_RESET))
											{
												someOBJECTdata->type.ext_dat &= REMOTE_ACT_UNUSABLE;
											}

									}
								}
							}
						} else {
							start_hud_event(LockedPrompt);
						}
					}
				}
			}
		}
		someOBJECTdata = step_linked_object_list(someOBJECTdata);
	}
	
}


void	ldata_manager(void)
{
	_declaredObject * someLDATA = get_first_in_object_list(LDATA);
	
	while(someLDATA != &dWorldObjects[objNEW])
	{
		if((someLDATA->type.ext_dat & LDATA_TYPE) == LEVEL_CHNG)
		{
			someLDATA->type.ext_dat |= 0x80;
		} else if((someLDATA->type.ext_dat & LDATA_TYPE) == ITEM_MANAGER)
		{
			run_an_item_manager(someLDATA);
		}
		someLDATA = step_linked_object_list(someLDATA);
	}
			
	manage_object_data();
	
}

void	mover_target_initialization(_declaredObject * dummy)
{
	
	
	//Mover Target Data Initialization
	//First we are going to look for any mover target type objects.
	//Then, we to initialize that by searching for an object that has the entity ID 
	for(int i = 0; i < objNEW; i++)
	{
		_declaredObject * dwo = &dWorldObjects[i];
		
		if((dwo->type.ext_dat & LDATA) == LDATA && (dwo->type.ext_dat & LDATA_TYPE) == MOVER_TARGET)
		{
			int sector_source = dwo->curSector;
			
			//Now that we know the source sector, we have to go to that sector and find the entity pointer we want.
			//This code **should** work in theory but it is less than portable due to how pointer magic can work.
			
			entity_t * target_entity = sectors[sector_source].ent;
			dwo->curSector = INVALID_SECTOR;
			//When we find the correct object which represents the mover, its entry [k] will be stored in <more_data>.
			for(int k = 0; k < objNEW; k++)
			{
				if(k == i) continue;
				_declaredObject * dwa = &dWorldObjects[k];
				//If this isn't a build-type object or a normal object, don't try and use its entity_ID.
				if((dwa->type.ext_dat & ETYPE) != BUILD && (dwa->type.ext_dat & ETYPE) != OBJECT) continue;
				entity_t * roscule = &entities[dwa->type.entity_ID];
				if(roscule == target_entity)
				{
					dwo->more_data = k;
					break;
				}
				
			}

		}		
	}

	//Furthurmore, we need to search the declared object list for another mover target with the same target object as this.
	//We will then link to it.
	//This is a polar link; each mover target will link to the other one.
	//As such, as designed, the system will only work for two-pole movers. Enough for doors or simple elevators.

	for(int i = 0; i < objNEW; i++)
	{
		_declaredObject * dwo = &dWorldObjects[i];
		int found_pair = 0;
		int found_dwa = 0;
		if((dwo->type.ext_dat & LDATA) == LDATA && (dwo->type.ext_dat & LDATA_TYPE) == MOVER_TARGET)
		{
			if(dwo->more_data == 0) continue;
			found_dwa = 1;
			if(dwo->more_data & 0xFF00)
			{
				found_pair = 1;
				continue;
			}
			for(int k = 0; k < objNEW; k++)
			{
				if(k == i) continue;
				_declaredObject * dwa = &dWorldObjects[k];
				if((dwa->type.ext_dat & LDATA) == LDATA && (dwa->type.ext_dat & LDATA_TYPE) == MOVER_TARGET)
				{
					if(dwa->more_data == dwo->more_data)
					{
						if(dwa->more_data & 0xFF00)
						{
							found_pair = 1;
							continue;
						}
						//Establish the polar link.
						dwo->more_data |= k<<8;
						dwa->more_data |= i<<8;
						found_pair = 1;
					}
				}
			}
		}
		
	//In case we have not found a pair for this trigger, we need to declare a pair for it.
	//We shall assume this waypoint was not the mover's closed point, as such we will place a closed waypoint on the mover.
	//This closed waypoint will be the pair.
			if(!found_pair && found_dwa)
			{
					_declaredObject * dwa = &dWorldObjects[dwo->more_data & 0xFF];
					unsigned short * used_radius = &dwa->type.radius[0];
				
					BuildingPayload[total_building_payload].object_type = 63; //(this could really be anything)
					BuildingPayload[total_building_payload].pos[X] = dwa->pos[X]>>16;
					BuildingPayload[total_building_payload].pos[Y] = dwa->pos[Y]>>16;
					BuildingPayload[total_building_payload].pos[Z] = dwa->pos[Z]>>16;
					BuildingPayload[total_building_payload].sector = INVALID_SECTOR; //(manually linked later)
					//Some way to find what entity # we're working with right now
					BuildingPayload[total_building_payload].root_entity = WORLD_ENTITY_ID;
					//Declare the object from this preset
					declare_building_object(dummy, &BuildingPayload[total_building_payload]);
					total_building_payload++;
					//Copy relevant data from the trigger to the new object
					dwa = &dWorldObjects[objNEW-1];
					dwa->type.entity_ID = dwo->type.entity_ID;
					dwa->type.clone_ID = dwo->type.clone_ID;
					dwa->type.radius[X] = used_radius[X] + 16; //Use the radius of the mover + contact radius
					dwa->type.radius[Y] = used_radius[Y] + 48; //Contact radius higher here
					dwa->type.radius[Z] = used_radius[Z] + 16;
					dwa->type.effect = dwo->type.effect;
					dwa->type.effectTimeLimit = dwo->type.effectTimeLimit;
					dwa->type.ext_dat = LDATA | MOVER_TARGET | MOVER_TARGET_PROX | MOVER_TARGET_DELAYED | (dwo->type.ext_dat & MOVER_TARGET_RATE);
					//Default to simple proximity trigger
					//dwa->type.ext_dat |= (dwo->type.ext_dat & MOVER_TARGET_RATE);
					dwa->more_data = (dwo->more_data & 0xFF) | (i<<8);
					dwo->more_data |= (objNEW-1)<<8;
			}
	}

	
}

void	object_activator_initialization(void)
{
	
	for(int i = 0; i < objNEW; i++)
	{
		_declaredObject * dwo = &dWorldObjects[i];
		
		if((dwo->type.ext_dat & ETYPE) == OBJECT && (dwo->type.ext_dat & OBJECT_TYPE) == REMOTE_ACTIVATOR)
		{

			int seek_type = objList[dwo->curSector]->ext_dat;
			int low_dist = 1<<30;
			
			for(int k = 0; k < objNEW; k++)
			{
				if(k == i) continue;
				_declaredObject * dwa = &dWorldObjects[k];
				
				if(seek_type == dwa->type.ext_dat)
				{
					int new_dist = approximate_distance(dwo->pos, dwa->pos);
					
					if(new_dist < low_dist)
					{
						low_dist = new_dist;
						dwo->more_data = k;
					}
				}
			}
			dwo->curSector = INVALID_SECTOR;
		}
	}
	
//
}


void	post_ldata_init_building_object_search(void)
{
	//Insofar for the sector constructed level data, we just have to look through and see if it has the right entity ID.
	//This unfortunately requires a dummy object be constructed.
	static _declaredObject dummy;
	dummy.type.entity_ID = WORLD_ENTITY_ID; 
	dummy.pos[X] = levelPos[X];
	dummy.pos[Y] = levelPos[Y];
	dummy.pos[Z] = levelPos[Z];
	for(int b = 0; b < total_building_payload; b++)
	{	
		declare_building_object(&dummy, &BuildingPayload[b]);
	}
	
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
			
				
			dWorldObjects[i].more_data |= BUILD_PAYLOAD_LOADED;
		}
	}

	mover_target_initialization(&dummy);
	
	object_activator_initialization();

}



