//ldata.c
//this file is compiled separately
#include <sl_def.h>
#include <SEGA_GFS.H>
#include "def.h"
#include "bounder.h"
#include "physobjet.h"
#include "mloader.h"
#include "render.h"
#include "player_phy.h"
#include "tga.h"
#include "pcmsys.h"
#include "pcmstm.h"

#include "ldata.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>



Bool ldata_ready = false;

void	declarations(void)
{

}

void	process_binary_ldata(void * source_data)
{
	purge_object_list();
	unsigned short * aptr;
	char * cptr = (char *)source_data;
	
	//Alright, I really did not want to do this in fixed-order-processing.
	//However, it's just so much less efficient to do it any other way.
	//I would've thought just searching a chunk of RAM for a string was simple, but no, there's no single function.
	
	//////////////////////////////////
	//Get the music names out of the file
	//////////////////////////////////
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
	//Prepare to load the object list
	//////////////////////////////////
	aptr = (unsigned short *)(cptr + 8);
	unsigned short objct = *aptr++;
	//nbg_sprintf(1, 10, "oct(%i)", objct);
	unsigned short args[9];
	
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
	start_pcm_stream(&stg_mus[0][0], 3);
	stm.times_to_loop = 255;
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


