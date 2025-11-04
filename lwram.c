//lwram.c
//this file is included in main.c

void	init_ztable(void)
{
	//Plan:
	//For (scrn_dist), divide it by (-32767 to +32767)<<16
	for(int i = -32767; i < 32768; i++)
	{
		int sample = fxdiv(scrn_dist, i<<16);
		zTable[i] = (sample);
	}

	// :)
	zTable[0] = zTable[1];
}

void	init_lwram(void)
{
	//MAGIC NUMBERS
	unsigned char * scary_zone_end = (unsigned char *)(LWRAM + (1024 * 768));
	unsigned char * scary_zone_start = (unsigned char *)(LWRAM + (1024 * 512));
	
	int ram_use_report = 0;
	////////////////////////////////////
	//Initialize all of LWRAM
	//This should not be needed, but whatever, you know?
	////////////////////////////////////
	short * init_sample = (short*)LWRAM;
	for(int i = 0; i < (512 * 1024); i++)
	{
		init_sample[i] = 0;
	}
// Overwritten loading buffer
	dirty_buf = (void*)(LWRAM_END)-65536; 
	ram_use_report+= (65536);
// Second buffer
	dirtier_buf = (void*)(dirty_buf)-65536; 
	ram_use_report+= 65536;
// Textue Definitions
	//In LWRAM because why use HWRAM for it? // 8kb
	pcoTexDefs = (void*)((unsigned int)(dirtier_buf-(sizeof(paletteCode) * 4096)));
	ram_use_report += sizeof(paletteCode) * 4096;
// Object Table
//	dWorldObjects = (void*)((unsigned int)(pcoTexDefs-(sizeof(_declaredObject) * MAX_WOBJS))); //In LWRAM // 12KBish
//	ram_use_report += sizeof(_declaredObject) * MAX_WOBJS;
// Building (Source Data) Object Table
//	BuildingPayload = (void*)((unsigned int)(dWorldObjects-(sizeof(_buildingObject) * MAX_BUILD_OBJECTS)));
//	ram_use_report += sizeof(_buildingObject) * MAX_BUILD_OBJECTS;
///////////////////////////////////////////////////
// This region is subject to some unusual memory corruption.
///////////////////////////////////////////////////
//Adjacent Quad Table. This has an arbitrary size.
	sectorPathHeap = (void*)((unsigned int)(pcoTexDefs - (32 * 1024)));
	pathStackPtr = (void*)(sectorPathHeap);
	pathStackMax = (void*)(sectorPathHeap + (32 * 1024));
	ram_use_report += 32 * 1024;
//Pathing Guides (sizeof(_pathHost))
	pathing = (void*)((unsigned int)(sectorPathHeap - (sizeof(_pathHost))));
	ram_use_report += sizeof(_pathHost);
//Space used from end of LWRAM: about 300KB
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Z-Table - this will be used with signed offsets; its size is actually 256kb
	zTable = (void*)((unsigned int)(LWRAM + (128 * 1024)));
	init_ztable();

//It took a lot of fighting to find a safe place to put this in RAM.
//Using RAM from 384-640KB into LWRAM
//RAM allocation for a duplicate of VDP2 viewmodel 0.
	viewmodel_0_workram_copy = (void*)((unsigned int)(scary_zone_start - (128 * 1024)));
//RAM allocation for a duplicate of VDP2 viewmodel 1.
	viewmodel_1_workram_copy = (void*)((unsigned int)(scary_zone_start));
	
	nbg_sprintf(3, 10, "ramuse(%i)", ram_use_report);
}

