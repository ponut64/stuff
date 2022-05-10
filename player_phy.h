#ifndef __PLAYER_PHY_H__
# define __PLAYER_PHY_H__

#include <jo/jo.h>
#include "def.h"
#include "draw.h"
#include "bounder.h"
#include "mymath.h"
#include "physobjet.h"
#include "hmap.h"

extern POINT alwaysLow;

void	player_phys_affect(void);
void	collideBoxes(_boundBox * boxD);
void	collide_with_heightmap(_boundBox * sbox);

#endif

