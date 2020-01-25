//
#include "tga.h"

unsigned int * cRAM_24bm = (unsigned int *)0x05F00000;
unsigned short * cRAM_16bm = (unsigned short *)0x05F00000;

unsigned char * GLOBAL_img_addr = (unsigned char *)LWRAM;

unsigned char * sprPalette = 0;
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
	
bool	set_tga_to_sprite_palette(void * file_start) //Returns "true" if successful.
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
	
	//unsigned short xSizeLoBits = readByte[12]; //unused
	//unsigned short ySizeLoBits = readByte[14]; //unused, because the has an assumed size 
	
	unsigned char byteFormat = readByte[16];
	
	if(byteFormat != 24){
		jo_printf(0, 0, "(TGA NOT 24BPP)");
		return false; //File Typing Demands 24 bpp.
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
		sprPalette = (unsigned char *)&cRAM_24bm[256];
		cRAM_24bm[1] = (255<<16) | (255<<8) | (255);
	
	return true;
}

void	add_texture_to_vram(int width, int height)
{
	static unsigned char * curVRAMptr = (unsigned char*)(VDP1_VRAM + VRAM_TEXTURE_BASE); //see render.h
	
	int totalPix = width * height;
	
	pcoTexDefs[numTex].SIZE = (width>>3)<<8 | height; //This table is //In LWRAM - see lwram.c
	slDMACopy((void*)GLOBAL_img_addr, (void*)curVRAMptr, totalPix);
	pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)curVRAMptr); //This table is //In LWRAM - see lwram.c
	curVRAMptr += totalPix;
	numTex++;
}

/*
Palette Colors --> Supplied by formatted TGA file.

Textures Palette --> Supplied by formatted 64-color palette in GIMP to match TGA palette section for "normal brightness" color.

Texture	--> Color Index Pixels of exported tga file.
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
	
	unsigned char col_map_size = (readByte[5] | readByte[6]<<8) * 3;
	
	unsigned char bpp = readByte[16];
	
	if(bpp != 8) {
		jo_printf(0, 0, "(REJECTED >8B PCO)");
		return false;
	}
	
	unsigned char imdat = id_field_size + col_map_size + 18;
	
	GLOBAL_img_addr = (unsigned char*)((int)readByte + imdat);
	
	add_texture_to_vram(readByte[12] | readByte[13]<<8, readByte[14] | readByte[15]<<8);
	
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
	
	unsigned char col_map_size = (readByte[5] | readByte[6]<<8) * 3;
	unsigned char bpp = readByte[16];
	
	if(bpp != 8) {
		jo_printf(0, 0, "(REJECTED >8B PCO)");
		return false;
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
	
	return true;
}

bool WRAP_NewPalette(Sint8 * filename, void * file_start)
{
	get_file_in_memory(filename, (void*)file_start);
	return set_tga_to_sprite_palette((void*)file_start);
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

