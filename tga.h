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

void	get_file_in_memory(Sint8 * filename, void * destination);
Bool	set_tga_to_sprite_palette(void * file_start);


void	color_offset_vdp1_palette(int colorCode, int * run_only_once);
void	restore_vdp1_palette(void);

void	add_texture_to_vram(int width, int height);

int		new_dithered_texture(int texno_a, int texno_b, short replaced_texture);
void	make_4way_combined_textures(int start_texture_number, int end_texture_number, short replacement_start);
void	make_combined_textures(int texture_number);
Bool	read_pco_in_memory(void * file_start);
int		read_tex_table_in_memory(void * file_start, int tex_height);
void	ReplaceTextureTable(void * file_start, int tex_height, int first_replaced_texno);
Bool	WRAP_NewPalette(Sint8 * filename, void * file_start);
Bool	WRAP_NewTexture(Sint8 * filename, void * file_start);
int		WRAP_NewTable(Sint8 * filename, void * file_start, int tex_height);
void	WRAP_ReplaceTable(Sint8 * filename, void * file_start, int tex_height, int first_replaced_texno);

#endif

