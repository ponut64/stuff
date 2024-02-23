#pragma once

#define MAX_PARTICLE_TYPES		(64)
#define PARTICLE_TYPE_NORMAL	(0)	//Gravity-enabled, collision-enabled particle.
#define PARTICLE_TYPE_NOGRAV	(1)	//Gravity-disabled, collision-enabled
#define PARTICLE_TYPE_NOCOL		(2) //Gravity-enabled, collision-disabled
#define PARTICLE_TYPE_GHOST		(3)	//Gravity-disabled, collision-disabled
#define PROJ_TEST	(4)

///////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////
// Whacky system to less-than-manually pack named data into 16 bits.
///////////////////////////////////////////////////////
typedef struct {
	union {
		unsigned short raw;
		
		struct {
		unsigned char garbage:1;
		unsigned char gravity:1;
		unsigned char collide:1;
		unsigned char bounce:1;
		unsigned char damage:4;
		} info;
	};
} ptypes;

extern ptypes particle_type_list[MAX_PARTICLE_TYPES];

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
	int lifetime;
	unsigned short luma; //Light emission value (unused)
	ptypes type;
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


