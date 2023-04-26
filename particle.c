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
	
	// if((ymin - (20<<16) - JO_ABS(part->velocity[Y])) > part->spr->pos[Y]) return;
	
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
	
	int deflectionFactor = 0;
//	slPrintFX(dotTri1, slLocate(0, 11));
//	slPrintFX(dotTri2, slLocate(0, 12));
//	slPrintFX(quad->verts[0][Z], slLocate(0, 13));
	
	//If triangle 1 is farther away than triangle 2, collide with triangle 2.
	//If triangle 1 is closer, collide with triangle 1.
	if(dotTri2 > 8192 && tri1Dist >= tri2Dist)
	{
		deflectionFactor = fxdot(part->velocity, tri2Norm);
			
		part->velocity[X] -= fxm(tri2Norm[X], deflectionFactor + REBOUND_ELASTICITY); 
		part->velocity[Y] -= fxm(tri2Norm[Y], deflectionFactor + REBOUND_ELASTICITY); 
		part->velocity[Z] -= fxm(tri2Norm[Z], deflectionFactor + REBOUND_ELASTICITY); 
		
		
	} else if(dotTri1 > 8192 && tri1Dist < tri2Dist)
	{
		deflectionFactor = fxdot(part->velocity, tri1Norm);
			
		part->velocity[X] -= fxm(tri1Norm[X], deflectionFactor + REBOUND_ELASTICITY); 
		part->velocity[Y] -= fxm(tri1Norm[Y], deflectionFactor + REBOUND_ELASTICITY); 
		part->velocity[Z] -= fxm(tri1Norm[Z], deflectionFactor + REBOUND_ELASTICITY); 
	}
	
	//(if we collided, deflection factor will be non-zero)
	if(deflectionFactor)
	{
		part->spr->pos[X] = part->lastPos[X];
		part->spr->pos[Y] = part->lastPos[Y];
		part->spr->pos[Z] = part->lastPos[Z];
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
		}
		
		particle_collide_heightmap(&particles[i]);
		
		particles[i].lastPos[X] = particles[i].spr->pos[X];
		particles[i].lastPos[Y] = particles[i].spr->pos[Y];
		particles[i].lastPos[Z] = particles[i].spr->pos[Z];
		particles[i].spr->pos[X] += fxm(particles[i].velocity[X], frmul);
		particles[i].spr->pos[Y] += fxm(particles[i].velocity[Y], frmul);
		particles[i].spr->pos[Z] += fxm(particles[i].velocity[Z], frmul);
		particles[i].lifetime -= delta_time;
	}
	
	
}

