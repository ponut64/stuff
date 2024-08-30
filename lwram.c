//lwram.c
//this file is included in main.c

void	init_ztable(void)
{
	//Plan:
	//For (scrn_dist), divide it by (-32767 to +32767)<<16
	for(int i = -32768; i < 32768; i++)
	{
		int sample = fxdiv(scrn_dist, i<<16);
		zTable[i] = (sample);
	}

	// :)
	zTable[0] = zTable[1];
}

void	init_lwram(void)
{
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
// Second buffer
	dirtier_buf = (void*)(dirty_buf)-65536; 
// Textue Definitions
	//In LWRAM because why use HWRAM for it? // 8kb
	pcoTexDefs = (void*)((unsigned int)(dirtier_buf-(sizeof(paletteCode) * 4096)));
// Object Table
	dWorldObjects = (void*)((unsigned int)(pcoTexDefs-(sizeof(_declaredObject) * MAX_WOBJS))); //In LWRAM // 12KBish
// Building (Source Data) Object Table
	BuildingPayload = (void*)((unsigned int)(dWorldObjects-(sizeof(_buildingObject) * MAX_BUILD_OBJECTS)));
//Pathing Table Heap. This is sized according to the max pathing step count, multiplied by the max active actors.
	pathTableHeap = (void*)((unsigned int)(BuildingPayload-(sizeof(_quad) * (MAX_PATHING_STEPS * MAX_PHYS_PROXY))));
//Adjacent Quad Table. This has an arbitrary size.
	adjacentPolyHeap = (void*)((unsigned int)(pathTableHeap - (32 * 1024)));
	adjPolyStackPtr = (void*)(adjacentPolyHeap);
	adjPolyStackMax = (void*)(pathTableHeap);
//Z-Table - this will be used with signed offsets; its size is actually 256kb
	zTable = (void*)((unsigned int)(adjacentPolyHeap - (128 * 1024)));
//Space used from end of LWRAM: about 512 KB
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	init_ztable();
//I have detected a 'black spot' in LWRAM, between bytes 512 - 768 of LWRAM /something/ bad happens.
//Pathing Guides - approx. 42 kb
	pathing = (void*)(LWRAM);
}

