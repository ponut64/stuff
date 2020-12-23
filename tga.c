//
#include "tga.h"
#include "render.h" //Weird mutual include exception, sorry!

unsigned int * cRAM_24bm = (unsigned int *)0x05F00000;
unsigned short * cRAM_16bm = (unsigned short *)0x05F00000;

unsigned char * GLOBAL_img_addr = (unsigned char *)LWRAM;
unsigned char * curVRAMptr = (unsigned char*)(VDP1_VRAM + VRAM_TEXTURE_BASE); //see render.h

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
	int totalPix = width * height;
	
	pcoTexDefs[numTex].SIZE = ((width>>3)<<8) | height; //This table is //In LWRAM - see lwram.c
	slDMACopy((void*)GLOBAL_img_addr, (void*)curVRAMptr, totalPix);
	pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)curVRAMptr); //This table is //In LWRAM - see lwram.c
	curVRAMptr += totalPix;
	numTex++;
}

void	make_combined_textures(int texture_number)
{
	
	///////////////////////////////////////////
	// The goal here is to make 3 new textures that are combined versions of the original texture.
	// This is specifically for polygon subdivision.
	// These are added after the original texture sets complete payload, so we don't re-use the data from the original.
	// To mask the change from the divided and subdivided polygons, we want larger, combined variants of the original.
	// The first texture combination is X * (Y*2). This is the easiest, because you can just copy the texture over once.	// The second texture is (X*2) * Y.
	// This is complex, because you first must allocate a space twice the original's size,
	// then write each line from the first texture twice.
	// The final texture is (X*2) * (Y*2).
	// This texture can just be (X*2) * Y, duplicated once, using the same source address as (X*2) * Y.
	///////////////////////////////////////////

	unsigned char * source_texture_data = (unsigned char *)((unsigned int)(VDP1_VRAM + (pcoTexDefs[texture_number].SRCA<<3)));
	unsigned char * readByte = source_texture_data;
	int base_x = (pcoTexDefs[texture_number].SIZE & 0x3F00)>>5;
	int base_y = (pcoTexDefs[texture_number].SIZE & 0xFF);
	
	// jo_printf(5, 10, "bx(%i)", base_x);
	// jo_printf(5, 11, "by(%i)", base_y);
	
	int total_bytes_of_original_texture = base_x * base_y;

	//Set up the texture structs.
	//First texture --> X , Y*2
	unsigned char * first_texture_start = curVRAMptr;
	pcoTexDefs[numTex].SIZE = (((base_x)>>3)<<8) | base_y<<1;
	pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)first_texture_start); 
	numTex++;
	//Second texture -> X*2, Y
	unsigned char * second_texture_start = curVRAMptr + (total_bytes_of_original_texture<<1);
	pcoTexDefs[numTex].SIZE = (((base_x*2)>>3)<<8) | base_y;
	pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)second_texture_start); 
	numTex++;
	//Third texture -> X*2, Y*2 -- starts at same place as second texture
	unsigned char * third_texture_start = second_texture_start;
	pcoTexDefs[numTex].SIZE = (((base_x*2)>>3)<<8) | base_y<<1;
	pcoTexDefs[numTex].SRCA = MAP_TO_VRAM((int)third_texture_start); 
	numTex++;
	//Texture 1: Copy the data from the original, twice over.
	for(int j = 0; j < total_bytes_of_original_texture; j++)
	{
		*first_texture_start++ = readByte[j];
	}
	for(int j = 0; j < total_bytes_of_original_texture; j++)
	{
		*first_texture_start++ = readByte[j];
	}
	//Texture 2: Copy the data from the original, but copy each line twice.
	for(int j = 0; j < base_y; j++)
	{
		for(int w = 0; w < base_x; w++)
		{
			*second_texture_start++ = readByte[(j * base_x) + w];
		}
		for(int w = 0; w < base_x; w++)
		{
			*second_texture_start++ = readByte[(j * base_x) + w];
		}
	}
	//Texture 3: Copy the data from texture 2. Do some pointer juggling.
		second_texture_start = third_texture_start;
		third_texture_start += (total_bytes_of_original_texture<<1);
	for(int j = 0; j < (total_bytes_of_original_texture<<1); j++)
	{
		*third_texture_start++ = second_texture_start[j];
	}
	//Total combined texture data is the original texture size, copied six times over.
	//Twice for the first duplicate, four times for the second.
	curVRAMptr += (total_bytes_of_original_texture * 6);
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

