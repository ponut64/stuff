//
//minimap.h
//some 2D drawing items

#ifndef __MINIMAP_H__
# define __MINIMAP_H__

#define MINIMAP_ONE_PIXEL	(0)
#define MINIMAP_X_PATTERN	(1)
#define MINIMAP_P_PATTERN	(2)

extern unsigned char oneLineCt;

void	init_minimap(void);
void	add_position_to_minimap(int xpos, int ypos, unsigned short color, short pattern);
void	update_mmap_1pass(void);
void	draw_minimap(void);

#endif

