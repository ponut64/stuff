//

#include <sl_def.h>
#include <SEGA_GFS.H>
#include "def.h"
#include "render.h" 
#include "vdp2.h"
#include "hmap.h"
#include "mymath.h"

#include "tga.h"

unsigned char * GLOBAL_img_addr = (unsigned char *)LWRAM;
unsigned char * curVRAMptr = (unsigned char*)(VDP1_VRAM + VRAM_TEXTURE_BASE); //see render.h

unsigned int sprPaletteCopy[256];

int numTex = 0;

int cutTex = 0;

void	get_file_in_memory(Sint8 * filename, void * destination)
{

	GfsHn gfs_tga;
	Sint32 sector_count;
	Sint32 file_size;
	
	Sint32 local_name = GFS_NameToId(filename);

//Open GFS
	gfs_tga = GFS_Open((Sint32)local_name);
//Get sectors
	GFS_GetFileSize(gfs_tga, NULL, &sector_count, NULL);
	GFS_GetFileInfo(gfs_tga, NULL, NULL, &file_size, NULL);
	
	GFS_Close(gfs_tga);
	
	GFS_Load(local_name, 0, (Uint32 *)destination, file_size);

}
	
void	set_tga_to_sprite_palette(void * file_start)
{
	unsigned char * readByte = (unsigned char *)file_start;
	unsigned short * readWord = (unsigned short *)file_start;
	
	unsigned char id_field_size = readByte[0];
	
	unsigned char col_map_type = readByte[1]; 
	
	if(col_map_type != 0){
		slPrint("(REJECTED NON-RGB TGA)", slLocate(0,0));
		return;
	}
	unsigned char data_type = readByte[2];
	
	if(data_type != 2) {
		slPrint("(REJECTED RLE TGA)", slLocate(0,0));
		return;
	}
	//Color Map Specification Data is ignored.
	
	//X / Y origin data is ignored.
	
	//unsigned short xSizeLoBits = readByte[12]; //unused
	//unsigned short ySizeLoBits = readByte[14]; //unused, because the has an assumed size 
	
	unsigned char bpp = readByte[16];
	
	if(bpp != 24){
		slPrint("(TGA NOT 24BPP)", slLocate(0,0));
		return; //File Typing Demands 24 bpp.
	}
	
	//Descriptor Bytes are skipped.
	
	int imdat = id_field_size + TGA_HEADER_GAP;
	
	GLOBAL_img_addr = (unsigned char*)((int)readWord + imdat);
	
	readByte = GLOBAL_img_addr;
	unsigned char component[XYZ] = {0, 0, 0}; //Actually "R, G, B"
	unsigned int final_color = 0;

	for(int i = 0; i < 256; i++){
		component[X] = readByte[(i*3)];
		component[Y] = readByte[(i*3)+1];
		component[Z] = readByte[(i*3)+2];
		
		final_color = (unsigned int)((component[X]<<16) | (component[Y]<<8) | (component[Z]));
		
		cRAM_24bm[i+SPRITE_PALETTE_OFFSET] = (final_color);
		sprPaletteCopy[i] = final_color;
		
	}
		//Set the text color (temporary, depends on library setting)
		//cRAM_24bm[1] = (255<<16) | (255<<8) | (255);
	
	return;
}

void	set_tga_to_nbg1_palette(void * file_start)
{
	unsigned char * readByte = (unsigned char *)file_start;
	unsigned short * readWord = (unsigned short *)file_start;
	
	unsigned char id_field_size = readByte[0];
	
	unsigned char col_map_type = readByte[1]; 
	
	if(col_map_type != 0){
		slPrint("(REJECTED NON-RGB TGA)", slLocate(0,0));
		return;
	}
	unsigned char data_type = readByte[2];
	
	if(data_type != 2) {
		slPrint("(REJECTED RLE TGA)", slLocate(0,0));
		return;
	}
	//Color Map Specification Data is ignored.
	
	//X / Y origin data is ignored.
	
	//unsigned short xSizeLoBits = readByte[12]; //unused
	//unsigned short ySizeLoBits = readByte[14]; //unused, because the has an assumed size 
	
	unsigned char bpp = readByte[16];
	
	if(bpp != 24){
		slPrint("(TGA NOT 24BPP)", slLocate(0,0));
		return; //File Typing Demands 24 bpp.
	}
	
	//Descriptor Bytes are skipped.
	
	int imdat = id_field_size + TGA_HEADER_GAP;
	
	GLOBAL_img_addr = (unsigned char*)((int)readWord + imdat);
	
	readByte = GLOBAL_img_addr;
	unsigned char component[XYZ] = {0, 0, 0}; //Actually "R, G, B"
	unsigned int final_color = 0;

	for(int i = 0; i < 256; i++){
		component[X] = readByte[(i*3)];
		component[Y] = readByte[(i*3)+1];
		component[Z] = readByte[(i*3)+2];
		
		final_color = (unsigned int)((component[X]<<16) | (component[Y]<<8) | (component[Z]));
		
		cRAM_24bm[i+HUD_PALETTE_OFFSET] = (final_color);
		
	}
	
	return;
}

void	set_8bpp_tga_to_nbg0_image(Sint32 fid, void * buffer)
{
	//These files are bigger than our image buffer space (64k vs 256 - 384k).
	//So we need a more clever way to do this.
	// Right now, this assumes the image width is 512, because that's what the hardware expects.
	
	GfsHn gfs_tga;
	Sint32 sector_count;
	Sint32 file_size;

//Open GFS
	gfs_tga = GFS_Open((Sint32)fid);
//Get sectors
	GFS_GetFileSize(gfs_tga, NULL, &sector_count, NULL);
	GFS_GetFileInfo(gfs_tga, NULL, NULL, &file_size, NULL);
	GFS_Close(gfs_tga);
	////////////////////////////////////////////////////////
	// We are going to load the file in 32 sector chunks (64k).
	////////////////////////////////////////////////////////
	int chunk_size_sectors = 16;
	int chunk_size_bytes = 32768;
	int chunk_pixels = chunk_size_bytes;
	int read_sectors = 0;
	////////////////////////////////////////////////////////
	// Perform the initial chunk load
	////////////////////////////////////////////////////////
	GFS_Load(fid, 0, buffer, chunk_size_bytes);
	read_sectors += chunk_size_sectors;

	unsigned char * readByte = (unsigned char *)buffer;
	
	unsigned char id_field_size = readByte[0];
	
	unsigned char col_map_type = readByte[1]; 
	
	if(col_map_type != 1){
		slPrint("(REJECTED RGB TGA)", slLocate(0,1));
		return;
	}

	unsigned char data_type = readByte[2];
	
	if(data_type != 1) {
		slPrint("(REJECTED RLE TGA)", slLocate(0,1));
		return;
	}
	
	unsigned char bpp = readByte[16];
	
	if(bpp != 8) {
		slPrint("(REJECTED >8B TGA)", slLocate(0,1));
		return;
	}
	
	short Twidth = readByte[12] | readByte[13]<<8;
	short Theight = readByte[14] | readByte[15]<<8;
	
	int col_map_size = (readByte[5] | readByte[6]<<8) * 3;
	int col_map_ct = col_map_size / 3;
	
	int initial_chunk_pixels = chunk_size_bytes - id_field_size - col_map_size - TGA_HEADER_GAP;
	
	////////////////////////////////////////////////////////
	//We shall load the 24-bit RGB palette straight from the TGA image.
	//The palette is located after the header and id_field.
	//The palette will be loaded to VDP2 CRAM
	////////////////////////////////////////////////////////
	unsigned char paldat = id_field_size + TGA_HEADER_GAP;
	readByte = buffer + paldat;
	
	unsigned char component[XYZ] = {0, 0, 0}; //Actually "R, G, B"
	unsigned int final_color = 0;

	for(int i = 0; i < (col_map_ct); i++)
	{

		component[X] = readByte[(i*3)];
		component[Y] = readByte[(i*3)+1];
		component[Z] = readByte[(i*3)+2];
		
		final_color = (unsigned int)((component[X]<<16) | (component[Y]<<8) | (component[Z]));
		
		cRAM_24bm[i+BG_PALETTE_OFFSET+1] = (final_color);
	}
	
	//The first color in a bank is transparent color.
	//I can disable it, but for some reason I want to preserve it.
	//In such case, the image will need to have a blank color added as zero, and all colors therefore offset by 1.
	cRAM_24bm[BG_PALETTE_OFFSET] = 0;
	////////////////////////////////////////////////////////
	//Now we're going to load the image to VDP2 VRAM
	//Offset the first chunk read by the TGA header and size of TGA palette
	////////////////////////////////////////////////////////
	readByte  += col_map_size;
	
	unsigned char * image_data = (unsigned char *)NBG0_BITMAP_ADDR;

	int total_pixels = Twidth * Theight;
	int wrote_pixels = 0;
	////////////////////////////////////////////////////////
	// Perform the initial chunk image offload
	// Remember we must add 1 to the color index to make room for the transparent color index
	////////////////////////////////////////////////////////
	for(int i = 0; i < initial_chunk_pixels && wrote_pixels < total_pixels; i++)
	{
		*image_data = readByte[i]+1;
		image_data++;
		wrote_pixels++;
	}
	
	////////////////////////////////////////////////////////
	// Load new chunks, then process their pixels, until we have done all pixels.
	////////////////////////////////////////////////////////
	// readByte no longer needs the offset to skip the TGA header.
	readByte = (unsigned char *)buffer;
	do{
		
		GFS_Load(fid, read_sectors, buffer, chunk_size_bytes);
		read_sectors += chunk_size_sectors;
		
		for(int i = 0; i < chunk_pixels && wrote_pixels < total_pixels; i++)
		{
			*image_data = readByte[i]+1;
			image_data++;
			wrote_pixels++;
		}
		
	}while(wrote_pixels < total_pixels && read_sectors <= sector_count);

}



//Note: colorCode will be signed.
//Use: Add a color code to all of VDP1's colors.
//Also has an "apply once" Boolean that controls color addition (will only apply when 1, and writes back 0 when done).
void	color_offset_vdp1_palette(int colorCode, int * run_only_once)
{
	if(*run_only_once == 0) return;
	int mod_color;
	short red_channel;
	short green_channel;
	short blue_channel;
	short mod_channel;
	int sign = (colorCode > 0) ? 1 : -1;
	int used_color = colorCode * sign;
	for(int i = 0; i < 256; i++)
	{
		mod_color = cRAM_24bm[i+SPRITE_PALETTE_OFFSET];
		
		blue_channel = (mod_color>>16) & 255;
		mod_channel = (used_color>>16) & 255;
		blue_channel = (sign > 0) ? (blue_channel + mod_channel) : (blue_channel - mod_channel);
		blue_channel = (blue_channel < 0) ? 0 : (blue_channel > 255) ? 255 : blue_channel;
		
		green_channel = (mod_color>>8) & 255;
		mod_channel = (used_color>>8) & 255;
		green_channel = (sign > 0) ? (green_channel + mod_channel) : (green_channel - mod_channel);
		green_channel = (green_channel < 0) ? 0 : (green_channel > 255) ? 255 : green_channel;
		
		red_channel = (mod_color) & 255;
		mod_channel = (used_color) & 255;
		red_channel = (sign > 0) ? (red_channel + mod_channel) : (red_channel - mod_channel);
		red_channel = (red_channel < 0) ? 0 : (red_channel > 255) ? 255 : red_channel;
		
		mod_color = (blue_channel << 16) | (green_channel << 8) | red_channel;
		cRAM_24bm[i+SPRITE_PALETTE_OFFSET] = mod_color;
	}
	
	//Also change the back screen color by this setting
	//Note: This is a 16-bit color, not a 24-bit (32 bit) color.
	mod_color = (unsigned short )*back_scrn_colr_addr;
	mod_channel = (used_color>>19) & 31;
	blue_channel = (mod_color>>10) & 31;
	blue_channel = (sign > 0) ? (blue_channel + mod_channel) : (blue_channel - mod_channel);
	blue_channel = (blue_channel < 0) ? 0 : (blue_channel > 31) ? 31 : blue_channel;
	
	mod_channel = (used_color>>11) & 31;
	green_channel = (mod_color>>5) & 31;
	green_channel = (sign > 0) ? (green_channel + mod_channel) : (green_channel - mod_channel);
	green_channel = (green_channel < 0) ? 0 : (green_channel > 31) ? 31 : green_channel;
	
	mod_channel = (used_color>>3) & 31;
	red_channel = (mod_color) & 31;
	red_channel = (sign > 0) ? (red_channel + mod_channel) : (red_channel - mod_channel);
	red_channel = (red_channel < 0) ? 0 : (red_channel > 31) ? 31 : red_channel;
	
	mod_color = (blue_channel<<10) | (green_channel<<5) | (red_channel);
	*back_scrn_colr_addr = (unsigned short)mod_color;
	
	*run_only_once = 0;
}

//Use: Restore VDP1 color palette after modifying it to original condition.
void	restore_vdp1_palette(void)
{
	for(int i = 0; i < 256; i++)
	{
		cRAM_24bm[i+SPRITE_PALETTE_OFFSET] = sprPaletteCopy[i];
	}
	//Also restore back color
	*back_scrn_colr_addr = back_color_setting;
}

void	add_texture_to_vram(int width, int height)
{
	int totalPix = width * height;
	
	/*
	SIZE parameter:
	CMDSIZE of VDP1 Command Table
	15		14		13		12		11		10		9		8		7		6		5		4		3		2		1		0
	N		N	|				Texture width / 8				||						Texture height						|
	13-8: Texture width (in << 3 units)
	7-0: Texture height (in integer units)
	*/
	pcoTexDefs[numTex].SIZE = ((width)<<5) | height; //This table is //In LWRAM - see lwram.c
	slDMACopy((void*)GLOBAL_img_addr, (void*)curVRAMptr, totalPix);
	pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)curVRAMptr); //This table is //In LWRAM - see lwram.c
	curVRAMptr += totalPix;
	numTex++;
}

int		new_dithered_texture(int texno_a, int texno_b, int replaced_texture)
{
	//Core concept: Create a new texture, same size as the both textures which also need to be the same size,
	// with every odd pixel from texture A, and every even pixel from texture B.
	//
	// If "replaced_texture" is not zero, it will replace that texture #, instead of making a new one.
	// If is zero, it will create a new texture.
	//
	
		unsigned char * a_data = (unsigned char *)((unsigned int)(VDP1_VRAM + (pcoTexDefs[texno_a].SRCA<<3)));
		unsigned char * b_data = (unsigned char *)((unsigned int)(VDP1_VRAM + (pcoTexDefs[texno_b].SRCA<<3)));
		
		if(pcoTexDefs[texno_a].SIZE != pcoTexDefs[texno_b].SIZE) return 0; //Unequal texture sizes, exit.
		//Past this point, we will be writing a new texture. So set it up.
		
		int base_x = (pcoTexDefs[texno_a].SIZE & 0x3F00)>>5;
		int base_y = (pcoTexDefs[texno_a].SIZE & 0xFF);
		int total_bytes_of_original_texture = base_x * base_y;
	
		unsigned char * texture_start;
		if(replaced_texture == 0)
		{
		texture_start = curVRAMptr;
		pcoTexDefs[numTex].SIZE = pcoTexDefs[texno_a].SIZE;
		pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)curVRAMptr); 
		curVRAMptr += total_bytes_of_original_texture;
		numTex++;
		} else {
		texture_start = (unsigned char *)(VDP1_VRAM + (pcoTexDefs[replaced_texture].SRCA<<3));
		}
		//This is the 50/50 dithering algorithm.
		//Because all textures will have an even number of pixels per line,
		//we need to switch which pixel receives data from which source texture each line.
		//In doing so, we avoid having bars, and disperse the pixels across the texture such that no X or Y lines form.
			int c = 0;
		for(int py = 0; py < base_y; py++)
		{
			int oddrule = (py & 1) ? 1 : 0;
			for(int px = 0; px < base_x; px++)
			{
				if(oddrule == 1)
				{
					texture_start[c] = (c & 1) ? a_data[c] : b_data[c];
				} else {
					texture_start[c] = (c & 1) ? b_data[c] : a_data[c];
				}
				c++;
			}
		}
	
		//End
		return numTex-1;
}

void	make_4way_combined_textures(int start_texture_number, int end_texture_number, int replacement_start)
{
	// If replacement_start is non-zero, texture replacement will start at that number.
	// If replacement is zero, new textures will be made instead.
	for(int t = start_texture_number; t < end_texture_number; t++)
	{
		unsigned char * source_texture_data = (unsigned char *)((unsigned int)(VDP1_VRAM + (pcoTexDefs[t].SRCA<<3)));
		unsigned char * readByte = source_texture_data;
		int base_x = (pcoTexDefs[t].SIZE & 0x3F00)>>5;
		int base_y = (pcoTexDefs[t].SIZE & 0xFF);
		int total_bytes_of_original_texture = base_x * base_y;
		int readPoint = 0;
		int write_point = 0;
		unsigned char * texture_start;
		if(replacement_start == 0)
		{
		texture_start = curVRAMptr;
		pcoTexDefs[numTex].SIZE = pcoTexDefs[t].SIZE;
		pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)curVRAMptr); 
		curVRAMptr += total_bytes_of_original_texture;
		numTex++;
		} else {
		texture_start = (unsigned char *)(VDP1_VRAM + (pcoTexDefs[replacement_start].SRCA<<3));
		replacement_start++;
		}
		//CORE CONCEPT: Get a copy of the texture, written in (base_x>>1) and (base_y>>1) size.
		//This copy is a downscale.
		//Then copy that to this texture, four times.
		/*
		GETTING DOWNSCALE
		*/
		for(int j = 0; j < (base_y>>1); j++)
		{
			for(int w = 0; w < (base_x>>1); w++)
			{
			readPoint = (j * base_x * 2) + (w * 2);
			dirty_buf[write_point] = readByte[readPoint];
			write_point++;
			}
		}
		/*
		WRITING DOWNSCALE 4 TIMES
		COMPLICATION: We have to write each scanline of the downscaled texture twice.
		COMPLICATION: We have to then write those, twice over!
		*/
		for(int j = 0; j < 2; j++)
		{
			for(int w = 0; w < (base_y>>1); w++)
			{
				for(int v = 0; v < (base_x>>1); v++)
				{
					*texture_start++ = dirty_buf[(w * (base_x>>1)) + v];
				}
				for(int v = 0; v < (base_x>>1); v++)
				{
					*texture_start++ = dirty_buf[(w * (base_x>>1)) + v];
				}
			}
		}
		
	}
}

/// Special note: the source image width is hard-coded as 64 here
void	select_and_cut_from_64xH(int * sel_sz, int * sel_min, unsigned char * img, unsigned char * sel_img)
{

	int pix_write_pt = 0;
	
	static int sel_max[2];
	sel_max[X] = sel_min[X] + sel_sz[X];
	sel_max[Y] = sel_min[Y] + sel_sz[Y];
	
	for(int iy = sel_min[Y]; iy < sel_max[Y]; iy++)
	{
		for(int ix = sel_min[X]; ix < sel_max[X]; ix++)
		{
			int target_pix = ix + (iy * 64);
			sel_img[pix_write_pt] = img[target_pix];
			pix_write_pt++;
		}
	}
	
}

void	generate_downscale_texture(int img_x, int img_y, int out_img_x, int out_img_y, unsigned char * img)
{
	FIXED x_pixel_scale = fxdiv(img_x<<16, out_img_x<<16);
	FIXED y_pixel_scale = fxdiv(img_y<<16, out_img_y<<16);
	int total_out_pix = out_img_x * out_img_y;
	int pix_read_pt = 0;
	
	pcoTexDefs[numTex].SIZE = ((out_img_x<<5) | (out_img_y));
	pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)curVRAMptr); 
	numTex++;
	
	for(int p = 0; p < total_out_pix; p++)
	{
		int numY = p / (out_img_x);
		int numX = p - (numY * (out_img_x));

		int sample_y = (numY * y_pixel_scale)>>16;
		int sample_x = (numX * x_pixel_scale)>>16;
		if(sample_y > img_y) sample_y = img_y;
		if(sample_x > img_x) sample_x = img_x;
		pix_read_pt = (sample_y * img_x) + sample_x;
		*curVRAMptr++ = img[pix_read_pt];
	}
	
}

void	replace_downscale_texture(int large_texno, int scaled_texno)
{
	
	int lrg_x = (pcoTexDefs[large_texno].SIZE & 0x3F00)>>5;
	int lrg_y = (pcoTexDefs[large_texno].SIZE & 0xFF);
	
	int scl_x = (pcoTexDefs[scaled_texno].SIZE & 0x3F00)>>5;
	int scl_y = (pcoTexDefs[scaled_texno].SIZE & 0xFF);
	
	FIXED x_pixel_scale = fxdiv(lrg_x<<16, scl_x<<16);
	FIXED y_pixel_scale = fxdiv(lrg_y<<16, scl_y<<16);
	int total_out_pix = scl_x * scl_y;
	int pix_read_pt = 0;
	
	unsigned char * readByte = (unsigned char *)((unsigned int)(VDP1_VRAM + (pcoTexDefs[large_texno].SRCA<<3)));
	unsigned char * writeByte = (unsigned char *)((unsigned int)(VDP1_VRAM + (pcoTexDefs[scaled_texno].SRCA<<3)));
	
	for(int p = 0; p < total_out_pix; p++)
	{
		int numY = p / (scl_x);
		int numX = p - (numY * (scl_x));

		int sample_y = (numY * y_pixel_scale)>>16;
		int sample_x = (numX * x_pixel_scale)>>16;
		if(sample_y > lrg_y) sample_y = lrg_y;
		if(sample_x > lrg_x) sample_x = lrg_x;
		pix_read_pt = (sample_y * lrg_x) + sample_x;
		*writeByte++ = readByte[pix_read_pt];
	}
	
}

void	uv_tile(void * source_texture_data, int base_x, int base_y)
{
	
	/////////////////////////////////////////////////
	// What this does:
	// It takes a small texture (presumably 8x8) and tiles it up to eight times, and generates textures for that.
	/////////////////////////////////////////////////

	unsigned char * readByte = source_texture_data;
	// nbg_sprintf(5, 10, "bx(%i)", base_x);
	// nbg_sprintf(5, 11, "by(%i)", base_y);
	int tf_sx = base_x<<3;
	int tf_sy = base_y<<3;
//	if(tf_sy < 64) tf_sy = 64;
	int total_full_scale = tf_sx * tf_sy;
	int ssx = 0;
	int ssy = 0;

	/*
	New:
	Write to the dirty_buf, the source texture tiled to x<<3 and y<<3.
	*/
	int read_point = 0;
	
	for(int p = 0; p < total_full_scale; p++)
	{
		int numY = p / (tf_sx);
		int numX = p - (numY * (tf_sx));
		
		int numY_BaseTex = numY % base_y;
		int numX_BaseTex = numX % base_x;
		
		read_point = (numY_BaseTex * base_x) + numX_BaseTex;
		dirty_buf[p] = readByte[read_point];
	}

	int max_axis = 32;
	
	/*
	
	TEXTURE ID MATRIX:
	1	- full size			+ to 6,	| to 2,	- to 5	
	2	- full y, 1/2 x		+ to 7,	| to 3, - to 6
	3	- full y, 1/4 x		+ to 8, | to 4, - to 7
	4	- full y, 1/8 x		NO +,	NO |,	- to 8
	5	- 1/2 y, full x		+ to 10,| to 6, - to 9 
	6	- 1/2 y, 1/2 x		+ to 11,| to 7, - to 10
	7	- 1/2 y, 1/4 x		+ to 12,| to 8, - to 11
	8	- 1/2 y, 1/8 x		NO +,	NO |,	- to 12
	9	- 1/4 y, full x		+ to 14,| to 10,- to 13
	10	- 1/4 y, 1/2 x		+ to 15,| to 11,- to 14
	11	- 1/4 y, 1/4 x		+ to 16,| to 12,- to 15
	12	- 1/4 y, 1/8 x		NO +,	NO |,	- to 16
	13	- 1/8 y, full x		NO +,	| to 14,NO -
	14	- 1/8 y, 1/2 x		NO +,	| to 15,NO -
	15	- 1/8 y, 1/4 x		NO +,	| to 16,NO -
	16	- 1/8 y, 1/8 x		NO FURTHER SUBDIVISION
	*/
	static int img_sz[2];
	static int img_min[2];
	
	unsigned short p_srca[16];
	unsigned short p_size[16];
	int stex = numTex-1;
	
	for(int dsy = 0; dsy < 4; dsy++)
	{
		ssy = tf_sy>>dsy;
		for(int dsx = 0; dsx < 4; dsx++)
		{
			ssx = tf_sx>>dsx;
			int tex_x = (ssx > max_axis) ? max_axis : ssx;
			int tex_y = (ssy > max_axis) ? max_axis : ssy;

			img_sz[X] = ssx;
			img_sz[Y] = ssy;

			img_min[X] = 0;
			img_min[Y] = 0;

			select_and_cut_from_64xH(img_sz, img_min, dirty_buf, buf_map);
			generate_downscale_texture(ssx, ssy, tex_x, tex_y, buf_map);
			int tgt = (dsy * 4) + dsx;
			p_srca[tgt] = pcoTexDefs[numTex-1].SRCA;
			p_size[tgt] = pcoTexDefs[numTex-1].SIZE;
		}
	}

	/*

	Remap:
	0 -> 	1 			+++
	1 -> 	2 & 3 		-++
	2 -> 	6 - 9 		--+
	3 -> 	14 - 21 	---
	4 -> 	4 & 5 		|++
	5 -> 	30 - 33 	++
	6 -> 	34 - 41 	-+
	7 ->	50 - 65 	--
	8 -> 	10 - 13 	||+
	9 -> 	42 - 49 	|+
	10 ->	82 - 97 	+
	11 ->	98 - 129 	-
	12 ->	22 - 29 	|||
	13 ->	66 - 81 	||
	14 ->	130 - 161	|
	15 ->	162 - 225	.
	
	*/
	for(int i = 162; i <= 225; i++)
	{
		pcoTexDefs[stex + i].SRCA = p_srca[15];
		pcoTexDefs[stex + i].SIZE = p_size[15];
	}
	
	for(int i = 130; i <= 161; i++)
	{
		pcoTexDefs[stex + i].SRCA = p_srca[14];
		pcoTexDefs[stex + i].SIZE = p_size[14];
	}
	
	for(int i = 66; i <= 81; i++)
	{
		pcoTexDefs[stex + i].SRCA = p_srca[13];
		pcoTexDefs[stex + i].SIZE = p_size[13];
	}
	
	for(int i = 22; i <= 29; i++)
	{
		pcoTexDefs[stex + i].SRCA = p_srca[12];
		pcoTexDefs[stex + i].SIZE = p_size[12];
	}
	
	for(int i = 98; i <= 129; i++)
	{
		pcoTexDefs[stex + i].SRCA = p_srca[11];
		pcoTexDefs[stex + i].SIZE = p_size[11];
	}
	
	for(int i = 82; i <= 97; i++)
	{
		pcoTexDefs[stex + i].SRCA = p_srca[10];
		pcoTexDefs[stex + i].SIZE = p_size[10];
	}
	
	for(int i = 42; i <= 49; i++)
	{
		pcoTexDefs[stex + i].SRCA = p_srca[9];
		pcoTexDefs[stex + i].SIZE = p_size[9];
	}
	
	for(int i = 10; i <= 13; i++)
	{
		pcoTexDefs[stex + i].SRCA = p_srca[8];
		pcoTexDefs[stex + i].SIZE = p_size[8];
	}
	
	for(int i = 50; i <= 65; i++)
	{
		pcoTexDefs[stex + i].SRCA = p_srca[7];
		pcoTexDefs[stex + i].SIZE = p_size[7];
	}
	
	for(int i = 34; i <= 41; i++)
	{
		pcoTexDefs[stex + i].SRCA = p_srca[6];
		pcoTexDefs[stex + i].SIZE = p_size[6];
	}
	
	for(int i = 30; i <= 33; i++)
	{
		pcoTexDefs[stex + i].SRCA = p_srca[5];
		pcoTexDefs[stex + i].SIZE = p_size[5];
	}
	
	pcoTexDefs[stex + 4].SRCA = p_srca[4];
	pcoTexDefs[stex + 4].SIZE = p_size[4];
	pcoTexDefs[stex + 5].SRCA = p_srca[4];
	pcoTexDefs[stex + 5].SIZE = p_size[4];
	
	for(int i = 14; i <= 21; i++)
	{
		pcoTexDefs[stex + i].SRCA = p_srca[3];
		pcoTexDefs[stex + i].SIZE = p_size[3];
	}
	
	for(int i = 6; i <= 9; i++)
	{
		pcoTexDefs[stex + i].SRCA = p_srca[2];
		pcoTexDefs[stex + i].SIZE = p_size[2];
	}
	
	pcoTexDefs[stex + 2].SRCA = p_srca[1];
	pcoTexDefs[stex + 2].SIZE = p_size[1];
	pcoTexDefs[stex + 3].SRCA = p_srca[1];
	pcoTexDefs[stex + 3].SIZE = p_size[1];
	
	pcoTexDefs[stex + 1].SRCA = p_srca[0];
	pcoTexDefs[stex + 1].SIZE = p_size[0];
	
	numTex += 209;
	
}

void	uv_cut(void * data_start, int wx, int yh)
{
/*
Successfully cuts a 64x64 or 32x32 texture into all of the necessary chunks for subdivision by the three rules:
+, -, and |.

Would be possible to patch in support for arbitrary heights, but width of 64 or 32 is set in stone.

*/
	unsigned char * readByte = data_start;

	unsigned char * used_dirty_buf = dirty_buf;
	unsigned char * second_dirty_buf = (unsigned char *)((unsigned int)dirty_buf + (12 * 1024));

	int img_sz[2] = {0, 0};
	int img_min[2] = {0, 0};
	int tsl = 0;
	int	tkd = 0;
	
	int size_switch = 0;
	if(wx == 64 && yh == 64) size_switch = 1;
	if(wx == 32 && yh == 32) size_switch = 2;
	if(size_switch == 0) return;
	
	if(size_switch == 2)
	{
		//In this case, we need to get a 32x32 texture scaled up to 64x64.
		//This is the second_dirty_buf.
		//Now we have to line-double the 32x32 image.
		for(int y = 0; y < 32; y++)
		{
			for(int x = 0; x < 32; x++)
			{
				second_dirty_buf[tsl] = readByte[x + (y * 32)];
				tsl++;
			}
			for(int x2 = 0; x2 < 32; x2++)
			{
				second_dirty_buf[tsl] = readByte[x2 + (y * 32)];
				tsl++;
			}
		}
		//Then we have to write that new 64x32 image twice.
		tkd = 0;
		for(int y = 0; y < 32; y++)
		{
			for(int x = 0; x < 64; x++)
			{
				second_dirty_buf[tsl] = second_dirty_buf[tkd];
				tsl++;
				tkd++;
			}
		}
		tkd = 0;
		for(int y = 0; y < 32; y++)
		{
			for(int x = 0; x < 64; x++)
			{
				second_dirty_buf[tsl] = second_dirty_buf[tkd];
				tsl++;
				tkd++;
			}
		}
		//Finally, we have to set the new source address.
		readByte = second_dirty_buf;
		
	}
	/* Original, downscaled */
	generate_downscale_texture(64, 64, 32, 32, readByte);

	/* Horizontal halves (32x64) -++	*/
	//for 32x32: use first texture, then repeat
	int swap_texno[32];
	tsl = 0;
	for(int x32 = 0; x32 < 2; x32++)
	{
		if(x32 == 1 && size_switch == 2)
		{
			pcoTexDefs[numTex].SIZE = pcoTexDefs[swap_texno[tsl]].SIZE;
			pcoTexDefs[numTex].SRCA = pcoTexDefs[swap_texno[tsl]].SRCA; 
			numTex++;
		} else {
		img_sz[Y] = 64;
		img_min[Y] = 0;
		
		img_sz[X] = 32;
		img_min[X] = x32 * 32;
		
		swap_texno[x32] = numTex;
		
		select_and_cut_from_64xH(img_sz, img_min, readByte, used_dirty_buf);
		//Cat call: This should output a 32x32, not a 32x64; down-scale to optimize RAM and performance.
		generate_downscale_texture(img_sz[X], img_sz[Y], 32, 32, used_dirty_buf);
		}
	}
	
	/* Vertical halves (64x32) |++	*/
	//for 32x32: use first texture, then repeat
	tsl = 0;
	for(int y32 = 0; y32 < 2; y32++)
	{
		
		if(y32 == 1 && size_switch == 2)
		{
			pcoTexDefs[numTex].SIZE = pcoTexDefs[swap_texno[tsl]].SIZE;
			pcoTexDefs[numTex].SRCA = pcoTexDefs[swap_texno[tsl]].SRCA; 
			numTex++;
		} else {
		
		img_sz[X] = 64;
		img_min[X] = 0;
		
		img_sz[Y] = 32;
		img_min[Y] = y32 * 32;
		
		swap_texno[y32] = numTex;
		
		select_and_cut_from_64xH(img_sz, img_min, readByte, used_dirty_buf);
		//Cat call: This should output a 32x32, not a 64x32; down-scale to optimize RAM and performance.
		generate_downscale_texture(img_sz[X], img_sz[Y], 32, 32, used_dirty_buf);
		}
	}
	

	/* Horizontal quarters (16x64)	--+	*/
	//The texture address of -+ CANNOT be made from these.
	//The reason is because they are down-scaled to 16x32 to optimize VDP1 performance.
	//for 32x32: use only the first two, then repeat

	tsl = 0;
	for(int x16 = 0; x16 < 4; x16++)
	{
		
		if(x16 >= 2 && size_switch == 2)
		{
			pcoTexDefs[numTex].SIZE = pcoTexDefs[swap_texno[tsl]].SIZE;
			pcoTexDefs[numTex].SRCA = pcoTexDefs[swap_texno[tsl]].SRCA; 
			tsl++;
			numTex++;
		} else {
		
		img_sz[Y] = 64;
		img_min[Y] = 0;
		
		img_sz[X] = 16;
		img_min[X] = x16 * 16;
		
		swap_texno[x16] = numTex;
		
		select_and_cut_from_64xH(img_sz, img_min, readByte, used_dirty_buf);
		//Cat call: This should output a 16x32, not a 16x64; down-scale to optimize RAM and performance.
		generate_downscale_texture(img_sz[X], img_sz[Y], 16, 32, used_dirty_buf);
		}
	}
	
	/* Vertical quarters (64x16)	||+	*/
	//for 32x32: use only the first two, then repeat
	tsl = 0;
	for(int y16 = 0; y16 < 4; y16++)
	{
		if(y16 >= 2 && size_switch == 2)
		{
			pcoTexDefs[numTex].SIZE = pcoTexDefs[swap_texno[tsl]].SIZE;
			pcoTexDefs[numTex].SRCA = pcoTexDefs[swap_texno[tsl]].SRCA; 
			tsl++;
			numTex++;
		} else {
		
		img_sz[X] = 64;
		img_min[X] = 0;
		
		img_sz[Y] = 16;
		img_min[Y] = y16 * 16;
		
		swap_texno[y16] = numTex;
		
		select_and_cut_from_64xH(img_sz, img_min, readByte, used_dirty_buf);
		//Cat call: This should output a 32x16, not a 64x16; down-scale to optimize RAM and performance.
		generate_downscale_texture(img_sz[X], img_sz[Y], 32, 16, used_dirty_buf);
		}
	}
	
	/* Vertical eighths (8x64) ---	*/
	//The texture address of -- and - CANNOT be made from these.
	//The reason is because they are down-scaled to 8x32 to optimize VDP1 performance.
	//for 32x32:
	//use only the first four textures, then repeat
	tsl = 0;
	for(int x8 = 0; x8 < 8; x8++)
	{
		if(x8 >= 4 && size_switch == 2)
		{
			pcoTexDefs[numTex].SIZE = pcoTexDefs[swap_texno[tsl]].SIZE;
			pcoTexDefs[numTex].SRCA = pcoTexDefs[swap_texno[tsl]].SRCA; 
			tsl++;
			numTex++;
		} else {
		img_sz[Y] = 64;
		img_min[Y] = 0;
		
		img_sz[X] = 8;
		img_min[X] = x8 * 8;
		
		swap_texno[x8] = numTex;
		
		select_and_cut_from_64xH(img_sz, img_min, readByte, used_dirty_buf);
		//Cat call: This should output a 8x32, not a 8x64; down-scale to optimize RAM and performance.
		generate_downscale_texture(img_sz[X], img_sz[Y], 8, 32, used_dirty_buf);
		}
	}
	
	/* Horizontal eighths (64x8) |||	*/
	//for 32x32:
	// only use the first four textures, then repeat
	tsl = 0;
	for(int y8 = 0; y8 < 8; y8++)
	{
		if(y8 >= 4 && size_switch == 2)
		{
			pcoTexDefs[numTex].SIZE = pcoTexDefs[swap_texno[tsl]].SIZE;
			pcoTexDefs[numTex].SRCA = pcoTexDefs[swap_texno[tsl]].SRCA; 
			tsl++;
			numTex++;
		} else {
		img_sz[X] = 64;
		img_min[X] = 0;
		
		img_sz[Y] = 8;
		img_min[Y] = y8 * 8;
		
		swap_texno[y8] = numTex;
		
		select_and_cut_from_64xH(img_sz, img_min, readByte, used_dirty_buf);
		//Cat call: This should output a 32x8, not a 64x8; down-scale to optimize RAM and performance.
		generate_downscale_texture(img_sz[X], img_sz[Y], 32, 8, used_dirty_buf);
		}
	}
	
	/* Quarters (32x32) ++	*/
	// Order of Quarters:
	// 1	2
	// 3	4
	//for 32x32:
	// 1	1
	// 1	1
	// (only generate one)
	//NOTHING ELSE CAN USE QUARTER_TEXNO!!
	int quarter_texno[4];
	tsl = 0;
	tkd = 0;
	for(int y32 = 0; y32 < 2; y32++)
	{
		img_sz[Y] = 32;
		img_min[Y] = y32 * 32;
		for(int x32 = 0; x32 < 2; x32++)
		{
			if(tsl > 0 && size_switch == 2)
			{
				pcoTexDefs[numTex].SIZE = pcoTexDefs[quarter_texno[0]].SIZE;
				pcoTexDefs[numTex].SRCA = pcoTexDefs[quarter_texno[0]].SRCA; 
				quarter_texno[tsl] = numTex;
				tsl++;
				numTex++;
			} else {
			img_sz[X] = 32;
			img_min[X] = x32 * 32;
			quarter_texno[tsl] = numTex;
			tsl++;
			select_and_cut_from_64xH(img_sz, img_min, readByte, used_dirty_buf);
			generate_downscale_texture(img_sz[X], img_sz[Y], img_sz[X], img_sz[Y], used_dirty_buf);
			}
		}
	}
	
	//From the texture address of the ++ quarters,
	// we should be able to get the addresses of: |+ and ||
	// When shifting the address like this, be aware that the address is in /8 (or <<3) units.
	
	/* horizontal-quarter, Vertical half (16x32) -+ */
	/*
	Order:
	1	2	3	4
	5	6	7	8
	for 32x32:
	1	2	1	2
	1	2	1	2
	*/
	tsl = 0;
	tkd = 0;
	for(int y32 = 0; y32 < 2; y32++)
	{
		img_sz[Y] = 32;
		img_min[Y] = y32 * 32;
		for(int x16 = 0; x16 < 4; x16++)
		{
			img_sz[X] = 16;
			img_min[X] = x16 * 16;
			if(tsl >= 2 && size_switch == 2)
			{
				pcoTexDefs[numTex].SIZE = pcoTexDefs[swap_texno[tkd]].SIZE;
				pcoTexDefs[numTex].SRCA = pcoTexDefs[swap_texno[tkd]].SRCA; 
				tkd = (tkd >= 1) ? 0 : tkd+1;
				numTex++;
			} else {
			swap_texno[tsl] = numTex;
			tsl++;
			select_and_cut_from_64xH(img_sz, img_min, readByte, used_dirty_buf);
			generate_downscale_texture(img_sz[X], img_sz[Y], img_sz[X], img_sz[Y], used_dirty_buf);
			}
		}
	}
	/* Horizontal halves, vertical quarters (32x16) |+ */
	/*
	ORDER:
	1,2 -> Halves of quarter one (top left)
	3,4 -> Halves of quarter two (top right)
	5,6 -> Halves of quarter three (bottom left)
	7,8 -> Halves of quarter four (bottom right)
	
	LAYOUT:
	11	33
	22	44
	55	77
	66	88
	for 32x32:
	1	1
	2	2
	1	1
	2	2
	Just use halves of quarter one.
	This shouldn't need any code adjustments for 32x32.
	*/
	for(int i = 0; i < 4; i++)
	{
		pcoTexDefs[numTex].SIZE = ((32<<5) | (16));
		pcoTexDefs[numTex].SRCA = pcoTexDefs[quarter_texno[i]].SRCA; 
		numTex++;
		//Step ahead half the size of the texture
		pcoTexDefs[numTex].SIZE = ((32<<5) | (16));
		pcoTexDefs[numTex].SRCA = (pcoTexDefs[quarter_texno[i]].SRCA) + (16 * 4); //"4" being "32>>3" 
		numTex++;
	}
	
	//The eighths...
	//The textures of each individual cell can be found in here.
	//NO OTHER SEGMENT CAN USE HORIEGHTHS
	int horieighth_texno[16];
	
	/* Horizontal eights, vertical halves (8x32) -- */
	/*
	1	2	3	4	5	6	7	8
	1	2	3	4	5	6	7	8
	1	2	3	4	5	6	7	8
	1	2	3	4	5	6	7	8
	9	10	11	12	13	14	15	16
	9	10	11	12	13	14	15	16
	9	10	11	12	13	14	15	16
	9	10	11	12	13	14	15	16
	for 32x32:
	1	2	3	4	1	2	3	4
	1	2	3	4	1	2	3	4
	1	2	3	4	1	2	3	4
	1	2	3	4	1	2	3	4
	1	2	3	4	1	2	3	4
	1	2	3	4	1	2	3	4
	1	2	3	4	1	2	3	4
	1	2	3	4	1	2	3	4
	Repeat the top-left quadrant
	*/
	tsl = 0;
	tkd = 0;
	for(int y32 = 0; y32 < 2; y32++)
	{
		img_sz[Y] = 32;
		img_min[Y] = y32 * 32;
		for(int x8 = 0; x8 < 8; x8++)
		{
			img_sz[X] = 8;
			img_min[X] = x8 * 8;
			
			if(tsl >= 4 && size_switch == 2)
			{
				pcoTexDefs[numTex].SIZE = pcoTexDefs[horieighth_texno[tkd]].SIZE;
				pcoTexDefs[numTex].SRCA = pcoTexDefs[horieighth_texno[tkd]].SRCA; 
				horieighth_texno[tsl] = numTex;
				tsl++;
				tkd = (tkd >= 3) ? 0 : tkd+1;
				numTex++;
			} else {
			horieighth_texno[tsl] = numTex;
			tsl++;
			select_and_cut_from_64xH(img_sz, img_min, readByte, used_dirty_buf);
			generate_downscale_texture(img_sz[X], img_sz[Y], img_sz[X], img_sz[Y], used_dirty_buf);
			}
		}
	}
	
	/* Horizontal halves, vertical eighths (32x8) || */
	/*
	ORDER:
	1,2,3,4 -> Vertical fourths of quarter one (top left)
	5,6,7,8 -> Vertical fourths of quarter two (top right)
	9,10,11,12 -> Vertical fourths of quarter three (bottom left)
	13,14,15,16 -> Vertical fourths of quarter four (bottom right)
	LAYOUT:
	1	5
	2	6
	3	7
	4	8
	9	13
	10	14
	11	15
	12	16
	for 32x32:
	1	1
	2	2
	3	3
	4	4
	1	1
	2	2
	3	3
	4	4
	Repeat the texture numbers for the top-left quadrant.
	*/
	for(int i = 0; i < 4; i++)
	{
		pcoTexDefs[numTex].SIZE = ((32<<5) | (8));
		pcoTexDefs[numTex].SRCA = pcoTexDefs[quarter_texno[i]].SRCA; 
		numTex++;
		//Step ahead a quarter the size of the texture
		pcoTexDefs[numTex].SIZE = ((32<<5) | (8));
		pcoTexDefs[numTex].SRCA = (pcoTexDefs[quarter_texno[i]].SRCA) + (8 * 4); //"4" being "32>>3" 
		numTex++;
		//Step ahead a quarter the size of the texture
		pcoTexDefs[numTex].SIZE = ((32<<5) | (8));
		pcoTexDefs[numTex].SRCA = (pcoTexDefs[quarter_texno[i]].SRCA) + (16 * 4); //"4" being "32>>3"	
		numTex++;
		pcoTexDefs[numTex].SIZE = ((32<<5) | (8));
		pcoTexDefs[numTex].SRCA = (pcoTexDefs[quarter_texno[i]].SRCA) + (24 * 4); //"4" being "32>>3"
		numTex++;
	}

	
	//From the texture address of the + quarters,
	// we should be able to get the addresses of: |
	// When shifting the address like this, be aware that the address is in /8 (or <<3) units.
	//NOTHING ELSE CAN USE EIGHTHS TEXNO
	int eighths_texno[16];
			
	/* Eighths  +*/
	// Remember the progression of cutting here.
	// It does not follow chirality.
	// Instead, it goes like this:
	// 1 - 2
	// 3 - 4
	// Thus, it is:
	// 1	2	3	4
	// 5	6	7	8
	// 9	10	11	12
	// 13	14	15	16
	// for 32x32:
	// 1	2	1	2
	// 5	6	5	6
	// 1	2	1	2
	// 5	6	5	6
	// Repeat the textures generated for the top-left quadrant.
	tsl = 0;
	tkd = 0;
	//swap_texno
		if(size_switch == 1)
		{
	for(int y16 = 0; y16 < 4; y16++)
	{
		img_sz[Y] = 16;
		img_min[Y] = (y16 * 16);
		for(int x16 = 0; x16 < 4; x16++)
		{
			img_sz[X] = 16;
			img_min[X] = (x16 * 16);
			eighths_texno[tsl] = numTex;
			tsl++;
			select_and_cut_from_64xH(img_sz, img_min, readByte, used_dirty_buf);
			generate_downscale_texture(img_sz[X], img_sz[Y], img_sz[X], img_sz[Y], used_dirty_buf);
		}
	}
		} else if(size_switch == 2)
		{
		img_sz[Y] = 16;
		img_sz[X] = 16;
		//We need:
		img_min[Y] = (0 * 16);
		img_min[X] = (0 * 16);
		eighths_texno[tsl] = numTex;
		tsl++;
		select_and_cut_from_64xH(img_sz, img_min, readByte, used_dirty_buf);
		generate_downscale_texture(img_sz[X], img_sz[Y], img_sz[X], img_sz[Y], used_dirty_buf);
		img_min[Y] = (0 * 16);
		img_min[X] = (1 * 16);
		eighths_texno[tsl] = numTex;
		tsl++;
		select_and_cut_from_64xH(img_sz, img_min, readByte, used_dirty_buf);
		generate_downscale_texture(img_sz[X], img_sz[Y], img_sz[X], img_sz[Y], used_dirty_buf);
		//Repeat 0
		eighths_texno[tsl] = eighths_texno[0];
		tsl++;
		pcoTexDefs[numTex].SIZE = pcoTexDefs[eighths_texno[0]].SIZE;
		pcoTexDefs[numTex].SRCA = pcoTexDefs[eighths_texno[0]].SRCA; 
		numTex++;
		//Repeat 1
		eighths_texno[tsl] = eighths_texno[1];
		tsl++;
		pcoTexDefs[numTex].SIZE = pcoTexDefs[eighths_texno[1]].SIZE;
		pcoTexDefs[numTex].SRCA = pcoTexDefs[eighths_texno[1]].SRCA; 
		numTex++;
		
		img_min[Y] = (1 * 16);
		img_min[X] = (0 * 16);
		eighths_texno[tsl] = numTex;
		tsl++;
		select_and_cut_from_64xH(img_sz, img_min, readByte, used_dirty_buf);
		generate_downscale_texture(img_sz[X], img_sz[Y], img_sz[X], img_sz[Y], used_dirty_buf);
		img_min[Y] = (1 * 16);
		img_min[X] = (1 * 16);
		eighths_texno[tsl] = numTex;
		tsl++;
		select_and_cut_from_64xH(img_sz, img_min, readByte, used_dirty_buf);
		generate_downscale_texture(img_sz[X], img_sz[Y], img_sz[X], img_sz[Y], used_dirty_buf);
		//Repeat 4 
		eighths_texno[tsl] = eighths_texno[4];
		tsl++;
		pcoTexDefs[numTex].SIZE = pcoTexDefs[eighths_texno[4]].SIZE;
		pcoTexDefs[numTex].SRCA = pcoTexDefs[eighths_texno[4]].SRCA; 
		numTex++;
		//Repeat 5
		eighths_texno[tsl] = eighths_texno[5];
		tsl++;
		pcoTexDefs[numTex].SIZE = pcoTexDefs[eighths_texno[5]].SIZE;
		pcoTexDefs[numTex].SRCA = pcoTexDefs[eighths_texno[5]].SRCA; 
		numTex++;
		
		//Then repeat first 8
		for(int i = 0; i < 8; i++)
		{
			eighths_texno[tsl] = eighths_texno[i];
			tsl++;
			pcoTexDefs[numTex].SIZE = pcoTexDefs[eighths_texno[i]].SIZE;
			pcoTexDefs[numTex].SRCA = pcoTexDefs[eighths_texno[i]].SRCA; 
			numTex++;
		}
			
		}
	
	/* 8x16 - */
	/*
	boy this one's confusing as hell
	1	3	5	7	9	11	13	15
	2	4	6	8	10	12	14	16
	
	17	19	21	23	25	27	29	31
	18	20	22	24	26	28	30	32
	for 32x32:
	1	3	5	7	1	3	5	7
	2	4	6	8	2	4	6	8
	1	3	5	7	1	3	5	7
	2	4	6	8	2	4	6	8
	broken record, but just repeat the texture numbers for the top-left quadrant
	*/
	for(int i = 0; i < 16; i++)
	{
		pcoTexDefs[numTex].SIZE = ((8<<5) | (16));
		pcoTexDefs[numTex].SRCA = pcoTexDefs[horieighth_texno[i]].SRCA; 
		numTex++;
		//Step ahead a quarter the size of the texture
		pcoTexDefs[numTex].SIZE = ((8<<5) | (16));
		pcoTexDefs[numTex].SRCA = (pcoTexDefs[horieighth_texno[i]].SRCA) + (8 * 2); //"2" being "16>>3" 
		numTex++;
	}
	
	/* 16x8 | */
	/*
	ORDER:
	1	3	5	7
	2	4	6	8
	
	9	11	13	15
	10	12	14	16
	
	17	19	21	23
	18	20	22	24
	
	25	27	29	31
	26	28	30	32
	for 32x32:
	1	3	1	3
	2	4   2	4
	9	11	9	11
	10	12	10	12
	
	1	3	1	3
	2	4   2	4
	9	11	9	11
	10	12	10	12
	Broken record, just being careful to repeat the texno's of the top-left quadrant.
	
	*/
	for(int i = 0; i < 16; i++)
	{
		pcoTexDefs[numTex].SIZE = ((16<<5) | (8));
		pcoTexDefs[numTex].SRCA = pcoTexDefs[eighths_texno[i]].SRCA; 
		numTex++;
		//Step ahead a quarter the size of the texture
		pcoTexDefs[numTex].SIZE = ((16<<5) | (8));
		pcoTexDefs[numTex].SRCA = (pcoTexDefs[eighths_texno[i]].SRCA) + (8 * 2); //"2" being "16>3" 
		numTex++;
	}
	
	/* Sixteenths */
	/*
	1	5	9	13	17	21	25	29
	2	6	10	14	18	22	26	30
	3	7	11	15	19	23	27	31
	4	8	12	16	20	24	28	32
	33	37	41	45	49	53	57	61
	34	38	42	46	50	54	58	62
	35	39	43	47	51	55	59	63
	36	40	44	48	52	56	60	64
	for 32x32:
	1	5	9	13	1	5	9	13
	2	6	10	14  2	6	10	14
	3	7	11	15  3	7	11	15
	4	8	12	16  4	8	12	16
	1	5	9	13	1	5	9	13
	2	6	10	14  2	6	10	14
	3	7	11	15  3	7	11	15
	4	8	12	16  4	8	12	16
	
	*/
	for(int i = 0; i < 16; i++)
	{
		pcoTexDefs[numTex].SIZE = ((8<<5) | (8));
		pcoTexDefs[numTex].SRCA = pcoTexDefs[horieighth_texno[i]].SRCA; 
		numTex++;
		//Step ahead a quarter the size of the texture
		pcoTexDefs[numTex].SIZE = ((8<<5) | (8));
		pcoTexDefs[numTex].SRCA = (pcoTexDefs[horieighth_texno[i]].SRCA) + (8 * 1); //"1" being "8>>3" 
		numTex++;
		//Step ahead a quarter the size of the texture
		pcoTexDefs[numTex].SIZE = ((8<<5) | (8));
		pcoTexDefs[numTex].SRCA = (pcoTexDefs[horieighth_texno[i]].SRCA) + (16 * 1); //"1" being "8>>3" 
		numTex++;
		//Step ahead a quarter the size of the texture
		pcoTexDefs[numTex].SIZE = ((8<<5) | (8));
		pcoTexDefs[numTex].SRCA = (pcoTexDefs[horieighth_texno[i]].SRCA) + (24 * 1); //"1" being "8>>3" 
		numTex++;
	}
		
}

/*
Palette Colors --> Supplied by formatted TGA file.

Textures Palette --> Supplied by formatted 64-color palette in GIMP to match TGA palette section for "normal brightness" color.

Texture	--> Color Index Pixels of exported tga file.
*/
Bool read_pco_in_memory(void * file_start)
{
	unsigned char * readByte = (unsigned char *)file_start;
	
	unsigned char id_field_size = readByte[0];
	
	unsigned char col_map_type = readByte[1]; 
	
	if(col_map_type != 1){
		slPrint("(REJECTED RGB TGA)", slLocate(0,0));
		return 0;
	}
	
	unsigned char data_type = readByte[2];
	
	if(data_type != 1) {
		slPrint("(REJECTED RLE TGA)", slLocate(0,0));
		return 0;
	}
	
	int col_map_size = (readByte[5] | readByte[6]<<8) * 3;
	
	unsigned char bpp = readByte[16];
	
	if(bpp != 8) {
		slPrint("(REJECTED >8B TGA)", slLocate(0,0));
		return 0;
	}
	
	int imdat = id_field_size + col_map_size + TGA_HEADER_GAP;
	
	GLOBAL_img_addr = (unsigned char*)((int)readByte + imdat);
	
	add_texture_to_vram(readByte[12] | readByte[13]<<8, readByte[14] | readByte[15]<<8);
	
	return 1;
}

//tex_height = the height of each individual texture. must be uniform.
//You can set 0 to let the system treat your file as having all textures as H x W,
//meaning the tex height is the width too.
int read_tex_table_in_memory(void * file_start, int tex_height)
{
	unsigned char * readByte = (unsigned char *)file_start;
	unsigned char id_field_size = readByte[0];
	unsigned char col_map_type = readByte[1]; 
	
	if(col_map_type != 1){
		slPrint("(REJECTED RGB TGA)", slLocate(0,0));
		return 0;
	}
	
	unsigned char data_type = readByte[2];
	
	if(data_type != 1) {
		slPrint("(REJECTED RLE TGA)", slLocate(0,0));
		return 0;
	}
	
	int col_map_size = (readByte[5] | readByte[6]<<8) * 3;
	unsigned char bpp = readByte[16];
	
	if(bpp != 8) {
		slPrint("(REJECTED >8B TGA)", slLocate(0,0));
		return 0;
	}
	
	int imdat = id_field_size + col_map_size + TGA_HEADER_GAP;
	
	GLOBAL_img_addr = (unsigned char*)((int)readByte + imdat);
	
	short Twidth = readByte[12] | readByte[13]<<8;				//Plan: Width objectively defines the texture width
	short Theight = readByte[14] | readByte[15]<<8;			//Height is just the total height of the texture table, in scanlines
	short totText = (tex_height != 0) ? Theight / tex_height : Theight / Twidth;	//To get total # of textures, divide the height by the height of each texture
	short numPix = (tex_height != 0) ? Twidth * tex_height : Twidth * Twidth;		//To produce each texture jump the img addr ahead by the numPix of each tex
	short rdTexHeight = (tex_height != 0) ? tex_height : Twidth; //If tex height input is 0, just assume its the same as the width
	
	for(int i = 0; i < totText; i++)
	{
		add_texture_to_vram(Twidth, rdTexHeight);
		GLOBAL_img_addr += numPix;
	}
	
	return Twidth;
}


void	ReplaceTextureTable(void * file_start, int tex_height, int first_replaced_texno)
{
	
	unsigned char * readByte = (unsigned char *)file_start;
	unsigned char id_field_size = readByte[0];
	unsigned char col_map_type = readByte[1]; 

	if(col_map_type != 1){
		slPrint("(REJECTED RGB TGA)", slLocate(0,0));
		return;
	}
	
	unsigned char data_type = readByte[2];
	
	if(data_type != 1) {
		slPrint("(REJECTED RLE TGA)", slLocate(0,0));
		return;
	}
	
	int col_map_size = (readByte[5] | readByte[6]<<8) * 3;
	unsigned char bpp = readByte[16];
	
	if(bpp != 8) {
		slPrint("(REJECTED >8B TGA)", slLocate(0,0));
		return;
	}
	int imdat = id_field_size + col_map_size + TGA_HEADER_GAP;
	
	GLOBAL_img_addr = (unsigned char*)((int)readByte + imdat);
	
	short Twidth = readByte[12] | readByte[13]<<8;				//Plan: Width objectively defines the texture width
	short Theight = readByte[14] | readByte[15]<<8;			//Height is just the total height of the texture table, in scanlines
	short totText = (tex_height != 0) ? Theight / tex_height : Theight / Twidth;	//To get total # of textures, divide the height by the height of each texture
	short numPix = (tex_height != 0) ? Twidth * tex_height : Twidth * Twidth;		//To produce each texture jump the img addr ahead by the numPix of each tex
	short rdTexHeight = (tex_height != 0) ? tex_height : Twidth; //If tex height input is 0, just assume its the same as the width
	
	///////////////////////////
	// Filtering:
	// If tex height and width are not the same as the texture we are replacing, stop.
	// But first, retrieve the information about what we are replacing.
	///////////////////////////
	unsigned char * writeAddress = (unsigned char *)(VDP1_VRAM + (pcoTexDefs[first_replaced_texno].SRCA<<3));
	short originalX = (pcoTexDefs[first_replaced_texno].SIZE>>8)<<3;
	short originalY = pcoTexDefs[first_replaced_texno].SIZE & 0xFF;
	
	if(originalX != Twidth || rdTexHeight != originalY) return;
	/////////////////////////
	// **Replacing** textures in VRAM.
	// This needs a slightly different way of doing things.
	// In fact, all we need to do is move the new data over.
	// We don't want to change anything in the texture table.
	/////////////////////////
	for(int i = 0; i < totText; i++)
	{
		slDMACopy((void*)GLOBAL_img_addr, (void*)writeAddress, numPix);
		GLOBAL_img_addr += numPix;
		writeAddress += numPix;
	}
	
}

void WRAP_NewPalette(Sint8 * filename, void * file_start)
{
	get_file_in_memory(filename, (void*)file_start);
	set_tga_to_sprite_palette((void*)file_start);
}

Bool WRAP_NewTexture(Sint8 * filename, void * file_start)
{
	
	get_file_in_memory(filename, file_start);
	
	return read_pco_in_memory(file_start);
}

int WRAP_NewTable(Sint8 * filename, void * file_start, int tex_height)
{
	
	get_file_in_memory(filename, file_start);
	
	return read_tex_table_in_memory(file_start, tex_height);
}

void	WRAP_ReplaceTable(Sint8 * filename, void * file_start, int tex_height, int first_replaced_texno)
{
	get_file_in_memory(filename, file_start);
	
	ReplaceTextureTable(file_start, tex_height, first_replaced_texno);	
}
