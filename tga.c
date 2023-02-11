//

#include <sl_def.h>
#include <SEGA_GFS.H>
#include "def.h"
#include "render.h" 
#include "vdp2.h"

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
	
	pcoTexDefs[numTex].SIZE = ((width>>3)<<8) | height; //This table is //In LWRAM - see lwram.c
	slDMACopy((void*)GLOBAL_img_addr, (void*)curVRAMptr, totalPix);
	pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)curVRAMptr); //This table is //In LWRAM - see lwram.c
	curVRAMptr += totalPix;
	numTex++;
}

int		new_dithered_texture(int texno_a, int texno_b)
{
	//Core concept: Create a new texture, same size as the both textures which also need to be the same size,
	// with every odd pixel from texture A, and every even pixel from texture B.
	
		unsigned char * a_data = (unsigned char *)((unsigned int)(VDP1_VRAM + (pcoTexDefs[texno_a].SRCA<<3)));
		unsigned char * b_data = (unsigned char *)((unsigned int)(VDP1_VRAM + (pcoTexDefs[texno_b].SRCA<<3)));
		
		if(pcoTexDefs[texno_a].SIZE != pcoTexDefs[texno_b].SIZE) return 0; //Unequal texture sizes, exit.
		//Past this point, we will be writing a new texture. So set it up.
		
		int base_x = (pcoTexDefs[texno_a].SIZE & 0x3F00)>>5;
		int base_y = (pcoTexDefs[texno_a].SIZE & 0xFF);
		int total_bytes_of_original_texture = base_x * base_y;
	
		unsigned char * texture_start = curVRAMptr;
		pcoTexDefs[numTex].SIZE = pcoTexDefs[texno_a].SIZE;
		pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)curVRAMptr); 
		numTex++;
	
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
		curVRAMptr += total_bytes_of_original_texture;
		return numTex-1;
}

void	make_4way_combined_textures(int start_texture_number, int end_texture_number)
{
	for(int t = start_texture_number; t < end_texture_number; t++)
	{
		unsigned char * source_texture_data = (unsigned char *)((unsigned int)(VDP1_VRAM + (pcoTexDefs[t].SRCA<<3)));
		unsigned char * readByte = source_texture_data;
		int base_x = (pcoTexDefs[t].SIZE & 0x3F00)>>5;
		int base_y = (pcoTexDefs[t].SIZE & 0xFF);
		int total_bytes_of_original_texture = base_x * base_y;
		int readPoint = 0;
		int write_point = 0;
		unsigned char * texture_start = curVRAMptr;
		pcoTexDefs[numTex].SIZE = pcoTexDefs[t].SIZE;
		pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)curVRAMptr); 
		numTex++;
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
		curVRAMptr += total_bytes_of_original_texture;
	}
}

void	make_combined_textures(int texture_number)
{
	
	/////////////////////////////////////////////////
	/*
	Creates combined textures of the same size as the original.
	Texture 1: Skips every even row (appears tiled in Y)
	Texture 2: Skips every even column (appears tiled in X)
	Texture 3: Skips every even row and column (appears tiled in XY)
	Texture 4: Low LOD texture. Texture data reduced to 8x8.
	*/
	/**
	New directive:
	Say we are starting with an 8x8 texture.
	This needs to instead create:
	Tile Once:
	8x16 Tile on Y
	16x8 Tile on X
	16x16 Tile on XY
	
	Tile Twice:
	8x16 Tile on Y Twice (half-res)
	16x8 Tile on X Twice (half-res)
	16x16 Tile on XY Twice (half-res)
	
	Tile Thrice:
	16x32 Tile on Y Thrice (quarter-res, in the end)
	32x16 Tile on X Thrice
	32x32 Tile on XY Thrice
	
	Say this polygon pattern is representing a polygon at maximum subdivision:
	Forgive the representation, I just mean every letter is a unique polygon.
	This is subdivision 3; maximal subdivision.
	A B C D E F a b
	H I J K L M c d		In this case, we should use the base texture.
	N O P Q R S e f
	T U V W X Y g h
	Z 0 1 2 3 4 i j
	5 6 7 8 9 + l m
	n o p q r s t u
	w x y z ! @ # $
	
	At subdivisision two, it looks like:
	In this case, same-letters mean same polygon.
	A A B B C C a a
	A A B B C C	a a		The texture is the base texture tiled twice / represents two identical textures.
	D D E E F F b b		In two dimensions, it represents four polygons.
	D D E E F F b b
	H H I I G G c c 
	H H I I G G c c
	d d e e f f g g
	d d e e f f g g
	
	At subdivision one, it looks like:
	A A A A B B B B
	A A A A B B B B		The texture is the base texture tiled four times / representing four identical textures.
	A A A A B B B B		In two dimensions, it represents 16 polygons.
	A A A A B B B B
	C C C C D D D D
	C C C C D D D D
	C C C C D D D D 
	C C C C D D D D
	
	With no subdivison, it is simply:
	
	A A A A A A A A
	A A A A A A A A		The texture is the base texture tiled eight times / representing eight identical textures.
	A A A A A A A A		In one dimension, of course. It ends up representing 64 polygons, or 64 textures if it is XY!
	A A A A A A A A
	A A A A A A A A
	A A A A A A A A
	A A A A A A A A
	A A A A A A A A
	
	**/
	/////////////////////////////////////////////////

	unsigned char * source_texture_data = (unsigned char *)((unsigned int)(VDP1_VRAM + (pcoTexDefs[texture_number].SRCA<<3)));
	unsigned char * readByte = source_texture_data;
	int base_x = (pcoTexDefs[texture_number].SIZE & 0x3F00)>>5;
	int base_y = (pcoTexDefs[texture_number].SIZE & 0xFF);
	
	// nbg_sprintf(5, 10, "bx(%i)", base_x);
	// nbg_sprintf(5, 11, "by(%i)", base_y);
	
	int total_bytes_of_original_texture = base_x * base_y;

	//Set up the texture structs.
	//First texture --> X , Y*2
	unsigned char * first_texture_start = curVRAMptr;
	pcoTexDefs[numTex].SIZE = pcoTexDefs[texture_number].SIZE;
	pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)first_texture_start); 
	numTex++;
	//Second texture -> X*2, Y
	unsigned char * second_texture_start = first_texture_start + (total_bytes_of_original_texture);
	pcoTexDefs[numTex].SIZE = pcoTexDefs[texture_number].SIZE;
	pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)second_texture_start); 
	numTex++;
	//Third texture -> X*2, Y*2 
	unsigned char * third_texture_start = second_texture_start + (total_bytes_of_original_texture);
	pcoTexDefs[numTex].SIZE = pcoTexDefs[texture_number].SIZE;
	pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)third_texture_start); 
	numTex++;
	//Fourth texture -> X = 8, Y = 8
	unsigned char * fourth_texture_start = third_texture_start + (total_bytes_of_original_texture);
	pcoTexDefs[numTex].SIZE = (1<<8) | 8;
	pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)fourth_texture_start); 
	numTex++;
	//Texture 1: Copy the base texture, but skip every even row.
	int readPoint = 0;
	int write_point = 0;
	//CORE CONCEPT: Get a copy of the texture, written in (base_x) and (base_y>>1) size.
	//This copy is a downscale.
	//Then copy that to this texture, two times.
	/*
	GETTING DOWNSCALE
	*/
	for(int j = 0; j < (base_y>>1); j++)
	{
		for(int w = 0; w < (base_x); w++)
		{
		readPoint = (j * base_x * 2) + (w);
		dirty_buf[write_point] = readByte[readPoint];
		write_point++;
		}
	}
	/*
	WRITING DOWNSCALE 2 TIMES
	COMPLICATION: Pretty simple actually, just write the downscaled texture twice.
	*/
	for(int j = 0; j < 2; j++)
	{
		for(int w = 0; w < (base_y>>1); w++)
		{
			for(int v = 0; v < (base_x); v++)
			{
				*first_texture_start++ = dirty_buf[(w * (base_x)) + v];
			}
		}
	}
	readPoint = 0;
	write_point = 0;
	//Texture 2: Copy the base texture, but skip every even column.
	//CORE CONCEPT: Get a copy of the texture, written in (base_x>>1) and (base_y) size.
	//This copy is a downscale.
	//Then copy that to this texture, two times.
	/*
	GETTING DOWNSCALE
	*/
	for(int j = 0; j < (base_y); j++)
	{
		for(int w = 0; w < (base_x>>1); w++)
		{
		readPoint = (j * base_x) + (w * 2);
		dirty_buf[write_point] = readByte[readPoint];
		write_point++;
		}
	}
	/*
	WRITING DOWNSCALE 2 TIMES
	COMPLICATION: Must write each line of the downscaled texture twice!
	*/
	for(int w = 0; w < (base_y); w++)
	{
		for(int v = 0; v < (base_x>>1); v++)
		{
			*second_texture_start++ = dirty_buf[(w * (base_x>>1)) + v];
		}
		for(int v = 0; v < (base_x>>1); v++)
		{
			*second_texture_start++ = dirty_buf[(w * (base_x>>1)) + v];
		}
	}
	readPoint = 0;
	write_point = 0;
	//Texture 3: Copy the base texture, but skip every even column and row.
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
				*third_texture_start++ = dirty_buf[(w * (base_x>>1)) + v];
			}
			for(int v = 0; v < (base_x>>1); v++)
			{
				*third_texture_start++ = dirty_buf[(w * (base_x>>1)) + v];
			}
		}
	}
	//Texture 4: Divide the texture's width and height by 8, and sample a pixel every d/8 texels.
	int x_skip = base_x / 8;
	int y_skip = base_y / 8;
	for(int j = 0; j < 8; j++)
	{
		for(int w = 0; w < 8; w++)
		{
		*fourth_texture_start++ = readByte[(j * y_skip * base_y) + (w * x_skip)];	
		}
	}
	//Total combined texture data is the original texture size, copied three times over, plus 64 (8x8).
	curVRAMptr = (fourth_texture_start + 64);
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
		slPrint("(REJECTED RGB PCO)", slLocate(0,0));
		return 0;
	}
	
	unsigned char data_type = readByte[2];
	
	if(data_type != 1) {
		slPrint("(REJECTED RLE PCO)", slLocate(0,0));
		return 0;
	}
	
	unsigned char col_map_size = (readByte[5] | readByte[6]<<8) * 3;
	
	unsigned char bpp = readByte[16];
	
	if(bpp != 8) {
		slPrint("(REJECTED >8B PCO)", slLocate(0,0));
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
		slPrint("(REJECTED RGB PCO)", slLocate(0,0));
		return 0;
	}
	
	unsigned char data_type = readByte[2];
	
	if(data_type != 1) {
		slPrint("(REJECTED RLE PCO)", slLocate(0,0));
		return 0;
	}
	
	unsigned char col_map_size = (readByte[5] | readByte[6]<<8) * 3;
	unsigned char bpp = readByte[16];
	
	if(bpp != 8) {
		slPrint("(REJECTED >8B PCO)", slLocate(0,0));
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
