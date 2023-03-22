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

#define BUILD_ORIGINAL_ENTITY_ID (0xFF0)
/*
///////////////////////////////////////////////////////////////
	ext_dat bitflag orientation for BUILD:
		15 <- pop
		14-12 <- will be 0x6 (1 1 0) for BUILD
		11 - 4 <- Permutation series definition. Has the entity ID of the mesh (in the entity list) which this object is a permutation of.
		If there is no permutation, this otherwise has the entity ID of the building object. 
		3 - 0 <- unused
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
			0 <- Will have *any* data if this gate is passed
		more_data 
			0 <- will be high if the post has been aligned with the other post, or if the user does not want it aligned.
*/
#define SET_GATE_POST_LINK(ext_dat, gate_num) (ext_dat | (gate_num << 4))
#define SET_GATE_TRACK_NUM(ext_dat, track_num) (ext_dat | (track_num << 8))
#define FIX_GATE_POST_ALIGNMENT(more_data)	(more_data | 0x1)
//ext_dat bitflag orientation for GATE_RING: GATE_R
// 15 <- pop
// 14-12 <- pattern is "0x3000" for gate ring (single-object gate)
// 11-8 <- TRACK # [all gates in a TRACK share this #]
// 7-4 <- unused
// 3-2 <- first or last gate flag (1 for first gate, 2 for last gate, 0 for all else) (patterns 0x4 first, 0x8 last)
// 1 <- Unused
// 0 <- Will have *any* data if this gate is passed
//ext_dat bitflag orientation for COLLISIONLESS:
// 15 <- pop
// 14-12 <- pattern is "0x5000" for collisionless entity
// all else unused
//
// You know it would also be fun to pick up a flag and, by any path neccessary, deliver it to a point.
// Octree objects maybe

//ext_dat bitflag orientation for LEVEL DATA:
//15 <- "1" if track is active. "0" if track is inactive.
//14-12 <- "0x4000" for level data specification
//11-8 <- Specifications beneath the LEVEL_DATA tree.
// 0x100 for TRACK_DATA
// 0x200 for LEVEL_CHNG
//////////////////////////////////////////////////////////////////
//TRACK_DATA orientation:
//ext_dat
//	4-7: TRACK fail speed setting (if player ever goes lower than this set speed, the gates reset)
//	3-0: TRACK timer setting (if player takes longer than this setting to get between gates, the gates reset)
//entity_ID :
//	0-3: TRACK select. In other words, this TRACK data is used for this TRACK.
//pix[X] : Passed # of gates in the series
//pix[Y] : total # of gates in the series
//more_data : bit 15 is TRACK COMPLETE!
//////////////////////////////////////////////////////////////////
/**
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
			delcared object array entry of another level change
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
**/


typedef struct {
	unsigned short entity_ID;
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
	short	more_data;
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

