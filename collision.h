#pragma once



#define HIT_TOLERANCE (6553)
#define REBOUND_ELASTICITY (0x8000)

extern int boxDisField[6];

void	init_box_handling(void);
int		edge_projection_test(int * pp0, int * pp1, int * pp2, int * pp3, _lineTable * boxAxis, _boundBox * box, int discard);
int		edge_wind_test(int * pp0, int * pp1, int * pp2, int * pp3, int * tpt, int discard, short shift);
Bool	simple_collide(FIXED pos[XYZ], _boundBox * targetBox);
void	standing_surface_alignment(FIXED * unitNormal);
void	finalize_alignment(_boundBox * fmtx);
int		broad_phase_sector_finder(int * pos, int * mesh_position, _sector * test_sector);
int		hitscan_vector_from_position_box(int * ray_normal, int * ray_pos, int * hit, int * hitNormal, _boundBox * box);
void	player_collision_test_loop(void);

//player_phy
void	reset_player(void);
void	player_phys_affect(void);
void	collideBoxes(_boundBox * boxD);
void	collide_with_heightmap(_boundBox * sbox, _lineTable * moverCFs, _lineTable * moverTimeAxis);

//object_col lision
void	purge_rotated_entities(void);
void	generate_rotated_entity_for_object(short declared_object_entry);
int		hitscan_vector_from_position_building(int * ray_normal, int * ray_pos, int * hit, int * hitPolyID, entity_t * ent, int * mesh_position, _sector * sct);
void *	buildAdjacentSectorList(int entity_id, void * workAddress);
void	collide_in_sector_of_entity(entity_t * ent, _sector * sct, _boundBox * mover, _lineTable * realTimeAxis);
void	collide_per_polygon_of_mesh(entity_t * ent, _boundBox * mover, _lineTable * realTimeAxis);

