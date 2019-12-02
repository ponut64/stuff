#ifndef __PHYSOBJET_H__
# define __PHYSOBJET_H__

#include "jo/jo.h"
#include "def.h"
#include "mymath.h"
#include "bounder.h"
#include "collision.h"

typedef struct {
	unsigned short entity_ID;
	unsigned short radius[XYZ];
} _sobject;

typedef struct {
	short pix[XY];
	_sobject type;
	ANGLE srot[XYZ];
	short height;
	unsigned short ext_dat;
} _declaredObject;


extern unsigned char objNEW;
extern unsigned char objDRAW[256];

void	declare_object_at_cell(short pixX, short pixY, _sobject type, ANGLE xrot, ANGLE yrot, ANGLE zrot, short height);

void	update_object(Uint8 boxNumber, int pixX, int pixY, FIXED Ydist, ANGLE rotx, ANGLE roty, ANGLE rotz, FIXED radx, FIXED rady, FIXED radz);

void	object_control_loop(int ppos[XY]);

void	declarations(void);
bool	has_entity_passed_between(short obj_id1, short obj_id2, _boundBox * tgt);
void	walk_map_between_objects(short obj_id1, short obj_id2);

bool	player_collide_boxes(_boundBox * stator, _boundBox * mover);
void	player_collision_test_loop(void);

//cleaned out

#endif 

