#pragma once

#define CELL_CULLING_DIST_MED (10)
#define CELL_CULLING_DIST_LONG (11)
#define HEIGHT_CULLING_DIST	(512<<16)

#define OBJPOP	(0x8000) //Is populated?
#define UNPOP	(0x7FFF) //Unpop
#define ETYPE	(0x7000) //Entity type bits (may also define a specific entity type)
#define OBJECT	(0x0000) //Normal entity
#define ITEM	(0x1000) //Collectible item
#define GATE_P	(0x2000) //Gate post [has collision]
#define GATE_R	(0x3000) //Ring-type gate entity
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
#define LADDER_OBJECT		(0x010)
#define CLIMB_OBJECT		(0x020)
#define FORCEFIELD_TOUCH	(0x030)
#define FORCEFIELD_REMOTE	(0x040)
#define CRUSH_BLOCK_SLOW	(0x050)

#define OBJECT_FLAGS		(0xE)
#define OBJECT_RESET		(0x7FF0)
#define FF_TIMER_STARTED	(0x2)
#define FF_RESET_STARTED	(0x4)
#define FF_RESET_FINISH		(0x8)

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
///////////////////////////////////////////////////////////////
	bitflag orientation for ITEM_MANAGER

	_sobject
		entity_ID
			Specifies the item series # that this item manager will manage.
		clone_ID
			Use is specified by the manager type
		ext_dat	
			15 : Active boolean. When 1, the item manager's conditions will not be checked (marked complete).
			14-12 : 0x4000, specifies level data.
			11-8 : 0x500, specifies item manager data
			7-4 : Condition type data
			3-0 : Condition pass boolean
		radius
			Radius of item event trigger, if used.
		light_bright 
			Use is specified by the manager type
		light_y_offset
			Use is specified by the manager type
	_declaredObject
		pos
		pix
			Position of item event trigger, if used.
		rot
			rot[X] : The total # of items registered in this manager's series
			rot[Y] : The # of collected items registered in this manager's series
		dist
			Use is specified by the manager type
			MANAGER_7RINGS: Used to count the rings collected
			MANAGER_CTF: First, used as distance check to trigger. Then, stores completion time.
			MANAGER_RETURN_PT: Unused
		more_data 
			Use is specified by the manager type
			MANAGER_7RINGS: Unused
			MANAGER_CTF: Flag par time
			MANAGER_RETURN_PT: Unused
		link
			Links to other LDATA types
*/
#define ITEM_MANAGER_INACTIVE	(0x8000)
#define ITEM_MANAGER_ACTIVE		(0x7FFF)
#define ITEM_CONDITION_TYPES	(0xF0)
#define ITEM_CONDITION_FLAGS	(0xF)
#define CLEAR_MANAGER_FLAGS		(0xFFF0)
#define CLEAR_MANAGER_TYPES		(0xFF0F)
#define MANAGER_COLLECT_ALL		(0x00)
#define COLLECT_ALL_COMPLETE	(0x01)
#define MANAGER_7RINGS			(0x10)
#define MANAGER_CTF				(0x20)
#define MANAGER_RETURN_PT		(0x30)
#define CTF_FLAG_OPEN			(0x1)
#define CTF_FLAG_TAKEN			(0x2)
#define CTF_FLAG_CAPTURED		(0x4)


/*
///////////////////////////////////////////////////////////////
	bitflag orientation for GATE POST:
	
	_sobject
		entity_ID :
			Contains the entity id# to be drawn for this gate.
		clone_ID :
			Since Gate-posts can be allowed to be BUILD-type object, this is used to refer to the original entity ID of the _sobject.
		ext_dat :
			15 : popped / visible flag
			14-12 : "0x2000", defines gate post type object
			11-8 : Track # specification
			7-4 : Link specification. Two posts of a gate share this number. (Gate #)
			3-2 : 1 for first gate, 2 for last gate, 0 for any other gate / all gates.
			1 : Boolean collision check flag. Writes 1 when collision with gates checked, refresh to 0 on frame start.
			0 : Boolean gate pass check. If 1, gate has been passed. Refresh to 0 when track is failed (or otherwise reset).
	_declaredObject
		pix[XY] :
			Contains the location of the object, in grid units.
		more_data : 
			0: alignment boolean. 1: alignment complete / do not align. 0: to be aligned.
			1: discovery boolean. 1: player has discovered it. 0: player has not discovered it.
		link 
			declared object array entry of another gate post
			
*/
#define SET_GATE_POST_LINK(ext_dat, gate_num) ((ext_dat & 0xFF0F) | (gate_num << 4))
#define SET_GATE_TRACK_NUM(ext_dat, track_num) ((ext_dat & 0xF0FF) | (track_num << 8))
#define FIX_GATE_POST_ALIGNMENT(more_data)	(more_data | 0x1)
#define	GATE_POST_ALIGNED	(0x1)
#define GATE_POST_CHECKED	(0x2)
#define GATE_PASSED			(0x1)
#define GATE_DISCOVERED		(0x2)
#define GATE_UNPASSED		(0xFFFE)
#define GATE_UNCHECKED		(0xFFFD)
#define GATE_LINK_NUMBER	(0xF0)
//ext_dat bitflag orientation for GATE_RING: GATE_R
// 15 <- pop
// 14-12 <- pattern is "0x3000" for gate ring (single-object gate)
// 11-8 <- TRACK # [all gates in a TRACK share this #]
// 7-4 <- gate # / gate link #. Should be unique per gate declared.
// 3-2 <- first or last gate flag (1 for first gate, 2 for last gate, 0 for all else) (patterns 0x4 first, 0x8 last)
// 1 <- Unused
// 0 <- Will be 1 if this gate is passed
//ext_dat bitflag orientation for COLLISIONLESS:
// 15 <- pop
// 14-12 <- pattern is "0x5000" for collisionless entity
// all else unused
//

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
	TRACK_DATA orientation
			This really doesn't need to exist. I've added more ways to do this kind of stuff.
			So I could really just backport the track manager back into an item manager.
	_sobject
		entity_ID : Track # selection / Item series #
		radius[xyz] (empty)
		ext_dat :
			15 : Active boolean (0 for inactive, 1 for active)
			14-12 : 0x4000, specifies level data.
			11-8 : 0x100, specifies track data (this entry).
			7-4 : (empty)
			3:
			2: Track reset boolean
			1: Track discovered boolean
			0: Track complete boolean
		light_bright : Track fail speed setting (0 for no speed ??? )
		light_y_offset : Track timer setting (0 for no fail time ??? )
	_declaredObject
		pix[XY] :
			[X] - the number of gates in the track series that have been passed (for an active track).
			[Y] - the total # of gates in the track series
		more_data : 
			7-4 : The gate # to guide towards
			0-3 : The last gate # checked.
		dist :
			Silent discovery timer. Counts-down from X seconds when discovered, then enables track.
			Also the tracks timer host when running.
		link 
			declared object array entry of another level change
			**/
#define TRACK_RESET			(0x4)
#define TRACK_DISCOVERED	(0x2)
#define TRACK_COMPLETE		(0x1)
#define	TRACK_CLEAR_RESET	(0xFFFB)
#define TRACK_UNCOMPLETE	(0xFFFE)
#define TRACK_ACTIVE		(0x8000)
#define TRACK_INACTIVE		(0x7FFF)
#define TRACK_NO_CHECK		(0xFFF0)
#define TRACK_NO_GUIDE		(0xFF0F)
#define TRACK_PAR_TIME		(0xFF00)
#define TRACK_GUIDE_NUMBER	(0xF0)
#define TRACK_LAST_CHECKED	(0xF)
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
			declared object array entry of another level change
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
} _declaredObject;

//extern _declaredObject dWorldObjects[257];
extern _declaredObject * dWorldObjects; //In LWRAM - see lwram.c
extern _sobject * objList[64];
extern unsigned short objNEW;
extern unsigned short objPREP[MAX_WOBJS];
extern unsigned short objDRAW[MAX_WOBJS];
extern unsigned short activeObjects[MAX_WOBJS];
extern _buildingObject * BuildingPayload; //In LWRAM
extern short link_starts[8];
extern int total_building_payload;
extern int objUP;

_declaredObject * step_linked_object_list(_declaredObject * previous_entry);
_declaredObject * get_first_in_object_list(unsigned short object_type_specification);

void	fill_obj_list(void);
void	purge_object_list(void);
void	declare_object_at_cell(short pixX, short height, short pixY, short type, ANGLE xrot, ANGLE yrot, ANGLE zrot, unsigned short more_data, unsigned short eeOrData);
void	post_ldata_init_building_object_search(void);

void	declarations(void);

void	update_object(Uint8 boxNumber, int pixX, int pixY, FIXED Ydist, ANGLE rotx, ANGLE roty, ANGLE rotz, FIXED radx, FIXED rady, FIXED radz);

void	object_control_loop(int ppos[XY]);

void	light_control_loop(void);

void	has_entity_passed_between(short obj_id1, short obj_id2, _boundBox * tgt);

void	test_gate_ring(int index, _boundBox * tgt);
void	test_gate_posts(int index, _boundBox * tgt);

void	item_collision(int index, _boundBox * tgt);
void	subtype_collision_logic(_declaredObject * someOBJECTdata, _boundBox * stator, _boundBox * mover);

void	ldata_manager(void);

//cleaned out


