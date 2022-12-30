#ifndef __PLAYER_PHY_H__
# define __PLAYER_PHY_H__

void	reset_player(void);
void	player_phys_affect(void);
void	collideBoxes(_boundBox * boxD);
void	collide_with_heightmap(_boundBox * sbox);

#endif

