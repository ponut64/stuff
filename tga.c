//
#include "tga.h"
#define VDP1_VRAM 0x25C00000
#define MAP_TO_VRAM(sh2map_vram_addr) ((sh2map_vram_addr - VDP1_VRAM)>>3) 

unsigned int * cRAM_24bm = (unsigned int *)0x05F00000;
unsigned short * cRAM_16bm = (unsigned short *)0x05F00000;

unsigned short * GLOBAL_img_addr = (unsigned short *)LWRAM;
short GLOBAL_img_line_count = 0;
short GLOBAL_img_line_width = 0;

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

void set_tga_as_palette(void)
{	
	unsigned char component[XYZ] = {0, 0, 0}; //Actually "R, G, B"
	unsigned char * readByte = (unsigned char *)GLOBAL_img_addr;
	unsigned int final_color = 0;

	for(int i = 0; i < 256; i++){
		component[X] = readByte[(i*3)];
		component[Y] = readByte[(i*3)+1];
		component[Z] = readByte[(i*3)+2];
		
		final_color = (unsigned int)((component[X]<<16) | (component[Y]<<8) | (component[Z]));
		
		cRAM_24bm[i+256] = (final_color);
	}
		cRAM_24bm[1] = (255<<16) | (255<<8) | (255);
}

	
bool	read_tga_in_memory(void * file_start) //Returns "true" if successful.
{
	unsigned char * readByte = (unsigned char *)file_start;
	unsigned short * readWord = (unsigned short *)file_start;
	
	unsigned char id_field_size = readByte[0];
	
	unsigned char col_map_type = readByte[1]; 
	
	if(col_map_type != 0){
		jo_printf(0, 0, "(REJECTED NON-RGB TGA)");
		return false;
	}
	unsigned char data_type = readByte[2];
	
	if(data_type != 2) {
		jo_printf(0, 0, "(REJECTED RLE TGA)");
		return false;
	}
	//Color Map Specification Data is ignored.
	
	//X / Y origin data is ignored.
	
	unsigned short xSizeLoBits = readByte[12];
	unsigned short ySizeLoBits = readByte[14];
	
	unsigned char byteFormat = readByte[16];
	
	if(byteFormat != 24){
		jo_printf(0, 0, "(TGA NOT 24BPP)");
		return false; //File Typing Demands 24 bpp.
	}
	
	//Descriptor Bytes are skipped.
	
	unsigned char imdat = id_field_size + 18;
	
	GLOBAL_img_addr = (unsigned short*)((int)readWord + imdat);
	GLOBAL_img_line_count = xSizeLoBits;
	GLOBAL_img_line_width = ySizeLoBits;
	
	set_tga_as_palette();
	
	return true;
}


/*
Palette Colors --> Supplied by formatted TGA file.

Textures Palette --> Supplied by formatted 64-color palette in GIMP to match TGA palette section for "normal brightness" color.

Texture	--> Color Index Pixels of exported pco file.
*/
bool read_pco_in_memory(void * file_start)
{
	unsigned char * readByte = (unsigned char *)file_start;
	
	unsigned char id_field_size = readByte[0];
	
	unsigned char col_map_type = readByte[1]; 
	
	if(col_map_type != 1){
		jo_printf(0, 0, "(REJECTED RGB PCO)");
		return false;
	}
	
	unsigned char data_type = readByte[2];
	
	if(data_type != 1) {
		jo_printf(0, 0, "(REJECTED RLE PCO)");
		return false;
	}
	
	unsigned char col_map_size = readByte[5] * 3;
	
	unsigned char bpp = readByte[16];
	
	if(bpp != 8) {
		jo_printf(0, 0, "(REJECTED >8B PCO)");
		return false;
	}
	
	unsigned char imdat = id_field_size + col_map_size + 18;
	
	GLOBAL_img_addr = (unsigned short*)((int)readByte + imdat);
	
	jo_img_8bits tex0;
	tex0.width = readByte[12];
	tex0.height = readByte[14];
	tex0.data = (unsigned char *)GLOBAL_img_addr;
	jo_sprite_add_8bits_image(&tex0);
	__jo_sprite_pic[jo_get_last_sprite_id()].data = (void *)MAP_TO_VRAM((int)__jo_sprite_pic[jo_get_last_sprite_id()].data);
	
	return true;
}

//tex_height = the height of each individual texture. must be uniform.
//You can set 0 to let the system treat your file as having all textures as H x W,
//meaning the tex height is the width too.
bool read_tex_table_in_memory(void * file_start, int tex_height)
{
	unsigned char * readByte = (unsigned char *)file_start;
	unsigned char id_field_size = readByte[0];
	unsigned char col_map_type = readByte[1]; 
	
	if(col_map_type != 1){
		jo_printf(0, 0, "(REJECTED RGB PCO)");
		return false;
	}
	
	unsigned char data_type = readByte[2];
	
	if(data_type != 1) {
		jo_printf(0, 0, "(REJECTED RLE PCO)");
		return false;
	}
	
	unsigned char col_map_size = readByte[5] * 3;
	unsigned char bpp = readByte[16];
	
	if(bpp != 8) {
		jo_printf(0, 0, "(REJECTED >8B PCO)");
		return false;
	}
	
	unsigned char imdat = id_field_size + col_map_size + 18;
	
	GLOBAL_img_addr = (unsigned short*)((int)readByte + imdat);
	
	short Twidth = readByte[12];				//Plan: Width objectively defines the texture width
	short Theight = readByte[14] | readByte[15]<<8;			//Height is just the total height of the texture table, in scanlines
	short totText = (tex_height != 0) ? Theight / tex_height : Theight / Twidth;	//To get total # of textures, divide the height by the height of each texture
	short numPix = (tex_height != 0) ? Twidth * tex_height : Twidth * Twidth;		//To produce each texture jump the img addr ahead by the numPix of each tex
	
	jo_img_8bits tex0;
	tex0.width = Twidth;
	tex0.height = (tex_height != 0) ? tex_height : Twidth;
	tex0.data = (unsigned char *)GLOBAL_img_addr;
	
	for(int i = 0; i < totText; i++)
	{
		jo_sprite_add_8bits_image(&tex0);
		tex0.data += numPix;
		__jo_sprite_pic[jo_get_last_sprite_id()].data = (void *)MAP_TO_VRAM((int)__jo_sprite_pic[jo_get_last_sprite_id()].data);
	}
	
	return true;
}

bool WRAP_NewPalette(Sint8 * filename, void * file_start)
{
	get_file_in_memory(filename, (void*)file_start);
	return read_tga_in_memory((void*)file_start);
}

bool WRAP_NewTexture(Sint8 * filename, void * file_start)
{
	
	get_file_in_memory(filename, file_start);
	
	return read_pco_in_memory(file_start);
}

bool WRAP_NewTable(Sint8 * filename, void * file_start, int tex_height)
{
	
	get_file_in_memory(filename, file_start);
	
	return read_tex_table_in_memory(file_start, tex_height);
}

