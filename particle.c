//particle.c
//This file compiled separately.

#include <SL_DEF.H>
#include "def.h"
#include "mymath.h"
#include "tga.h"
#include "pcmsys.h"
#include "render.h"
#include "bounder.h"
#include "collision.h"
#include "physobjet.h"
#include "hmap.h"

#include "particle.h"

//I guess I need a particle type list to start with too, don't I?

_sprite		pentry; 
_particle	particle_starter = {
	.spr = &pentry
	};
_particle	particles[MAX_SPRITES];

void	spawn_particle(_particle * part)
{
	
	//Particles are co-related with the sprite system, since they draw with the sprite list.
	//Because of this, whether a particle spawns or not is determined first by whether there's a free sprite entry.
	short spr_entry = add_to_sprite_list(part->spr->pos, part->spr->span, part->spr->texno, part->spr->colorBank,
	part->spr->mesh, 'B', part->spr->useClip, part->spr->lifetime);
	
	if(spr_entry == -1) return;
	
	particles[spr_entry] = *part;
	particles[spr_entry].spr = &sprWorkList[spr_entry];
	particles[spr_entry].lifetime = part->spr->lifetime;
}

void	particle_collision_handler(_particle * part, int * normal)
{
	
	int deflectionFactor = fxdot(part->velocity, normal);
		
	part->velocity[X] -= fxm(normal[X], deflectionFactor + REBOUND_ELASTICITY); 
	part->velocity[Y] -= fxm(normal[Y], deflectionFactor + REBOUND_ELASTICITY); 
	part->velocity[Z] -= fxm(normal[Z], deflectionFactor + REBOUND_ELASTICITY); 
	
	part->spr->pos[X] = part->prevPos[X];
	part->spr->pos[Y] = part->prevPos[Y];
	part->spr->pos[Z] = part->prevPos[Z];
	
}


short	particle_collide_object(_particle * part, _boundBox * obj)
{
	
	//Box Populated Check
	if(obj->status[1] != 'C')
	{
		return false;
	}

	//Box Distance Culling Check
	int bigRadius = JO_MAX(JO_MAX(obj->brad[X], obj->brad[Y]), obj->brad[Z]);
		
	int centerDif[XYZ];
	centerDif[X] = obj->pos[X] - part->spr->pos[X];
	centerDif[Y] = obj->pos[Y] - part->spr->pos[Y];
	centerDif[Z] = obj->pos[Z] - part->spr->pos[Z];
	
	int bigDif = JO_MAX(JO_MAX(JO_ABS(centerDif[X]), JO_ABS(centerDif[Y])), JO_ABS(centerDif[Z]));

	//slPrintFX(bigRadius, slLocate(1, 7));
	//slPrintFX(bigDif, slLocate(1, 8));

	if(bigDif > (bigRadius + (20<<16))) return false;
	
	if(obj->status[3] != 'B')
	{
		finalize_collision_proxy(obj);
	}
	
	static int hitPt[XYZ];
	/*
	Collision Logic Lesson
	In case someone 20 years from now, After The Bombs Fall decides to read this code for some silly reason...
	Collision and collision response (physics) logic needs to generate three basic things.
	1. Resolve a possible point of collision.
	It does not matter if you are testing a point, you must resolve collision to a point.
	Ergo, the collision math has to generate a new point which is potentially the point of contact.
	2. Test the point of collision with BOTH objects.
	Again, even if you are testing just a single point, you have to test the point of collision against it somehow.
	A single point is not valid for testing collision by itself.
	If you are testing collision on a point, it is likely that point was moving.
	So you can use its last position to get two points, and thus a line/segment/direction.
	The reason a single point is not valid because it does not have enough information to tell you about the collision.
	It could only report that there was one.
	The point of collision has to be tested with both objects and satisfy the collision rule for both objects.
	3. A normal for collision must be found or calculated.
	There is nothing useful about a physical collision unless a normal of contact is found or calculated.
	Your objects usually have normals around the collision point. So try to search around there.
	*/
	for(int i = 0; i < 6; i++)
	{
   		//Backfacing Faces
		if(fxdot(centerDif, obj->nmtbl[i]) > 0) continue;
		
		int hitline = line_hit_plane_here(part->spr->pos, part->prevPos, obj->cftbl[i], obj->nmtbl[i], obj->pos, 16384, hitPt);

		if(hitline)
		{
			if(edge_wind_test(obj->pltbl[i][0], obj->pltbl[i][1], hitPt, boxDisField[i]) > 0)
			{
				if(edge_wind_test(obj->pltbl[i][1], obj->pltbl[i][2], hitPt, boxDisField[i]) > 0)
				{
					if(edge_wind_test(obj->pltbl[i][2], obj->pltbl[i][3], hitPt, boxDisField[i]) > 0)
					{
						if(edge_wind_test(obj->pltbl[i][3], obj->pltbl[i][0], hitPt, boxDisField[i]) > 0)
						{
							particle_collision_handler(part, obj->nmtbl[i]);
							return true;
						}
					}
				}
			} 
		}
	}
	return false;
}

void	particle_collide_heightmap(_particle * part)
{
	
	static _pquad floor;
	
	//Negated position.
	//Why? Fuck man, I don't know. It just works this way. Coordinate system mayhem.
	int npk[3] = {-part->spr->pos[X], -part->spr->pos[Y], -part->spr->pos[Z]};
	
	generate_cell_from_position(npk, &floor);
	
	int ymin = JO_MIN(JO_MIN(floor.verts[0][Y], floor.verts[1][Y]), JO_MIN(floor.verts[2][Y], floor.verts[3][Y]));
	//If we are too far above the floor, return.
	
//	slPrintFX(part->spr->pos[Y], slLocate(1, 11));
//	slPrintFX(ymin, slLocate(1, 8));
	
	if((ymin - (20<<16) - JO_ABS(part->velocity[Y])) > part->spr->pos[Y]) return;
	
	static int tri1CF[XYZ];
	static int tri1Norm[XYZ];
	static int tri2CF[XYZ];
	static int tri2Norm[XYZ];
	
	divide_cell_return_cfnorms(&floor, tri1CF, tri1Norm, tri2CF, tri2Norm);
	
	int tri1Dist =  slSquartFX(fxm(floor.verts[1][X] + npk[X], floor.verts[1][X] + npk[X])
	+ fxm(floor.verts[1][Z] + npk[Z], floor.verts[1][Z] + npk[Z]));
	
	int tri2Dist =  slSquartFX(fxm(floor.verts[3][X] + npk[X], floor.verts[3][X] + npk[X])
	+ fxm(floor.verts[3][Z] + npk[Z], floor.verts[3][Z] + npk[Z]));
	
	int dotTri1 = realpt_to_plane(npk, tri1Norm, tri1CF);
	int dotTri2 = realpt_to_plane(npk, tri2Norm, tri2CF);
	

//	slPrintFX(dotTri1, slLocate(0, 11));
//	slPrintFX(dotTri2, slLocate(0, 12));
//	slPrintFX(quad->verts[0][Z], slLocate(0, 13));
	
	//If triangle 1 is farther away than triangle 2, collide with triangle 2.
	//If triangle 1 is closer, collide with triangle 1.
	if(dotTri2 > 8192 && tri1Dist >= tri2Dist)
	{
		particle_collision_handler(part, tri2Norm);
	} else if(dotTri1 > 8192 && tri1Dist < tri2Dist)
	{
		particle_collision_handler(part, tri1Norm);
	}
	
	
}

void	operate_particles(void)
{
	
	for(int i = 0; i < MAX_SPRITES; i++)
	{
		//Particle is dead or empty, stop.
		if(particles[i].lifetime < 0 || particles[i].type == PARTICLE_TYPE_EMPTY)
		{
			particles[i].type = PARTICLE_TYPE_EMPTY;
			continue;
		}
		
		if(particles[i].type == PARTICLE_TYPE_NORMAL || particles[i].type == PARTICLE_TYPE_NOCOL)
		{
			particles[i].velocity[Y] += fxm(GRAVITY, frmul);
			particle_collide_heightmap(&particles[i]);
			
			for(int u = 0; u < MAX_PHYS_PROXY; u++)
			{
				unsigned short edata = dWorldObjects[activeObjects[u]].type.ext_dat;
				unsigned short boxtype = edata & (0xF000);
				if(RBBs[u].status[1] != 'C') continue;
				if(boxtype == (OBJECT | OBJPOP))
				{
					particle_collide_object(&particles[i], &RBBs[u]);
					
					// slPrintFX(RBBs[u].cftbl[1][X], slLocate(9,  8));
					// slPrintFX(RBBs[u].cftbl[1][Y], slLocate(19, 8));
					// slPrintFX(RBBs[u].cftbl[1][Z], slLocate(29, 8));
					
					// slPrintFX(RBBs[u].cftbl[2][X], slLocate(9,  9));
					// slPrintFX(RBBs[u].cftbl[2][Y], slLocate(19, 9));
					// slPrintFX(RBBs[u].cftbl[2][Z], slLocate(29, 9));
					
					// slPrintFX(RBBs[u].cftbl[3][X], slLocate(9,  10));
					// slPrintFX(RBBs[u].cftbl[3][Y], slLocate(19, 10));
					// slPrintFX(RBBs[u].cftbl[3][Z], slLocate(29, 10));
					
				}
			}
		}
		
		particles[i].prevPos[X] = particles[i].spr->pos[X];
		particles[i].prevPos[Y] = particles[i].spr->pos[Y];
		particles[i].prevPos[Z] = particles[i].spr->pos[Z];
		particles[i].spr->pos[X] += fxm(particles[i].velocity[X], frmul);
		particles[i].spr->pos[Y] += fxm(particles[i].velocity[Y], frmul);
		particles[i].spr->pos[Z] += fxm(particles[i].velocity[Z], frmul);
		particles[i].lifetime -= delta_time;
	}
	
	
}

