#ifndef __OBJECT_COL_H__
# define __OBJECT_COL_H__

#include <jo/jo.h>
#include "def.h"
#include "bounder.h"
#include "collision.h"
#include "mymath.h"

void	per_poly_collide(entity_t * ent, _boundBox * mover, FIXED * mesh_position);
void	plane_rendering_with_subdivision(entity_t * ent);

#endif

