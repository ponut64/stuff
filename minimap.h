//
//minimap.h
//some 2D drawing items

#ifndef __MINIMAP_H__
# define __MINIMAP_H__

extern unsigned char oneLineCt;

void	init_minimap(void);
void	add_object_to_minimap(_declaredObject * obj, unsigned short color);
void	update_mmap_1pass(void);
void	draw_minimap(void);

#endif

