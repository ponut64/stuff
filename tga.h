///tga.h

#ifndef __TGA_H__
# define __TGA_H__

extern unsigned int * cRAM_24bm;
extern unsigned short * cRAM_16bm;

extern unsigned char * sprPalette;
extern unsigned int sprPaletteCopy[256];

extern unsigned char * GLOBAL_img_addr;
extern short GLOBAL_img_line_count;
extern short GLOBAL_img_line_width;
extern int numTex;

void	color_offset_vdp1_palette(int colorCode, int * run_only_once);
void	restore_vdp1_palette(void);

int		new_dithered_texture(int texno_a, int texno_b);
void	make_4way_combined_textures(int start_texture_number, int end_texture_number);
void	make_combined_textures(int texture_number);
void	add_texture_to_vram(int width, int height);
void	get_file_in_memory(Sint8 * filename, void * destination);
Bool	WRAP_NewPalette(Sint8 * filename, void * file_start);
Bool	WRAP_NewTexture(Sint8 * filename, void * file_start);
int		WRAP_NewTable(Sint8 * filename, void * file_start, int tex_height);

#endif

