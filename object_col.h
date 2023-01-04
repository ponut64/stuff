#ifndef __OBJECT_COL_H__
#define __OBJECT_COL_H__

void	generate_rotated_entity_for_object(short declared_object_entry);

void	per_poly_collide(entity_t * ent, _boundBox * mover, FIXED * mesh_position);
void	plane_rendering_with_subdivision(entity_t * ent);

#endif

