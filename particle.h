#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#define PARTICLE_TYPE_EMPTY		(0) //Not a particle
#define PARTICLE_TYPE_NORMAL	(1)	//Gravity-enabled, collision-enabled particle.
#define PARTICLE_TYPE_NOGRAV	(2)	//Gravity-disabled, collision-enabled
#define PARTICLE_TYPE_GHOST		(3)	//Gravity-disabled, collision-disabled
#define PARTICLE_TYPE_NOCOL		(4) //Gravity-enabled, collision-disabled

typedef struct {
	_sprite * spr;
	int prevPos[XYZ];
	int velocity[XYZ];
	int lifetime;
	unsigned short luma; //Light emission value
	unsigned short type;
	unsigned short extra; //Type-specific data
} _particle;

extern _sprite		pentry; 
extern _particle	particle_starter;
extern _particle	particles[MAX_SPRITES];

void	particle_collision_handler(_particle * part, int * normal);
void	spawn_particle(_particle * part);
void	operate_particles(void);

#endif
