//

#include <sl_def.h>
#include <SEGA_GFS.H>
#include "def.h"
#include "render.h" 
#include "vdp2.h"
#include "hmap.h"
#include "mymath.h"

#include "tga.h"

unsigned int * cRAM_24bm = (unsigned int *)0x05F00000;
unsigned short * cRAM_16bm = (unsigned short *)0x05F00000;

unsigned char * GLOBAL_img_addr = (unsigned char *)LWRAM;
unsigned char * curVRAMptr = (unsigned char*)(VDP1_VRAM + VRAM_TEXTURE_BASE); //see render.h

unsigned int sprPaletteCopy[256];

int numTex = 0;

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
	
Bool	set_tga_to_sprite_palette(void * file_start) //Returns "1" if successful.
{
	unsigned char * readByte = (unsigned char *)file_start;
	unsigned short * readWord = (unsigned short *)file_start;
	
	unsigned char id_field_size = readByte[0];
	
	unsigned char col_map_type = readByte[1]; 
	
	if(col_map_type != 0){
		slPrint("(REJECTED NON-RGB TGA)", slLocate(0,0));
		return 0;
	}
	unsigned char data_type = readByte[2];
	
	if(data_type != 2) {
		slPrint("(REJECTED RLE TGA)", slLocate(0,0));
		return 0;
	}
	//Color Map Specification Data is ignored.
	
	//X / Y origin data is ignored.
	
	//unsigned short xSizeLoBits = readByte[12]; //unused
	//unsigned short ySizeLoBits = readByte[14]; //unused, because the has an assumed size 
	
	unsigned char byteFormat = readByte[16];
	
	if(byteFormat != 24){
		slPrint("(TGA NOT 24BPP)", slLocate(0,0));
		return 0; //File Typing Demands 24 bpp.
	}
	
	//Descriptor Bytes are skipped.
	
	unsigned char imdat = id_field_size + 18;
	
	GLOBAL_img_addr = (unsigned char*)((int)readWord + imdat);
	
	readByte = GLOBAL_img_addr;
	unsigned char component[XYZ] = {0, 0, 0}; //Actually "R, G, B"
	unsigned int final_color = 0;

	for(int i = 0; i < 256; i++){
		component[X] = readByte[(i*3)];
		component[Y] = readByte[(i*3)+1];
		component[Z] = readByte[(i*3)+2];
		
		final_color = (unsigned int)((component[X]<<16) | (component[Y]<<8) | (component[Z]));
		
		cRAM_24bm[i+256] = (final_color);
		sprPaletteCopy[i] = final_color;
		
	}
		//Set the text color (temporary, depends on library setting)
		cRAM_24bm[1] = (255<<16) | (255<<8) | (255);
	
	return 1;
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
		mod_color = cRAM_24bm[i+256];
		
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
		cRAM_24bm[i+256] = mod_color;
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
		cRAM_24bm[i+256] = sprPaletteCopy[i];
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

int		new_dithered_texture(int texno_a, int texno_b, short replaced_texture)
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

void	make_4way_combined_textures(int start_texture_number, int end_texture_number, short replacement_start)
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

void	select_and_cut_from_x0y0(int img_x, int img_y, int sel_max_x, int sel_max_y, unsigned char * img, unsigned char * sel_img)
{
	int total_img_pix = img_x * img_y;
	int pix_write_pt = 0;
	
	for(int p = 0; p < total_img_pix; p++)
	{
		int numY = p / (img_x);
		int numX = p - (numY * (img_x));

		if(numY >= sel_max_y) continue;
		if(numX >= sel_max_x) continue;
		sel_img[pix_write_pt] = img[p];
		pix_write_pt++;
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

void	make_combined_textures(int texture_number)
{
	
	/////////////////////////////////////////////////
	/**

	REWIND
	
	What I want to do, actually, is intake a 64x64 texture and tile it all the way down to 8x8.
	Texture cuts of the following sizes are needed:
	
	8x64	8x32	8x16	8x8
	16x64	16x32	16x16	16x8
	32x64	32x32	32x16	32x8
	64x64	64x32	64x16	64x8
	(Dimensions past 32x32 will be down-sampled to 32x32)
	
	For initial testing, I will start by tiling the 8x8 texture all the way up to 128x128.
	
	From that 128x128 source, I will generate all of the above textures.
	I will also continue to support texture-to-tiles like that.
	
	**/
	/////////////////////////////////////////////////

	unsigned char * source_texture_data = (unsigned char *)((unsigned int)(VDP1_VRAM + (pcoTexDefs[texture_number].SRCA<<3)));
	unsigned char * readByte = source_texture_data;
	int base_x = (pcoTexDefs[texture_number].SIZE & 0x3F00)>>5;
	int base_y = (pcoTexDefs[texture_number].SIZE & 0xFF);
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
	
	for(int dsy = 0; dsy < 4; dsy++)
	{
		ssy = tf_sy>>dsy;
		for(int dsx = 0; dsx < 4; dsx++)
		{
			ssx = tf_sx>>dsx;
			int tex_x = (ssx > max_axis) ? max_axis : ssx;
			int tex_y = (ssy > max_axis) ? max_axis : ssy;
			select_and_cut_from_x0y0(tf_sx, tf_sy, ssx, ssy, dirty_buf, buf_map);
			generate_downscale_texture(ssx, ssy, tex_x, tex_y, buf_map);
		}
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
	
	unsigned char col_map_size = (readByte[5] | readByte[6]<<8) * 3;
	
	unsigned char bpp = readByte[16];
	
	if(bpp != 8) {
		slPrint("(REJECTED >8B TGA)", slLocate(0,0));
		return 0;
	}
	
	unsigned char imdat = id_field_size + col_map_size + 18;
	
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
	
	unsigned char col_map_size = (readByte[5] | readByte[6]<<8) * 3;
	unsigned char bpp = readByte[16];
	
	if(bpp != 8) {
		slPrint("(REJECTED >8B TGA)", slLocate(0,0));
		return 0;
	}
	
	unsigned char imdat = id_field_size + col_map_size + 18;
	
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
	
	unsigned char col_map_size = (readByte[5] | readByte[6]<<8) * 3;
	unsigned char bpp = readByte[16];
	
	if(bpp != 8) {
		slPrint("(REJECTED >8B TGA)", slLocate(0,0));
		return;
	}
	unsigned char imdat = id_field_size + col_map_size + 18;
	
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

Bool WRAP_NewPalette(Sint8 * filename, void * file_start)
{
	get_file_in_memory(filename, (void*)file_start);
	return set_tga_to_sprite_palette((void*)file_start);
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
