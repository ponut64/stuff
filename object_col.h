#ifndef __OBJECT_COL_H__
#define __OBJECT_COL_H__

void	generate_rotated_entity_for_object(short declared_object_entry);
int		edge_wind_test(POINT plane_p0, POINT plane_p1, POINT test_pt, int discard);
void	per_poly_collide(entity_t * ent, _boundBox * mover, FIXED * mesh_position);

#endif

