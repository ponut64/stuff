//ldata.c
//this file is compiled separately
#include <sl_def.h>
#include <SEGA_GFS.H>
#include "pcmsys.h"
#include "pcmstm.h"
#include "def.h"

#include "physobjet.h"
#include "mloader.h"
#include "render.h"
#include "tga.h"
#include "sound.h"
#include "mymath.h"
#include "collision.h"

#include "ldata.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>



Bool ldata_ready = false;

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

	//We don't do anything with these because these are for the heightmap that is no longer used.
	for(int l = 0; l < 5; l++)
	{
		for(int i = 0; i < 12; i++)
		{
			cptr++;
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
	
	
	new_file_request((Sint8*)palname, dirty_buf, set_tga_to_sprite_palette, HANDLE_FILE_ASAP);
	
	//////////////////////////////////
	// Prepare to get the background file name out of the file
	// First, skip over the marker "BACKTX"
	//////////////////////////////////
	
	cptr += 6;
	static char bgName[13];
	
	for(int i = 0; i < 12; i++)
	{
		if(*cptr != ' ')
		{
			bgName[i] = *cptr++;
		} else {
			bgName[i] = 0; //NUL character / string terminator
			cptr++;
		}
	}
	
	new_special_request((Sint8*)bgName, dirty_buf, set_8bpp_tga_to_nbg0_image_from_cd);
	
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

void	fetch_and_load_leveldata(Sint8 * filename)
{
	purge_rotated_entities();
	purge_object_list();
	unsigned short * aptr;
	char * cptr = (char *)dirty_buf;
	
	Sint32 fid = GFS_NameToId((Sint8*)filename);
	
	get_file_in_memory(fid, dirty_buf);
	
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

	//We don't do anything with these because these are for the heightmap that is no longer used.
	for(int l = 0; l < 5; l++)
	{
		for(int i = 0; i < 12; i++)
		{
			cptr++;
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
	
	
	//new_file_request((Sint8*)palname, dirty_buf, set_tga_to_sprite_palette, HANDLE_FILE_ASAP);
	
	//////////////////////////////////
	// Prepare to get the background file name out of the file
	// First, skip over the marker "BACKTX"
	//////////////////////////////////
	
	cptr += 6;
	static char bgName[13];
	
	for(int i = 0; i < 12; i++)
	{
		if(*cptr != ' ')
		{
			bgName[i] = *cptr++;
		} else {
			bgName[i] = 0; //NUL character / string terminator
			cptr++;
		}
	}
	
	//new_special_request((Sint8*)bgName, dirty_buf, set_8bpp_tga_to_nbg0_image_from_cd);
	
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
	
	fid = GFS_NameToId((Sint8*)palname);
	get_file_in_memory(fid, dirty_buf);
	set_tga_to_sprite_palette(dirty_buf);
	
	fid = GFS_NameToId((Sint8*)bgName);
	set_8bpp_tga_to_nbg0_image_from_cd(fid, dirty_buf);
	
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

void declarations(void)
{
	declare_object_at_cell(0, -(0), -240, 1 /*DestroyBlock*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(0, -(0), 240, 60 /*TestSpawner*/, 0, 0, 0, 0, 0);
	

}

void	level_data_basic(void)
{

	declarations();
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
		_declaredObject * obj = &dWorldObjects[i];
		//The following condition should indicate that we've found a declared player start and it hasn't been used yet.
		if((obj->type.ext_dat & LDATA_TYPE) == PSTART && (obj->type.ext_dat & ETYPE) == LDATA)
		{
			//They're negative because of COORDINATE SYSTEM MAYHEM.
			you.startPos[X] = -obj->pos[X];
			you.startPos[Y] = -obj->pos[Y];
			you.startPos[Z] = -obj->pos[Z];
			reset_player();
			obj->type.ext_dat |= 0x8000;
		}
		
		if(entities[obj->type.entity_ID].type == MODEL_TYPE_SECTORED)
		{
			levelPos[X] = obj->pos[X];
			levelPos[Y] = obj->pos[Y];
			levelPos[Z] = obj->pos[Z];
		}
	}

}


