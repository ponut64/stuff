#ifndef __PHYSOBJET_H__
# define __PHYSOBJET_H__

#include "jo/jo.h"
#include "def.h"
#include "mymath.h"
#include "bounder.h"
#include "collision.h"
#include "ldata.h"

#define OBJPOP	(0x8000) //Is populated?
#define UNPOP	(0x7FFF) //Unpop
#define OBJECT	(0x0000) //Normal entity
#define ITEM	(0x1000) //Collectible item
#define GATE_P	(0x2000) //Gate post [has collision]
#define GATE_R	(0x3000) //Ring-type gate entity
#define LDATA	(0x4000) //Level data definition
#define GHOST	(0x5000) //Entity, but no collision (ghost)

#define LDATA_TRACK (0x100) //Level data, gate data definition

#define MAX_WOBJS (257)

//ext_dat bitflag orientation for ITEM:
// 15 <- pop
// 14-12 <- Pattern is "0x1000" for ITEM
// 11-4 <- ?
// 3 	<- Root entity bitflag [booleans]
// 2-0 <- ?
//ext_dat bitflag orientation for NORMAL:
// 15 <- pop
// 14-12 <- pattern is "0 0 0" for normal
// all else unused
//ext_dat bitflag orientation for GATE POST:
// 15 <- pop
// 14-12 <- pattern is "0x2000" for gate post objects
// 11-8 <- TRACK # [all gates in a series share this #]
// 7-4 <- Link specification [gates in the same TRACK that share this # form the two sides of a gate]
// 3-2 <- first or last gate flag (1 for first gate, 2 for last gate, 0 for all else) (patterns 0x4 first, 0x8 last)
// 1 <- Will be 1 if the post was checked this loop
// 0 <- Will have *any* data if this gate is passed
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
// 0x100 for TRACK DATA.
//TRACK data orientation:
//ext_dat
// 4-7: TRACK fail speed setting (if player ever goes lower than this set speed, the gates reset)
// 3-0: TRACK timer setting (if player takes longer than this setting to get between gates, the gates reset)
//entity_ID : 0-3: TRACK select. In other words, this TRACK data is used for this TRACK.
//pix[X] : Passed # of gates in the series
//pix[Y] : total # of gates in the series
//height : bit 15 is TRACK COMPLETE!
//

typedef struct {
	unsigned short entity_ID;
	unsigned short radius[XYZ];
	unsigned short ext_dat;
} _sobject;

typedef struct {
	short pix[XY];
	_sobject type;
	ANGLE srot[XYZ];
	short height;
	short link;
	int dist; //Just *some* static data to keep for every object because so many need it, but not all do.
	unsigned char status; //Done, in-view, passed, in process, never know
} _declaredObject;

//extern _declaredObject dWorldObjects[257];
extern _declaredObject * dWorldObjects; //In LWRAM - see lwram.c
extern unsigned short objNEW;
extern unsigned short objDRAW[512];
extern unsigned short activeObjects[512];
extern short link_starts[8];

void	fill_obj_list(void);

void	declare_object_at_cell(short pixX, short pixY, int type, ANGLE xrot, ANGLE yrot, ANGLE zrot, char height);

void	declarations(void);

void	update_object(Uint8 boxNumber, int pixX, int pixY, FIXED Ydist, ANGLE rotx, ANGLE roty, ANGLE rotz, FIXED radx, FIXED rady, FIXED radz);

void	object_control_loop(int ppos[XY]);

void	has_entity_passed_between(short obj_id1, short obj_id2, _boundBox * tgt);
void	walk_map_between_objects(short obj_id1, short obj_id2);

void	run_item_collision(int index, _boundBox * tgt);

void	test_gate_ring(int index, _boundBox * tgt);
void	test_gate_posts(int index, _boundBox * tgt);

void	gate_track_manager(void);

//cleaned out

#endif 

