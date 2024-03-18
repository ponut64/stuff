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
#define TRACK_DATA		(0x100) //Level data, gate data definition
#define LEVEL_CHNG		(0x200) //Level data, level change location definition
#define PSTART			(0x300) //Level data, player start location definition
#define EVENT_TRIG		(0x400) //Level data, sound event trigger, stream-type
#define SDTRIG_PCM		(0x410) //Level data, sound event trigger, PCM-type
#define ITEM_MANAGER	(0x500) //Level data, item manager

#define MAX_WOBJS (256)
#define MAX_BUILD_OBJECTS (256)

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
			11-4 : Object type definition
				#0: Normal collidable object.
				#1: Ladder climbable object
				#2: Free-climbable object
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
#define OBJECT_TYPE			(0xFF0)
#define NO_TYPE				(0x000)
#define OBJECT_DISABLED		(0x1)
#define OBJECT_ENABLE		(0x7FFE)
#define SET_OBJECT_TYPE(ext_dat, object_num)	(ext_dat | (object_num & 0xFF)<<4)
#define GET_OBJECT_TYPE(ext_dat)				(ext_dat & OBJECT_TYPE)
#define OBJECT_DESTRUCTIBLE	(0x10)
#define CLIMB_OBJECT		(0x20)
#define LADDER_OBJECT		(0x30)

#define OBJECT_FLAGS		(0xE)
#define OBJECT_RESET		(0x7FF0)
#define DESTRUCTIBLE_HEALTH	(0xE)


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
				#0 : Normal collectible 
				#1 - #7: 7 Rings special collectible
				#8: Flag
			3-1 : Bits reserved for use by item type
			0 : Collected flag. If 1, the object has been collected.
		radius
			Collision radius of the item
		light_bright / light_y_offset
			Data for lighting
	_declaredObject
		Since this object is rendered, conventional use applies.
		
		more_data :
			0 : Always collided / snap-collision flag. If 1, collision will always evaluate as true (if possible to collide).
*/
#define ITEM_COLLECTED			(0x1)
#define ITEM_RESET				(0x7FFE)
#define ITEM_TYPE				(0xFF0)
#define SET_ITEM_TYPE(ext_dat, item_num)	(ext_dat | (item_num & 0xFF)<<4)
#define GET_ITEM_TYPE(ext_dat)				((ext_dat & ITEM_TYPE))
#define ITEM_TYPE_PTADR			(0x00)
#define ITEM_TYPE_RING1			(0x10)
#define ITEM_TYPE_RING2			(0x20)
#define ITEM_TYPE_RING3			(0x30)
#define ITEM_TYPE_RING4			(0x40)
#define ITEM_TYPE_RING5			(0x50)
#define ITEM_TYPE_RING6			(0x60)
#define ITEM_TYPE_RING7			(0x70)
#define ITEM_TYPE_FLAG			(0x80)
#define ITEM_FLAGS				(0xE)
#define ITEM_NO_FLAGS			(0x7FF0)
#define FLAG_OPEN				(0x2)
#define FLAG_GRABBED			(0x4)
#define FLAG_RETURN				(0x8)

#define ITEM_MDATA_SNAP_COLLISION	(0x1)


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
			declared object array entry of another level start
/////////////////////////////////////////////////////////////////

**/


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
	unsigned short root_entity;
} _buildingObject;

typedef struct {
	int		pos[XYZ];
	short 	pix[XY];
	ANGLE	rot[XYZ];
	_sobject type;
	int		dist; 
	unsigned short	more_data;
	short	link; //Has the declared object list ID of the next object in the list. -1 for last-in-list.
	short	garbage; //Stuff for garbage collector
} _declaredObject;

//extern _declaredObject dWorldObjects[257];
extern _declaredObject * dWorldObjects; //In LWRAM - see lwram.c
extern _sobject * objList[64];
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

void	object_control_loop(int ppos[XY]);

void	light_control_loop(void);

void	has_entity_passed_between(short obj_id1, short obj_id2, _boundBox * tgt);

void	item_collision(int index, _boundBox * tgt);
void	subtype_collision_logic(_declaredObject * someOBJECTdata, _boundBox * stator, _boundBox * mover);

void	ldata_manager(void);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// used for actor.c

#define MAX_PATHING_STEPS	(30)

typedef struct {
	union {
		char raw;
		struct {
			unsigned char losTarget:1;
			unsigned char alive:1;
			unsigned char active:1;
			unsigned char hitWall:1;
			unsigned char hitFloor:1;
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
	_boundBox * box;
	int entity_ID;
	_declaredObject * spawner;
	int lifetime;
	//Collision Information
	int floorPos[3];
	int wallPos[3];
	int totalFriction;
	short rot[3];
	short dRot[3];
	short pix[2];
	unsigned short health;
	unsigned short maxHealth;
	unsigned short boxID;
	_actor_info info;
	unsigned char type;

} _actor;

extern _actor spawned_actors[MAX_PHYS_PROXY];
extern unsigned char * adjacentPolyHeap;
extern unsigned char * pathTableHeap;
extern unsigned char * adjPolyStackPtr;
extern unsigned char * adjPolyStackMax;

void	init_pathing_system(void);

int		create_actor_from_spawner(_declaredObject * spawner, int boxID);
void	manage_actors(int * ppix, int * ppos);

