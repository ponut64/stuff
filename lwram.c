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

//	int ram_use_report = 0;
	////////////////////////////////////
	//Initialize all of LWRAM
	//This should not be needed, but whatever, you know?
	////////////////////////////////////
	short * init_sample = (short*)LWRAM;
	for(int i = 0; i < (512 * 1024); i++)
	{
		init_sample[i] = 0;
	}
	
	void * lwram_ldptr = (void*)LWRAM;
	// Overwritten loading buffer
	dirty_buf = lwram_ldptr;
	lwram_ldptr += 65536; //sizeof buf
	// Second buffer
	dirtier_buf = lwram_ldptr;
	lwram_ldptr += 65536; //sizeof buf
	//In LWRAM because why use HWRAM for it? // 8kb
	pcoTexDefs = lwram_ldptr;
	lwram_ldptr += sizeof(paletteCode) * 4096;
	//Adjacent Quad Table. This has an arbitrary size.
	sectorPathHeap = lwram_ldptr;
	pathStackPtr = lwram_ldptr;
	lwram_ldptr += (32 * 1024);
	pathStackMax = lwram_ldptr;
	//Pathing Guides (sizeof(_pathHost))
	pathing = lwram_ldptr;
	lwram_ldptr += sizeof(_pathHost);
	viewmodel_0_workram_copy = lwram_ldptr;
	lwram_ldptr += (129 * 1024); //(the background images are literally 128kb but i guess you need a bit extra margin)
	viewmodel_1_workram_copy = lwram_ldptr;
	//Z-Table - this will be used with signed offsets; its size is actually 256kb
	lwram_ldptr += (257 * 1024); //(the background images are literally 128kb but i guess you need a bit extra margin)
	zTable = lwram_ldptr;
	init_ztable();
	lwram_ldptr += (128 * 1024);
	//Declared Object List
	dWorldObjects = (void*)(lwram_ldptr);
	lwram_ldptr += sizeof(_declaredObject) * MAX_WOBJS;
	lwram_ldptr = align_4(lwram_ldptr);
	//Building Item Scratchpad List
	BuildingPayload = (void*)(lwram_ldptr);
	lwram_ldptr += sizeof(_buildingObject) * MAX_BUILD_OBJECTS;
	lwram_ldptr = align_4(lwram_ldptr);
	
	
}

