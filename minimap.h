//
//minimap.h
//some 2D drawing items

#ifndef __MINIMAP_H__
# define __MINIMAP_H__

extern unsigned char oneLineCt;

void	init_minimap(void);
void	add_position_to_minimap(int xpos, int ypos, unsigned short color);
void	update_mmap_1pass(void);
void	draw_minimap(void);

#endif

