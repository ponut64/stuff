//particle.c
//This file compiled separately.

#include <SL_DEF.H>
#include "def.h"
#include "mymath.h"
#include "tga.h"
#include "pcmsys.h"
#include "render.h"
#include "mloader.h"

#include "collision.h"
#include "physobjet.h"

#include "particle.h"


/*


*/
ptypes particle_type_list[MAX_PARTICLE_TYPES];


int sparkTexno;
int puffTexno;
int auraTexno;

_sprite		TestSpr = {
	.lifetime = 3<<16,
	.span[X] = 5,
	.span[Y] = 5,
	.span[Z] = 5,
	.texno = 5,
	.colorBank = 1,
	.useClip = 0,
	.extra = 0,
	.type.info.drawMode = SPRITE_TYPE_BILLBOARD,
	.type.info.drawOnce = 0,
	.type.info.mesh = 1,
	.type.info.sorted = 1,
	.type.info.alive = 1
}; 

_sprite		SmallPuff = {
	.lifetime = 1<<16,
	.span[X] = 1,
	.span[Y] = 1,
	.span[Z] = 1,
	.texno = 0,
	.colorBank = 2,
	.useClip = 0,
	.extra = 0,
	.type.info.drawMode = SPRITE_TYPE_BILLBOARD,
	.type.info.drawOnce = 0,
	.type.info.mesh = 1,
	.type.info.sorted = 1,
	.type.info.alive = 1
}; 

_sprite		GlowPuff = {
	.lifetime = 8192,
	.span[X] = 1,
	.span[Y] = 1,
	.span[Z] = 1,
	.texno = 0,
	.colorBank = 1,
	.useClip = 0,
	.extra = 0,
	.type.info.drawMode = SPRITE_TYPE_BILLBOARD,
	.type.info.drawOnce = 0,
	.type.info.mesh = 1,
	.type.info.sorted = 1,
	.type.info.alive = 1
}; 

_sprite 	HitPuff = {
	.lifetime = 24576,
	.span[X] = 1,
	.span[Y] = 1,
	.span[Z] = 1,
	.texno = 0,
	.colorBank = 0,
	.useClip = 0,
	.extra = 0,
	.type.info.drawMode = SPRITE_TYPE_BILLBOARD,
	.type.info.drawOnce = 0,
	.type.info.mesh = 0,
	.type.info.sorted = 1,
	.type.info.alive = 1
};

_sprite 	ThrowPuff = {
	.lifetime = 24576,
	.span[X] = 16,
	.span[Y] = 4,
	.span[Z] = 16,
	.texno = 0,
	.colorBank = 0,
	.useClip = 0,
	.extra = 0,
	.type.info.drawMode = SPRITE_TYPE_BILLBOARD,
	.type.info.drawOnce = 0,
	.type.info.mesh = 1,
	.type.info.sorted = 1,
	.type.info.alive = 1
};

_sprite		DropPuff = {
	.lifetime = 65536,
	.span[X] = 1,
	.span[Y] = 1,
	.span[Z] = 1,
	.texno = 0,
	.colorBank = 0,
	.useClip = 0,
	.extra = 0,
	.type.info.drawMode = SPRITE_TYPE_BILLBOARD,
	.type.info.drawOnce = 0,
	.type.info.mesh = 1,
	.type.info.sorted = 1,
	.type.info.alive = 1
}; 

_sprite		HopPuff = {
	.lifetime = 32768,
	.span[X] = 1,
	.span[Y] = 1,
	.span[Z] = 1,
	.texno = 0,
	.colorBank = 2,
	.useClip = 0,
	.extra = 0,
	.type.info.drawMode = SPRITE_TYPE_BILLBOARD,
	.type.info.drawOnce = 0,
	.type.info.mesh = 0,
	.type.info.sorted = 1,
	.type.info.alive = 1
}; 

_particle	particles[MAX_SPRITES];

void	init_particle(void)
{
	HopPuff.texno = puffTexno;
	DropPuff.texno = auraTexno;
	HitPuff.texno = sparkTexno;
	GlowPuff.texno = sparkTexno;
	SmallPuff.texno = puffTexno;
	ThrowPuff.texno = puffTexno;
	
	particle_type_list[PARTICLE_TYPE_NORMAL].info.gravity = 1;
	particle_type_list[PARTICLE_TYPE_NORMAL].info.collide = 1;
	particle_type_list[PARTICLE_TYPE_NORMAL].info.bounce = 1;
	
	particle_type_list[PARTICLE_TYPE_NOGRAV].info.gravity = 0;
	particle_type_list[PARTICLE_TYPE_NOGRAV].info.collide = 1;
	particle_type_list[PARTICLE_TYPE_NOGRAV].info.bounce = 1;
	
	particle_type_list[PARTICLE_TYPE_NOCOL].info.gravity = 1;
	particle_type_list[PARTICLE_TYPE_NOCOL].info.collide = 0;
	particle_type_list[PARTICLE_TYPE_NOCOL].info.bounce = 1;
	
	particle_type_list[PARTICLE_TYPE_GHOST].info.gravity = 0;
	particle_type_list[PARTICLE_TYPE_GHOST].info.collide = 0;
	particle_type_list[PARTICLE_TYPE_GHOST].info.bounce = 1;
	
	particle_type_list[PROJ_TEST].info.gravity = 0;
	particle_type_list[PROJ_TEST].info.collide = 1;
	particle_type_list[PROJ_TEST].info.bounce = 0;
	particle_type_list[PROJ_TEST].info.damage = 15;
	
}

_particle *	spawn_particle(_sprite * spr_type, unsigned short p_type, int * pos, int * velocity)
{
	
	//Particles are co-related with the sprite system, since they draw with the sprite list.
	//Because of this, whether a particle spawns or not is determined first by whether there's a free sprite entry.
	short spr_entry = add_to_sprite_list(pos, spr_type->span, spr_type->texno, spr_type->colorBank,
	spr_type->type, spr_type->useClip, spr_type->lifetime);
	
	if(spr_entry == -1) return &particles[MAX_SPRITES-1];
	
	particles[spr_entry].spr = &sprWorkList[spr_entry];
	particles[spr_entry].lifetime = spr_type->lifetime;
	particles[spr_entry].type = particle_type_list[p_type];
	particles[spr_entry].velocity[X] = velocity[X];
	particles[spr_entry].velocity[Y] = velocity[Y];
	particles[spr_entry].velocity[Z] = velocity[Z];
	particles[spr_entry].prevPos[X] = spr_type->pos[X];
	particles[spr_entry].prevPos[Y] = spr_type->pos[Y];
	particles[spr_entry].prevPos[Z] = spr_type->pos[Z];
	return &particles[spr_entry];
}

//Emits particles of a type with random velocities/directions from a radius
void	emit_particle_explosion(_sprite * spr_type, unsigned short p_type, int * pos, int * inertia, int radius, int intensity, int count)
{
	for(int i = 0; i < count; i++)
	{
		int partVelocity[XYZ];
		partVelocity[X] = fxm(getRandom(), intensity);
		partVelocity[Y] = fxm(getRandom(), intensity);
		partVelocity[Z] = fxm(getRandom(), intensity);
		_particle * part = spawn_particle(spr_type, p_type, pos, partVelocity);
		int newTime = (fxm(getRandom(), spr_type->lifetime) + spr_type->lifetime)>>1;
		part->lifetime = newTime;
		part->spr->lifetime = newTime;
		if(radius > 65535)
		{
		int final_radius = (radius + fxm(radius, getRandom()))>>1;
		quick_normalize(part->velocity, part->dirUV);
		part->spr->pos[X] += fxm(part->dirUV[X], final_radius);
		part->spr->pos[Y] += fxm(part->dirUV[Y], final_radius);
		part->spr->pos[Z] += fxm(part->dirUV[Z], final_radius);
		part->velocity[X] += inertia[X];
		part->velocity[Y] += inertia[Y];
		part->velocity[Z] += inertia[Z];
		}
	}
}

// Mostly particle effect processor, but also arbitrates other effects e.g. scale
void	object_effects(int obj_index, int box_index)
{
	//If no effect is selected, return.
	static _declaredObject * obj;
	static _boundBox * box;
	obj = &dWorldObjects[obj_index];
	box = &RBBs[box_index];
	switch(obj->type.effect)
	{
		case(EFFECT_NONE):
			return;
			break;
		case(EFFECT_SPARKLE):
			if(obj->type.effectTimeCount > obj->type.effectTimeLimit)
			{
			emit_particle_explosion(&GlowPuff, PARTICLE_TYPE_GHOST, obj->pos, zPt, 12<<16, 8192, 2);
			obj->type.effectTimeCount = 0;
			}
			obj->type.effectTimeCount += delta_time;
			return;
			break;
		case(EFFECT_SHRINK):
			if(obj->type.effectTimeCount <= obj->type.effectTimeLimit)
			{
				obj->type.effectTimeCount += delta_time;
				int timeScale = fxdiv(1<<16, obj->type.effectTimeLimit);
				int shrinkAmt = (1<<16) - fxm(timeScale, obj->type.effectTimeCount);
				set_box_scale(box, shrinkAmt, shrinkAmt, shrinkAmt);
			} else {
				set_box_scale(box, 1, 1, 1);
			}
			return;
			break;
		case(EFFECT_GROW):
			if(obj->type.effectTimeCount <= obj->type.effectTimeLimit)
			{
				obj->type.effectTimeCount += delta_time;
				int timeScale = fxdiv(1<<16, obj->type.effectTimeLimit);
				int shrinkAmt = fxm(timeScale, obj->type.effectTimeCount);
				set_box_scale(box, shrinkAmt, shrinkAmt, shrinkAmt);
			}
			return;
		case(EFFECT_THROW_PARTICLES):
		{
			if(obj->type.effectTimeCount > obj->type.effectTimeLimit)
			{
			emit_particle_explosion(&ThrowPuff, PARTICLE_TYPE_GHOST, obj->pos, box->UVNZ, box->radius[Z], 32768, 2);
			obj->type.effectTimeCount = 0;
			}
			obj->type.effectTimeCount += delta_time;
		}
			break;
	}
	
}

void	object_particle_collision_handler(_particle * part, int bound_box_entry)
{
	_declaredObject * obj = &dWorldObjects[activeObjects[bound_box_entry]];
	unsigned short * edata = &obj->type.ext_dat;
	unsigned short otype = GET_OBJECT_TYPE(*edata);
	
	int health = 0;
	switch(otype)
	{
		case(OBJECT_DESTRUCTIBLE):
		health = (*edata & DESTRUCTIBLE_HEALTH)>>1;
		
		health-= part->type.info.damage;
		health = (health < 0) ? 0 : health;
		health<<=1;
		*edata &= 0x7FF0;
		*edata |= health;
		if(health <= 0)
		{
			*edata |= OBJECT_DISABLED;
		}
		break;
		default:
		break;
	}
	
}

void	particle_collision_handler(_particle * part, int * normal)
{
	
	int deflectionFactor = -fxdot(part->velocity, normal);
		
	part->velocity[X] += fxm(normal[X], deflectionFactor + REBOUND_ELASTICITY);// - (normal[X]>>4);
	part->velocity[Y] += fxm(normal[Y], deflectionFactor + REBOUND_ELASTICITY);// - (normal[Y]>>4);
	part->velocity[Z] += fxm(normal[Z], deflectionFactor + REBOUND_ELASTICITY);// - (normal[Z]>>4);
	//Small push to secure surface release
	part->spr->pos[X] += normal[X]>>4;
	part->spr->pos[Y] += normal[Y]>>4;
	part->spr->pos[Z] += normal[Z]>>4;
	
	part->type.info.garbage = (part->type.info.bounce) ? 0 : 1;
	
	//part->spr->pos[X] = part->prevPos[X];
	//part->spr->pos[Y] = part->prevPos[Y];
	//part->spr->pos[Z] = part->prevPos[Z];
	
}

short	particle_collide_polygon(entity_t * ent, int * ent_pos, _particle * part)
{
	
	//If the entity is not loaded, cease the test.
	if(ent->file_done != true) return false;

	GVPLY * mesh = ent->pol;
	
	//Distance Culling Check
	//Note the entity radius is 16-bit, have to shift it.
	int bigRadius = JO_MAX(JO_MAX(ent->radius[X], ent->radius[Y]), ent->radius[Z])<<16;
		
	static int discard_vector[XYZ];
	static int plane_center[XYZ];
	static int plane_points[4][XYZ];
	static int hitPt[XYZ];
	static int dominant_axis = N_Yp;
	discard_vector[X] = ent_pos[X] - part->spr->pos[X];
	discard_vector[Y] = ent_pos[Y] - part->spr->pos[Y];
	discard_vector[Z] = ent_pos[Z] - part->spr->pos[Z];
	
	int bigDif = JO_MAX(JO_MAX(JO_ABS(discard_vector[X]), JO_ABS(discard_vector[Y])), JO_ABS(discard_vector[Z]));

	//slPrintFX(bigRadius, slLocate(1, 5));
	//slPrintFX(bigDif, slLocate(1, 6));

	if(bigDif > (bigRadius + (20<<16))) return false;
	
	int usedSpan;
	if(part->spr->type.info.drawMode != SPRITE_TYPE_3DLINE)
	{
		usedSpan = part->spr->span[X];
	} else {
		usedSpan = 5;
	}
	for(unsigned int i = 0; i < mesh->nbPolygon; i++)
	{
		//First, back-facing.
		//First we have to match the vector spaces for the particle and for the mesh.
		//Then we have to get to the vector space of a particular polygon.
		//After that point, we can use a dot-product to check on which side of the polygon we are.
		int * plpt = mesh->pntbl[mesh->pltbl[i].vertices[0]];
		int * plnm = mesh->nmtbl[i];
		discard_vector[X] = (plpt[X] + ent_pos[X]) - part->spr->pos[X];
		discard_vector[Y] = (plpt[Y] + ent_pos[Y]) - part->spr->pos[Y];
		discard_vector[Z] = (plpt[Z] + ent_pos[Z]) - part->spr->pos[Z];
		
		//If the dot product is negative, I've determined that to mean:
		//1. The current tested position is behind the plane, and
		//2. The current tested normal is facing away
		if(fxdot(discard_vector, plnm) < 0) continue;
		//////////////////////////////////////////////////////////////
		// Grab the dominant axis
		//////////////////////////////////////////////////////////////
		dominant_axis = mesh->maxtbl[i];
		//////////////////////////////////////////////////////////////
		// Add the position of the mesh to the position of its points
		// PDATA vector space is inverted, so we negate them
		// "Get world-space point position"
		//////////////////////////////////////////////////////////////
		plane_center[X] = 0;
		plane_center[Y] = 0;
		plane_center[Z] = 0;
		for(int u = 0; u < 4; u++)
		{
		plane_points[u][X] = (mesh->pntbl[mesh->pltbl[i].vertices[u]][X] + ent_pos[X] ); 
		plane_points[u][Y] = (mesh->pntbl[mesh->pltbl[i].vertices[u]][Y] + ent_pos[Y] ); 
		plane_points[u][Z] = (mesh->pntbl[mesh->pltbl[i].vertices[u]][Z] + ent_pos[Z] );
		//Add to the plane's center
		plane_center[X] += plane_points[u][X];
		plane_center[Y] += plane_points[u][Y];
		plane_center[Z] += plane_points[u][Z];
		}
		//Divide sum of plane points by 4 to average all the points
		plane_center[X] >>=2;
		plane_center[Y] >>=2;
		plane_center[Z] >>=2;
		
		int segPt[XYZ] = {part->spr->pos[X] - fxm(part->velocity[X], time_fixed_scale),
		part->spr->pos[Y] - fxm(part->velocity[Y], time_fixed_scale),
		part->spr->pos[Z] - fxm(part->velocity[Z], time_fixed_scale)};
		//Yes, the line method is truly better than dot product tests.
		//Though part of how it's better is that the collision check can include velocity (in the line segment).
		//You do that so you can catch things before they just casually phase through by virtue of going fast.
		int hitLine = line_hit_plane_here(part->spr->pos, segPt, plane_center, plnm, zPt, 16384 + (usedSpan<<16), hitPt);

		if(hitLine)
		{
			if(edge_wind_test(plane_points[0], plane_points[1], plane_points[2], plane_points[3], hitPt, dominant_axis, 12))
			{
				particle_collision_handler(part, plnm);
				return true;
			}
		}
	//Loop end stub
	}
	
	
	return false;
}


short	particle_collide_object(_particle * part, int bound_box_entry)
{
	_boundBox * obj = &RBBs[bound_box_entry];
	//Box Populated Check
	if(obj->status[1] != 'C')
	{
		return false;
	}

	//Box Distance Culling Check
	int bigRadius = JO_MAX(JO_MAX(obj->radius[X], obj->radius[Y]), obj->radius[Z]);
		
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
	
	
	int usedSpan;
	if(part->spr->type.info.drawMode != SPRITE_TYPE_3DLINE)
	{
		usedSpan = part->spr->span[X];
	} else {
		usedSpan = 5;
	}
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
		
		int segPt[XYZ] = {part->spr->pos[X] - fxm(part->velocity[X], time_fixed_scale),
		part->spr->pos[Y] - fxm(part->velocity[Y], time_fixed_scale),
		part->spr->pos[Z] - fxm(part->velocity[Z], time_fixed_scale)};
		
		int hitLine = line_hit_plane_here(part->spr->pos, segPt, obj->cftbl[i], obj->nmtbl[i], obj->pos, 16384 + (usedSpan<<16), hitPt);

		if(hitLine)
		{
			if(edge_wind_test(obj->pltbl[i][0], obj->pltbl[i][1], obj->pltbl[i][2], obj->pltbl[i][3], hitPt, boxDisField[i], 12))
			{
				particle_collision_handler(part, obj->nmtbl[i]);
				object_particle_collision_handler(part, bound_box_entry);
				return true;
			} 
		}
	}
	return false;
}

void	operate_particles(void)
{
	short pHit = false;
	for(int i = 0; i < MAX_SPRITES; i++)
	{
		//Particle is dead or empty, stop.
		if(particles[i].lifetime < 0 || particles[i].type.info.garbage)
		{
			particles[i].type.info.garbage = 1;
			particles[i].lifetime = -1;
			particles[i].spr->lifetime = -1;
			continue;
		}
		
		if(particles[i].type.info.collide)
		{
				//Used for collisions
				quick_normalize(particles[i].velocity, particles[i].dirUV);
				pHit = false;
				for(int u = 0; u < MAX_PHYS_PROXY; u++)
				{
					unsigned short edata = dWorldObjects[activeObjects[u]].type.ext_dat;
					unsigned short boxtype = edata & (0xF000);
					if(RBBs[u].status[1] != 'C') continue;
					if(entities[dWorldObjects[activeObjects[u]].type.entity_ID].type == MODEL_TYPE_BUILDING)
					{
						pHit |= particle_collide_polygon(&entities[dWorldObjects[activeObjects[u]].type.entity_ID], RBBs[u].pos, &particles[i]);
					} else if(boxtype == (OBJECT | OBJPOP) || boxtype == SPAWNER)
					{
						pHit |= particle_collide_object(&particles[i], u);
					}
				}
				
				if(particles[i].spr->type.info.drawMode == SPRITE_TYPE_3DLINE)
				{
					particles[i].spr->span[X] = particles[i].dirUV[X]>>1;
					particles[i].spr->span[Y] = particles[i].dirUV[Y]>>1;
					particles[i].spr->span[Z] = particles[i].dirUV[Z]>>1;
				}
				
		} else if(particles[i].spr->type.info.drawMode == SPRITE_TYPE_3DLINE)
		{
			//Exception: If it's a line, we still need this data to display it.
			quick_normalize(particles[i].velocity, particles[i].dirUV);
			//Set the data
			particles[i].spr->span[X] = particles[i].dirUV[X]>>1;
			particles[i].spr->span[Y] = particles[i].dirUV[Y]>>1;
			particles[i].spr->span[Z] = particles[i].dirUV[Z]>>1;
		}
		
		if(particles[i].type.info.gravity)
		{
			particles[i].velocity[Y] += fxm(GRAVITY, time_fixed_scale);
		}
		
		if(pHit == false)
		{
		particles[i].prevPos[X] = particles[i].spr->pos[X];
		particles[i].prevPos[Y] = particles[i].spr->pos[Y];
		particles[i].prevPos[Z] = particles[i].spr->pos[Z];
		}
		particles[i].spr->pos[X] += fxm(particles[i].velocity[X], time_fixed_scale);
		particles[i].spr->pos[Y] += fxm(particles[i].velocity[Y], time_fixed_scale);
		particles[i].spr->pos[Z] += fxm(particles[i].velocity[Z], time_fixed_scale);
		particles[i].lifetime -= delta_time;
	}
	
	
}

