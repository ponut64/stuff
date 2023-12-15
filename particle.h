#pragma once

#define PARTICLE_TYPE_EMPTY		(0) //Not a particle
#define PARTICLE_TYPE_NORMAL	(1)	//Gravity-enabled, collision-enabled particle.
#define PARTICLE_TYPE_NOGRAV	(2)	//Gravity-disabled, collision-enabled
#define PARTICLE_TYPE_GHOST		(3)	//Gravity-disabled, collision-disabled
#define PARTICLE_TYPE_NOCOL		(4) //Gravity-enabled, collision-disabled
#define PARTICLE_HIT			(0x8000) //Bitflag to set when particle has hit something

#define	EFFECT_NONE				(0)
#define EFFECT_SPARKLE			(1)
#define EFFECT_SHRINK			(2)
#define EFFECT_GROW				(3)
#define EFFECT_THROW_PARTICLES	(4)

typedef struct {
	_sprite * spr;
	int prevPos[XYZ];
	int velocity[XYZ];
	int dirUV[XYZ];
	int spd;
	int lifetime;
	unsigned short luma; //Light emission value (unused)
	unsigned short type;
	unsigned short extra; //Type-specific data
} _particle;

extern int sparkTexno;
extern int puffTexno;
extern int auraTexno;

extern _sprite		TestSpr; 
extern _sprite		SmallPuff;
extern _sprite		GlowPuff;
extern _sprite		HitPuff;
extern _sprite		DropPuff;
extern _sprite		HopPuff;	
extern _particle	particle_starter;
extern _particle	particles[MAX_SPRITES];

void		init_particle(void);
_particle *	spawn_particle(_sprite * spr_type, unsigned short p_type, int * pos, int * velocity);
void		emit_particle_explosion(_sprite * spr_type, unsigned short p_type, int * pos, int * inertia, int radius, int intensity, int count);
void		player_sliding_particles(void);
void		object_effects(int obj_index, int box_index);
void		particle_collision_handler(_particle * part, int * normal);
void		operate_particles(void);


