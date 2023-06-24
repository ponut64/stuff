#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "bounder.h"

#define HIT_TOLERANCE (6553)
#define REBOUND_ELASTICITY (0x8000)

extern int boxDisField[6];

void	init_box_handling(void);
int		edge_wind_test(POINT plane_p0, POINT plane_p1, POINT test_pt, int discard, short shift);
Bool	simple_collide(FIXED pos[XYZ], _boundBox * targetBox);
void	standing_surface_alignment(FIXED * unitNormal);
void	finalize_alignment(_boundBox * fmtx);
void	player_collision_test_loop(void);

#endif

