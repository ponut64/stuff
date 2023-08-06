//ldata.c
//this file is compiled separately
#include <sl_def.h>
#include <SEGA_GFS.H>
#include "pcmsys.h"
#include "pcmstm.h"
#include "def.h"
#include "bounder.h"
#include "physobjet.h"
#include "mloader.h"
#include "render.h"
#include "player_phy.h"
#include "tga.h"
#include "object_col.h"
#include "sound.h"
#include "hmap.h"

#include "ldata.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>



Bool ldata_ready = false;

void	declarations(void)
{

}

void	replace_table_0(void * source_data)
{
	ReplaceTextureTable(source_data, 24, map_texture_table_numbers[0]);
	make_4way_combined_textures(map_texture_table_numbers[0], map_end_of_original_textures, map_end_of_original_textures);
	make_dithered_textures_for_map(1); 
	for(int i = 0; i < 13; i++)
	{
		old_tex_tbl_names[0][i] = map_tex_tbl_names[0][i];
	}
}

void	replace_table_1(void * source_data)
{
	ReplaceTextureTable(source_data, 24, map_texture_table_numbers[1]);
	make_4way_combined_textures(map_texture_table_numbers[0], map_end_of_original_textures, map_end_of_original_textures);
	make_dithered_textures_for_map(1); 
	for(int i = 0; i < 13; i++)
	{
		old_tex_tbl_names[1][i] = map_tex_tbl_names[1][i];
	}
}

void	replace_table_2(void * source_data)
{
	ReplaceTextureTable(source_data, 24, map_texture_table_numbers[2]);
	make_4way_combined_textures(map_texture_table_numbers[0], map_end_of_original_textures, map_end_of_original_textures);
	make_dithered_textures_for_map(1); 
	for(int i = 0; i < 13; i++)
	{
		old_tex_tbl_names[2][i] = map_tex_tbl_names[2][i];
	}
}

void	replace_table_3(void * source_data)
{
	ReplaceTextureTable(source_data, 24, map_texture_table_numbers[3]);
	make_4way_combined_textures(map_texture_table_numbers[0], map_end_of_original_textures, map_end_of_original_textures);
	make_dithered_textures_for_map(1); 
	for(int i = 0; i < 13; i++)
	{
		old_tex_tbl_names[3][i] = map_tex_tbl_names[3][i];
	}
}

void	replace_table_4(void * source_data)
{
	ReplaceTextureTable(source_data, 24, map_texture_table_numbers[4]);
	make_4way_combined_textures(map_texture_table_numbers[0], map_end_of_original_textures, map_end_of_original_textures);
	make_dithered_textures_for_map(1); 
	for(int i = 0; i < 13; i++)
	{
		old_tex_tbl_names[4][i] = map_tex_tbl_names[4][i];
	}
}

void	process_binary_ldata(void * source_data)
{
	purge_rotated_entities();
	purge_object_list();
	unsigned short * aptr;
	char * cptr = (char *)source_data;
	
	//Alright, I really did not want to do this in fixed-order-processing.
	//However, it's just so much less efficient to do it any other way.
	//I would've thought just searching a chunk of RAM for a string was simple, but no, there's no single function.

	
	//////////////////////////////////
	//Get the music names out of the file
	//The file leads with the label "MUSIC!"
	//We must step the character pointer ahead by 6 to pass that.
	//////////////////////////////////
	cptr+=6;
	for(int l = 0; l < 3; l++)
	{
		for(int i = 0; i < 12; i++)
		{
			if(*cptr != ' ')
			{
				stg_mus[l][i] = *cptr++;
			} else {
				stg_mus[l][i] = 0; //NUL character / string terminator
				cptr++;
			}
		}
	}
	
	
	//////////////////////////////////
	//Prepare to get the texture list names out of the file
	//The file has the label "MAPTEX" after the music names.
	//We must step the pointer ahead by 6 to pass that.
	//////////////////////////////////
	cptr+=6;

	for(int l = 0; l < 5; l++)
	{
		short parity = 0;
		for(int i = 0; i < 12; i++)
		{
			if(*cptr != ' ')
			{
				if(*cptr == old_tex_tbl_names[l][i]) parity++;
				map_tex_tbl_names[l][i] = *cptr;
			} else {
				map_tex_tbl_names[l][i] = 0; //NUL character / string terminator
				if(old_tex_tbl_names[l][i] == 0) parity++;
			}
			cptr++;
		}
		if(parity != 12)
		{
			if(l == 0)
			{
				new_file_request((Sint8*)map_tex_tbl_names[l], dirty_buf, replace_table_0, 1); 
				//nbg_sprintf(2, 8, "0(%i)", parity);
				//nbg_sprintf(2, 8, "%s", &map_tex_tbl_names[l][0]);
			} else if(l == 1){
				new_file_request((Sint8*)map_tex_tbl_names[l], dirty_buf, replace_table_1, 1); 
				//nbg_sprintf(2, 9, "%s", &map_tex_tbl_names[l][0]);
			} else if(l == 2){
				new_file_request((Sint8*)map_tex_tbl_names[l], dirty_buf, replace_table_2, 1); 
				//nbg_sprintf(2, 10, "%s", &map_tex_tbl_names[l][0]);
			} else if(l == 3){
				new_file_request((Sint8*)map_tex_tbl_names[l], dirty_buf, replace_table_3, 1); 
				//nbg_sprintf(2, 11, "3(%i)", parity);
			} else if(l == 4){
				new_file_request((Sint8*)map_tex_tbl_names[l], dirty_buf, replace_table_4, 1); 
				//nbg_sprintf(2, 12, "4(%i)", parity);
			}
		}
	}
	
	//////////////////////////////////
	// Prepare to get the palette out of the file
	// First, skip over the marker "PALTEX"
	//////////////////////////////////
	cptr += 6;
	static char palname[13];
	
	for(int i = 0; i < 12; i++)
	{
		if(*cptr != ' ')
		{
			palname[i] = *cptr++;
		} else {
			palname[i] = 0; //NUL character / string terminator
			cptr++;
		}
	}
	
	new_file_request((Sint8*)palname, dirty_buf, set_tga_to_sprite_palette, 1);
	
	//////////////////////////////////
	//Prepare to load the object list
	//The character pointer must move ahead of the label "OBJECTS!". That is 8 characters.
	//////////////////////////////////
	aptr = (unsigned short *)(cptr + 8);
	unsigned short objct = *aptr++;
	//nbg_sprintf(1, 10, "oct(%i)", objct);
	unsigned short args[9];
	//////////////////////////////////
	//	IMPORTANT:
	//	The level data is controlled by a variety of linked lists.
	//	These lists terminate with -1. They must start at -1 or else it will crash.
	//////////////////////////////////
	for(int i = 0; i < 8; i++)
	{
		link_starts[i] = -1;
	}
	//////////////////////////////////
	// Load the object list
	// Nine arguments to function, two bytes per argument, 18 bytes at a time.
	//////////////////////////////////
	for(int i = 0; i < objct; i++)
	{
		args[0] = *aptr++;
		args[1] = *aptr++;
		args[2] = *aptr++;
		args[3] = *aptr++;
		args[4] = *aptr++;
		args[5] = *aptr++;
		args[6] = *aptr++;
		args[7] = *aptr++;
		args[8] = *aptr++;
		declare_object_at_cell(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]);
	}
	//////////////////////////////////
	// Start the level's idle music (first entered music)
	//////////////////////////////////
	set_music_track = 0;
	stm.times_to_loop = 255;
	
	level_data_basic();
	
}

void	testing_level_data(Sint8 * filename, void * destination)
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
	
	process_binary_ldata(destination);
	
}

void	level_data_basic(void)
{

	//declarations();
	ldata_ready = true;

	post_ldata_init_building_object_search();
	
		//////////////////////////////////
		//
		// Send player to map's start location
		// Notice: Code chunk must run after objects are declared
		// Notice: Only finds first start location, uses it, then flags it as used.
		//////////////////////////////////
	for(int i = 0; i < objNEW; i++)
	{
		//The following condition should indicate that we've found a declared player start and it hasn't been used yet.
		if((dWorldObjects[i].type.ext_dat & LDATA_TYPE) == PSTART && (dWorldObjects[i].type.ext_dat & ETYPE) == LDATA)
		{
			//They're negative because of COORDINATE SYSTEM MAYHEM.
			you.startPos[X] = -dWorldObjects[i].pos[X];
			you.startPos[Y] = -dWorldObjects[i].pos[Y];
			you.startPos[Z] = -dWorldObjects[i].pos[Z];
			reset_player();
			dWorldObjects[i].type.ext_dat |= 0x8000;
		}
	}

}


