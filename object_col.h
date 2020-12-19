#ifndef __OBJECT_COL_H__
# define __OBJECT_COL_H__

#include <jo/jo.h>
#include "def.h"
#include "bounder.h"
#include "collision.h"
#include "mymath.h"

void	per_poly_collide(PDATA * mesh, POINT mesh_position, _boundBox * mover);
void	plane_rendering_with_subdivision(PDATA * mesh, POINT mesh_position);

#endif

