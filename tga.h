///tga.h

#ifndef __TGA_H__
# define __TGA_H__

#define LWRAM	(2097152)
#define	HIMEM	(100679680)
#include "jo/jo.h"
#include "render.h"

extern unsigned int * cRAM_24bm;
extern unsigned short * cRAM_16bm;


extern unsigned char * GLOBAL_img_addr;
extern short GLOBAL_img_line_count;
extern short GLOBAL_img_line_width;
extern int numTex;

void	get_file_in_memory(Sint8 * filename, void * destination);
bool	WRAP_NewPalette(Sint8 * filename, void * file_start);
bool	WRAP_NewTexture(Sint8 * filename, void * file_start);
bool	WRAP_NewTable(Sint8 * filename, void * file_start, int tex_height);

#endif

