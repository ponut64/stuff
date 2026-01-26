#pragma once

#define CELL_CULLING_DIST_MED (10)
#define CELL_CULLING_DIST_LONG (11)
#define HEIGHT_CULLING_DIST	(512<<16)

#define OBJPOP	(0x8000) //Is populated?
#define UNPOP	(0x7FFF) //Unpop
#define ETYPE	(0x7000) //Entity type bits (may also define a specific entity type)
#define OBJECT	(0x0000) //Normal entity
#define ITEM	(0x1000) //Collectible item
#define MISSILE	(0x2000) //Projectile
#define SPAWNER	(0x3000) //Spawner for actors (NPCs)
#define LDATA	(0x4000) //Level data definition
#define GHOST	(0x5000) //Entity, but no collision (ghost)
#define BUILD	(0x6000) //Building. Per polygon collision. May have polygons or other elements that define other object types.
#define GET_ETYPE(ext_dat)	(ext_dat & ETYPE)

#define LDATA_TYPE		(0xF00) //Level data type bits

#define LEVEL_CHNG		(0x200) //Level data, level change location definition
#define PSTART			(0x300) //Level data, player start location definition
#define EVENT_TRIG		(0x400) //Level data, sound event trigger, stream-type
#define SDTRIG_PCM		(0x410) //Level data, sound event trigger, PCM-type
#define ITEM_MANAGER	(0x500) //Level data, item manager
#define MOVER_TARGET	(0x600) //Level data, mover waypoint data

#define MAX_WOBJS (128)
#define MAX_BUILD_OBJECTS (128)

#define BUILD_PAYLOAD_LOADED (0x8000)
#define BUILD_MASTER_OCCLUDER (0x800)
/*
///////////////////////////////////////////////////////////////
	ext_dat bitflag orientation for BUILD:
		15 <- pop
		14-12 <- will be 0x6 (1 1 0) for BUILD
		11	<- Flag for "Master Occluder" object
		10-1 : Unused
		0: Object enable/disable flag
	clone_id <- Permutation series definition. Has the entity ID of the mesh (in the entity list) which this object is a permutation of.
		If there is no permutation, this otherwise has the entity ID of the building object. 

*/

/*
///////////////////////////////////////////////////////////////
bitflag orientation for SPAWNER:
	_sobject
		entity_ID
			Drawn entity ID for the actor.
		clone_ID
			Series ID. Objects that share this # are registered to the same item manager (if one exists).
		ext_dat	
			15 : popped / visible flag
			14-12 : 0x3000 specifies spawner data
			11: Unused
			10: Unused
			9: Spawner active flag (will not try to spawn again if 1)
			8: Spawner disabled flag (will not try to spawn at all if 1)
			7-0: Spawned actor type bits
		radius
			Collision radius of the actor spawned
		light_bright / light_y_offset
			Unused (?)
	_declaredObject
		pos: Position of the actor spawned
		pix: Approx. grid location of the spawner
		rot: Orientation of the actor spawned. Only the Y axis applies.
		more_data :
			No data right now
		dist :
			Unused (?)
///////////////////////////////////////////////////////////////
*/
#define SPAWNED_ACTOR_TYPE	(0xFF)
#define SPAWNER_DISABLED	(0x100)
#define SPAWNER_ACTIVE		(0x200)
#define SET_SPAWNED_TYPE(ext_dat, object_num)	(ext_dat | (object_num & SPAWNED_ACTOR_TYPE))
#define GET_SPAWNED_TYPE(ext_dat)				(ext_dat & SPAWNED_ACTOR_TYPE)
#define SPAWNER_T_EXAMPLE	(0x1)

/*
///////////////////////////////////////////////////////////////
bitflag orientation for OBJECT:
	_sobject
		entity_ID
			Drawn entity ID for the item.
		clone_ID
			Series ID. Objects that share this # are registered to the same item manager (if one exists).
		ext_dat	
			15 : popped / visible flag
			14-12 : 0x0000 specifies object data (default)
			11-8 : Object type definition
				#0: Normal collidable object.
				#1: Ladder climbable object
			7-4: Type-specific flags
			3-1: Reserved for type-specific data
			0: Enable/disable flag. If 1, the object will not be displayed or collided with at all.
		radius
			Collision radius of the object
		light_bright / light_y_offset
			Data for lighting
	_declaredObject
		Since this object is rendered, conventional use applies.
		
		more_data :
			No data right now
		dist :
			Reserved for type-specific data
*/
#define OBJECT_TYPE			(0xF00)
#define NO_TYPE				(0x000)
#define OBJECT_DISABLED		(0x1)
#define OBJECT_ENABLE		(0x7FFE)
#define SET_OBJECT_TYPE(ext_dat, object_num)	(ext_dat | object_num)
#define GET_OBJECT_TYPE(ext_dat)				(ext_dat & OBJECT_TYPE)
#define OBJECT_DESTRUCTIBLE	(0x100)
#define CLIMB_OBJECT		(0x200)
#define LADDER_OBJECT		(0x300)

#define OBJECT_FLAGS		(0xE)
#define OBJECT_RESET		(0x7FF0)
#define DESTRUCTIBLE_HEALTH	(0xE)


/*
Proximity Object Activator Layout Information

	_sobject
		entity_ID: entity # to draw for this trigger
		clone_ID: <-conventional use applies->
		radius: <-conventional use applies->
		ext_dat :
			15 : popped / visible flag
			14-12: "OBJECT" definition
			8-11: "Object Activator" definition
			7-4: <unused>
			3: Reset flag (if 1, the trigger will reset according to the plausible states set in the triggered item)
			2: Proximity or activation flag (if 0, triggers by proximity. if 1, gives a prompt and triggers on button press)
			1: Usable flag (if 0, trigger is usable. If 1, trigger is unusable)
			0: Enable/disable flag. If 1, the object will not be displayed or collided with at all.
		effect : Effect to play upon use (can include sound)
		effectTimeLimit : <--Use appropriate to effect system-->
	_declaredObject
		Since this object is rendered, conventional use applies.
		curSector
			Pre-processor uses curSector as object archetype for search for. Otherwise, conventional use applies (once rendered).
		link 
			declared object array for more objects
	more_data :
		the object ID to be manipulated by this trigger
		
	Special note:
	the location an actor or player will have to be in to interact with this trigger will be in front of it.
	so be mindful of the rotation of the entity, as this will affect the interaction with this trigger.
	Further, a standard player-sized interaction box will be set with no other positional adjustments.
	
*/

#define REMOTE_ACTIVATOR	(0x400)

#define REMOTE_ACT_TYPE		(0xF0)
#define SET_REMOTE_TYPE		(ext_dat, obj_type)		(ext_dat | (obj_type>>8))
#define REMOTE_ACT_RESET	(0x8)
#define REMOTE_ACT_PROX		(0x4)
#define REMOTE_ACT_USABLE	(0x2)
#define REMOTE_ACT_UNUSABLE (0xFFFD)


/*
///////////////////////////////////////////////////////////////
	bitflag orientation for ITEM:

	_sobject
		entity_ID
			Drawn entity ID for the item.
		clone_ID
			Series ID. Items that share this # are registered to the same item manager (if one exists).
		ext_dat	
			15 : popped / visible flag
			14-12 : 0x1000 specifies ITEM data.
			11-4 : item sub-type flag. Just determines what the game does with it.
				#0 :
				#1 :
			3-1 : Bits reserved for use by item type
			0 : Collected flag (used by some items). If 1, the object has been collected.
		radius
			Collision radius of the item
		light_bright / light_y_offset
			Data for lighting
	_declaredObject
		Since this object is rendered, conventional use applies.
		
		more_data :
			0 : <unused>
*/
#define ITEM_COLLECTED			(0x1)
#define ITEM_RESET				(0x7FFE)
#define ITEM_TYPE				(0xFF0)
#define SET_ITEM_TYPE(ext_dat, item_num)	(ext_dat | (item_num & 0xFF)<<4)
#define GET_ITEM_TYPE(ext_dat)				((ext_dat & ITEM_TYPE)>>4)
#define ITEM_FLAGS				(0xE)
#define ITEM_NO_FLAGS			(0x7FF0)


/*
	EVENT_TRIG orientation:
	_sobject
		ext_dat 
			15 : Boolean. Enable/disable trigger.
			14-12 : "LDATA" definition bits.
			8-11 : "EVENT_TRIG" definition bits.
			7-5 : nothing
			4 : Stream or from-memory bitflag. 
				If bit 4 is low, the sound is treated as an ADX stream, and is selected from a special ADX stream list.
				If bit 4 is high, the sound is played from the on-hand PCM list on the driver.
			0-3 : Presently unused.
		radius
			the radius of the trigger (in >>16 units)
	_declaredObject
		pos[xyz]
			Location of the trigger
		more_data
			7 - 0 : The sound number intended to play back.
			11 - 8 : The volume of the sound; directly related to the volume sent to the driver (shift right eight).
		link 
			declared object array entry of another level data
*/

#define DISABLE_TRIGGER 	(0x8000)
#define ENABLE_TRIGGER		(0x7FFF)
#define TRIGGER_TYPE		(0xF0)
#define TRIGGER_TYPE_ADX	(0x10)
#define TRIGGER_TYPE_PCM	(0x00)
#define TRIGGER_TYPE_HUD	(0x20)

#define MDAT_NUMBER			(0xFF)
#define MDAT_VOLUME			(0xF00)
/**
//////////////////////////////////////////////////////////////////
	LEVEL_CHNG orientation:
	_sobject
		entity_ID :
			0-7 : Level to load
		radius[xyz]
			Distance from position in which to trigger, if enabled
		ext_dat
			15 	: Boolean. True if the trigger has been used, false if it has not.
			14-12 : "LDATA" definition bits.
			8-11 : "LEVEL_CHNG" definition bits.
			7 	: Boolean. True if trigger is enabled, false if trigger is disabled.
			0-6	: Information about what information to check to enable or disable the level changer.
	_declaredObject
		pos[xyz] 
			Location of the trigger
		link 
			declared object array entry of another level change
//////////////////////////////////////////////////////////////////
	PSTART orientation:
	_sobject
		ext_dat
			15 : Boolean. If player has started here or not.
			14-12 : "LDATA" definition bits.
			8-11 : "PSTART" definition bits.
			0-7 : Presently unused.
	_declaredObject
		pos[xyz] 
			Location of the trigger
		link 
			declared object array for more level data
/////////////////////////////////////////////////////////////////

**/

/*
Mover Target Bitwise Layout Information

	

	_sobject
		entity_ID: Sound number to be played upon starting (0 counts as no sound)
		clone_ID: Sound number to be played upon stopping (0 counts as no sound)
		radius: the radius of the trigger
		ext_dat :
			15 : Active (0 if inactive, 1 if being used as mover target)
			14-12: "LDATA" definition
			8-11: "MOVER_TARGET" definition
			7 : Trigger with delay
			6 : Return after destination reached?
			5-4 : Trigger type bits (By other object, proximity, or action button)
			0-3 : Mover speed (in time-scaled arbitrary units)
		effect : Sound number to be played upon trigger (0 counts as no sound)
		effectTimeLimit : Time limit of trigger, return, and activation delay
		effectTimeCount : Time counter of trigger, return, and activation delay
	_declaredObject
		pos[xyz] 
			Location of the trigger
		link 
			declared object array for more level data
	more_data :
		15-8:the object ID of the opposing mover trigger
		0-7 :the object ID to be manipulated as a mover by this trigger
		
*/

#define MOVER_TARGET_DELAYED	(0x80)
#define MOVER_TARGET_RETURN		(0x40)
#define MOVER_TARGET_TYPE		(0x30)
#define MOVER_TARGET_PROX		(0x10) //(Triggered by proximity)
#define MOVER_TARGET_ACTION		(0x20) //(Triggered by player action button)
#define MOVER_TARGET_REMOTE		(0x30) //(Triggered by another game object, typically for door return points)
#define MOVER_TARGET_CALLBACK	(0x00) //(Untyped trigger; when found, operates as toggle)
#define CLEAR_MOVER_TARGET		(0xFFCF)
#define MOVER_TARGET_RATE		(0xF)


#define OBJECT_ENTRY_CAP (128)

typedef struct {
	unsigned char entity_ID;
	unsigned char clone_ID;
	unsigned short radius[XYZ];
	unsigned short ext_dat;
	unsigned short light_bright;
	unsigned short light_y_offset;
	unsigned short effect;
	int effectTimeLimit;
	int effectTimeCount;
} _sobject;

typedef struct {
	unsigned short object_type;
	short pos[XYZ];
	unsigned char sector;
	unsigned char root_entity; //(we need to be able to use this as a sector specification, or split the 2 bytes)
	int normal[XYZ];
} _buildingObject;

typedef struct {
	int		pos[XYZ];
	int		dist; 
	_sobject type;
	unsigned short	curSector;
	unsigned short	more_data;
	short	sound_num; //Sound instance number emitted by the object
	short	bbnum; //id# of the box used for this object (if physical). note: this is dynamic, so you have to check the box too.
	short	link; //links to another object of same type. -1 for last in-list. used by garbage collector, do not mess with.
	short	garbage; //Stuff for garbage collector
	ANGLE	rot[XYZ];
} _declaredObject;

//extern _declaredObject dWorldObjects[257];
extern _declaredObject * dWorldObjects; //In LWRAM - see lwram.c
extern _sobject * objList[OBJECT_ENTRY_CAP];
extern unsigned short objNEW;
extern unsigned short objPREP[MAX_WOBJS + MAX_PHYS_PROXY];
extern unsigned short objDRAW[MAX_WOBJS + MAX_PHYS_PROXY];
extern unsigned short activeObjects[MAX_WOBJS];
extern _buildingObject * BuildingPayload; //In LWRAM
extern short link_starts[8];
extern int total_building_payload;
extern int objUP;

_declaredObject * step_linked_object_list(_declaredObject * previous_entry);
_declaredObject * get_first_in_object_list(unsigned short object_type_specification);

void	fill_obj_list(void);
void	purge_object_list(void);
void	declare_object_at_cell(short posX, short height, short posZ, short type, ANGLE xrot, ANGLE yrot, ANGLE zrot, unsigned short more_data, unsigned short eeOrData);
void	post_ldata_init_building_object_search(void);

void	declarations(void);

void	object_control_loop(void);

void	light_control_loop(void);

void	has_entity_passed_between(short obj_id1, short obj_id2, _boundBox * tgt);

void	item_collision(int index, _boundBox * tgt);
void	subtype_collision_logic(_declaredObject * someOBJECTdata, _boundBox * stator, _boundBox * mover);

void	ldata_manager(void);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// used for actor.c

#define MAX_PATHING_STEPS	(4)
#define ACTOR_PATH_EXCEPTION_TIME (1<<16) //(maybe i should scale this based on actor's size or speed)

typedef struct {
	int * pos; //the position of the path step
	int * dir; //the direction out of the path step (may not be used)
	short actorID; //the ID of the actor or actor group using this path
	short winding; //Left or right winding for this path step
	unsigned char fromSector; //sector to path from
	unsigned char toSector; //sector to path to
} _pathStep;

typedef struct {
	char numStepsUsed[MAX_PHYS_PROXY];
	_pathStep steps[MAX_PHYS_PROXY][MAX_PATHING_STEPS];
} _pathStepHost;

extern _pathStepHost pathStepHeap;

typedef struct {
	union {
		short raw;
		struct {
			unsigned char losTarget:1;
			unsigned char alive:1;
			unsigned char active:1;
			unsigned char hitWall:1;
			unsigned char hitFloor:1;
			unsigned char inCombat:1;
			unsigned char locked:1;
			unsigned char looking:1;
			unsigned char onPathNode:1;
		} flags;
	};
} _actor_info;

typedef struct {
	int pos[3];
	int nextPos[3];
	int dV[3];
	int velocity[3];
	int dirUV[3];
	int pathUV[3];
	int pathTarget[3];
	int pathGoal[3];
	int exceptionPos[3];
	int exceptionDir[3];
	int blockedLOSNorm[3];
	_boundBox * box;
	int entity_ID;
	_declaredObject * spawner;
	int lifetime;
	//Collision Information
	int floorPos[3];
	int wallPos[3];
	int totalFriction;
	int exceptionTimer;
	int animationTimer;
	int idleActionTimer;
	int aggroTimer;
	short rot[3];
	short dRot[3];
	short curPathStep;
	short exceptionStep;
	unsigned short curSector;
	unsigned short prevSector;
	unsigned short health;
	unsigned short maxHealth;
	unsigned short boxID;
	unsigned short goalSector;
	unsigned short pathingLatch;
	unsigned short markedSector;
	unsigned short atGoal;
	unsigned short animPriorityQueue; //Used to register allowed animations in a bitwise priority queue
	unsigned short animState; //Reports the current animation state (certain gamestates may need animation commands to change actor behavior)
	_actor_info info;
	unsigned char type;

} _actor;

extern _actor spawned_actors[MAX_PHYS_PROXY];
extern unsigned char * sectorPathHeap;
extern unsigned char * pathStackPtr;
extern unsigned char * pathStackMax;

void	init_pathing_system(void);
void	actorPopulateGoalInfo(_actor * act, int * goal, int target_sector);

void	pathing_exception(int actor_id);
void	actor_hit_wall(_actor * act, int * wall_norm);

int		actorMoveToPos(_actor * act, int * target, int rate, int gap);
int		create_actor_from_spawner(_declaredObject * spawner, int boxID);
void	manage_actors(void);

