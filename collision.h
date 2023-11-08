#pragma once

#include "bounder.h"

#define HIT_TOLERANCE (6553)
#define REBOUND_ELASTICITY (0x8000)

extern int boxDisField[6];

void	init_box_handling(void);
int		edge_wind_test(int * pp0, int * pp1, int * pp2, int * pp3, int * tpt, int discard, short shift);
Bool	simple_collide(FIXED pos[XYZ], _boundBox * targetBox);
void	standing_surface_alignment(FIXED * unitNormal);
void	finalize_alignment(_boundBox * fmtx);
void	player_collision_test_loop(void);

//player_phy
void	reset_player(void);
void	player_phys_affect(void);
void	collideBoxes(_boundBox * boxD);
void	collide_with_heightmap(_boundBox * sbox, _lineTable * moverCFs, _lineTable * moverTimeAxis);

//object_col lision
void	purge_rotated_entities(void);
void	generate_rotated_entity_for_object(short declared_object_entry);
void	per_poly_collide(entity_t * ent, _boundBox * mover, FIXED * mesh_position, _lineTable * moverCFs, _lineTable * moverTimeAxis);


