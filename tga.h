///tga.h

#ifndef __TGA_H__
# define __TGA_H__

#define LWRAM	(2097152)
#define	HIMEM	(100679680)
#include "jo/jo.h"

extern unsigned int * cRAM_24bm;
extern unsigned short * cRAM_16bm;


extern unsigned short * GLOBAL_img_addr;
extern short GLOBAL_img_line_count;
extern short GLOBAL_img_line_width;

void	get_file_in_memory(Sint8 * filename, void * destination);
bool	read_tga_in_memory(void * file_start);
void	set_tga_as_palette(void);
bool	read_pco_in_memory(void * file_start);
bool	WRAP_NewPalette(Sint8 * filename, void * file_start);
bool	WRAP_NewTexture(Sint8 * filename, void * file_start);
bool	WRAP_NewTable(Sint8 * filename, void * file_start, int tex_height);

#endif

