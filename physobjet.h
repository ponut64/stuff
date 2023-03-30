#ifndef __PHYSOBJET_H__
#define __PHYSOBJET_H__

#define CELL_CULLING_DIST_MED (10)
#define CELL_CULLING_DIST_LONG (14)
#define HEIGHT_CULLING_DIST	(512<<16)

#define OBJPOP	(0x8000) //Is populated?
#define UNPOP	(0x7FFF) //Unpop
#define OTYPE	(0x7000) //Entity type bits (may also define a specific entity type)
#define OBJECT	(0x0000) //Normal entity
#define ITEM	(0x1000) //Collectible item
#define GATE_P	(0x2000) //Gate post [has collision]
#define GATE_R	(0x3000) //Ring-type gate entity
#define LDATA	(0x4000) //Level data definition
#define GHOST	(0x5000) //Entity, but no collision (ghost)
#define BUILD	(0x6000) //Building. Per polygon collision. May have polygons or other elements that define other object types.

#define SUB_DATA (0xFFF)
#define LADDER	(0x800)
#define CLIMBABLE (0xC00)

#define LDATA_TYPE	(0xF00) //Level data type bits
#define TRACK_DATA	(0x100) //Level data, gate data definition
#define LEVEL_CHNG	(0x200) //Level data, level change location definition
#define PSTART		(0x300) //Level data, player start location definition
#define SOUND_TRIG	(0x400) //Level data, sound event trigger, stream-type
#define SDTRIG_PCM	(0x410) //Level data, sound event trigger, PCM-type

#define MAX_WOBJS (512)
#define MAX_BUILD_OBJECTS (256)

#define BUILD_PAYLOAD_LOADED (0x8000)
/*
///////////////////////////////////////////////////////////////
	ext_dat bitflag orientation for BUILD:
		15 <- pop
		14-12 <- will be 0x6 (1 1 0) for BUILD
		11 - 0 <- unused
	clone_id <- Permutation series definition. Has the entity ID of the mesh (in the entity list) which this object is a permutation of.
		If there is no permutation, this otherwise has the entity ID of the building object. 

*/

//ext_dat bitflag orientation for ITEM:
// 15 <- pop
// 14-12 <- Pattern is "0x1000" for ITEM
// 11-4 <- ?
// 3 	<- Root entity bitflag [Booleans]
// 2-0 <- ?
//ext_dat bitflag orientation for OBJECT:
// 15 <- pop
// 14-12 <- pattern is "0 0 0" for OBJECT
// 11 <- Flags entity as "climbable" or as "ladder". This entities' walls can be climbed vertically.
/*
///////////////////////////////////////////////////////////////
	bitflag orientation for GATE POST:
		ext_dat 
			15 <- pop
			14-12 <- pattern is "0x2000" for gate post objects
			11-8 <- TRACK # [all gates in a series share this #]
			7-4 <- Link specification [gates in the same TRACK that share this # form the two sides of a gate]
			3-2 <- first or last gate flag (1 for first gate, 2 for last gate, 0 for all else) (patterns 0x4 first, 0x8 last)
			1 <- Will be 1 if the post has had its collision checked this frame
			0 <- Will be 1 if this gate is passed

	_sobject
		entity_ID :
			Contains the entity id# to be drawn for this gate.
		ext_dat :
			15 : popped / visible flag
			14-12 : "0x2000", defines gate post type object
			11-8 : Track # specification
			7-4 : Link specification. Two gates of a post share this number.
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
#define SET_GATE_POST_LINK(ext_dat, gate_num) (ext_dat | (gate_num << 4))
#define SET_GATE_TRACK_NUM(ext_dat, track_num) (ext_dat | (track_num << 8))
#define FIX_GATE_POST_ALIGNMENT(more_data)	(more_data | 0x1)
#define	GATE_POST_ALIGNED	(0x1)
#define GATE_POST_CHECKED	(0x2)
#define GATE_PASSED			(0x1)
#define GATE_DISCOVERED		(0x2)
#define GATE_UNPASSED		(0xFFFE)
#define GATE_UNCHECKED		(0xFFFD)
//ext_dat bitflag orientation for GATE_RING: GATE_R
// 15 <- pop
// 14-12 <- pattern is "0x3000" for gate ring (single-object gate)
// 11-8 <- TRACK # [all gates in a TRACK share this #]
// 7-4 <- unused
// 3-2 <- first or last gate flag (1 for first gate, 2 for last gate, 0 for all else) (patterns 0x4 first, 0x8 last)
// 1 <- Unused
// 0 <- Will be 1 if this gate is passed
//ext_dat bitflag orientation for COLLISIONLESS:
// 15 <- pop
// 14-12 <- pattern is "0x5000" for collisionless entity
// all else unused
//
// You know it would also be fun to pick up a flag and, by any path neccessary, deliver it to a point.

#define TRACK_DISCOVERED (0x2)
#define TRACK_COMPLETE (0x1)
#define TRACK_ACTIVE	(0x8000)
#define TRACK_INACTIVE	(0x7FFF)
/**
//////////////////////////////////////////////////////////////////
	TRACK_DATA orientation
	_sobject
		entity_ID :
			0-3 :  Track # selection
		radius[xyz] (empty)
		ext_dat :
			15 : Active boolean (0 for inactive, 1 for active)
			14-12 : 0x4000, specifies level data.
			11-8 : 0x100, specifies track data (this entry).
			7-4 : Track fail speed setting (0 for no fail speed)
			3-0 : Track timer setting (0 for no fail time)
	_declaredObject
		pix[XY] :
			[X] - the number of gates in the track series that have been passed (for an active track).
			[Y] - the total # of gates in the track series
		more_data : 
			0 : Track completion boolean (if 1, track is complete)
			1 : Discovered boolean (if 1, all gates have been discovered)
		dist :
			Silent discovery timer. Counts-down from X seconds when discovered, then enables track.
		link 
			declared object array entry of another level change
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
	SOUND_TRIG orientation:
	_sobject
		ext_dat 
			15 : Boolean. If this sound has triggered yet, or not. ("popped")
			14-12 : "LDATA" definition bits.
			8-11 : "SOUND_TRIG" definition bits.
			7-5 : nothing
			4 : Stream or from-memory bitflag. 
				If bit 4 is low, the sound is treated as an ADX stream, and is selected from a special ADX stream list.
				If bit 4 is high, the sound is played from the on-hand PCM list on the driver.
			0-3 : Presently unused.
	_declaredObject
		pos[xyz]
			Location of the trigger
		rot[xyz]
			The radius of the trigger (in whole-number sizes e.g. "1" is approximately 1<<16 ... as per the declared function)
		more_data
			7 - 0 : The sound number intended to play back.
			11 - 8 : The volume of the sound; directly related to the volume sent to the driver (shift right eight).
		link 
			declared object array entry of another level change
**/


typedef struct {
	unsigned char entity_ID;
	unsigned char clone_ID;
	unsigned short radius[XYZ];
	unsigned short ext_dat;
	unsigned short light_bright;
	unsigned short light_y_offset;
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
extern unsigned short objNEW;
extern unsigned short objDRAW[MAX_WOBJS];
extern unsigned short activeObjects[MAX_WOBJS];
extern _buildingObject * BuildingPayload; //In LWRAM
extern short link_starts[8];
extern int total_building_payload;
extern int objUP;

void	fill_obj_list(void);

void	declare_object_at_cell(short pixX, short height, short pixY, int type, ANGLE xrot, ANGLE yrot, ANGLE zrot, short more_data);

void	declarations(void);

void	update_object(Uint8 boxNumber, int pixX, int pixY, FIXED Ydist, ANGLE rotx, ANGLE roty, ANGLE rotz, FIXED radx, FIXED rady, FIXED radz);

void	object_control_loop(int ppos[XY]);

void	light_control_loop(void);

void	has_entity_passed_between(short obj_id1, short obj_id2, _boundBox * tgt);
void	walk_map_between_objects(short obj_id1, short obj_id2);

void	run_item_collision(int index, _boundBox * tgt);

void	test_gate_ring(int index, _boundBox * tgt);
void	test_gate_posts(int index, _boundBox * tgt);

void	gate_track_manager(void);

//cleaned out

#endif 

