///tga.h

#pragma once

#define TGA_HEADER_GAP	(18)
#define SPRITE_PALETTE_OFFSET	(256)
#define BG_PALETTE_OFFSET		(512)
#define HUD_PALETTE_OFFSET		(0)

extern unsigned char * sprPalette;
extern unsigned int sprPaletteCopy[256];

extern unsigned char * GLOBAL_img_addr;
extern short GLOBAL_img_line_count;
extern short GLOBAL_img_line_width;
extern int numTex;

void	get_file_in_memory(Sint8 * filename, void * destination);
void	set_tga_to_sprite_palette(void * file_start);
void	set_tga_to_nbg1_palette(void * file_start);
void	set_8bpp_tga_to_nbg0_image(Sint32 fid, void * buffer);

void	uv_cut(void * file_start, int wx, int yh);
void	uv_tile(void * source_texture_data, int base_x, int base_y);

void	generate_downscale_texture(int img_x, int img_y, int out_img_x, int out_img_y, unsigned char * img);
void	replace_downscale_texture(int large_texno, int scaled_texno);

void	color_offset_vdp1_palette(int colorCode, int * run_only_once);
void	restore_vdp1_palette(void);

void	add_texture_to_vram(int width, int height);

int		new_dithered_texture(int texno_a, int texno_b, int replaced_texture);
void	make_4way_combined_textures(int start_texture_number, int end_texture_number, int replacement_start);
Bool	read_pco_in_memory(void * file_start);
int		read_tex_table_in_memory(void * file_start, int tex_height);
void	ReplaceTextureTable(void * file_start, int tex_height, int first_replaced_texno);
void	WRAP_NewPalette(Sint8 * filename, void * file_start);
Bool	WRAP_NewTexture(Sint8 * filename, void * file_start);
int		WRAP_NewTable(Sint8 * filename, void * file_start, int tex_height);
void	WRAP_ReplaceTable(Sint8 * filename, void * file_start, int tex_height, int first_replaced_texno);

