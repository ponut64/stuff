#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "bounder.h"

#define HIT_TOLERANCE (6553)
#define REBOUND_ELASTICITY (0x8000)

typedef struct {
	FIXED xp0[XYZ];
	FIXED xp1[XYZ];
	FIXED yp0[XYZ];
	FIXED yp1[XYZ];
	FIXED zp0[XYZ];
	FIXED zp1[XYZ];
} _lineTable;

extern int boxDisField[6];

void	init_box_handling(void);
int		edge_wind_test(POINT plane_p0, POINT plane_p1, POINT test_pt, int discard, short shift);
Bool	simple_collide(FIXED pos[XYZ], _boundBox * targetBox);
void	standing_surface_alignment(FIXED * unitNormal);
void	finalize_alignment(_boundBox * fmtx);
Bool	player_collide_boxes(_boundBox * stator, _boundBox * mover, short obj_type_data);
void	player_collision_test_loop(void);

#endif

