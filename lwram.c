//lwram.c
//this file is included in main.c

void	init_division_table(void)
{
	division_table[0] = 65535;
	for(int i = 1; i < 65536; i++)
	{
		division_table[i] = 65536 / i;
	}
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
// Actual Main Map
	main_map =  (void*)(dirty_buf-65536);
// Ready Loaded Map
	buf_map = (void*)(main_map-65536);
// Textue Definitions
	//In LWRAM because why use HWRAM for it? It is frequently cached data, anyhow. // 4096 bytes
	pcoTexDefs = (void*)((unsigned int)(buf_map-(sizeof(paletteCode) * 2048)));
// Object Table
	dWorldObjects = (void*)((unsigned int)(pcoTexDefs-(sizeof(_declaredObject) * MAX_WOBJS))); //In LWRAM // 12KBish
// Building (Source Data) Object Table
	BuildingPayload = (void*)((unsigned int)(dWorldObjects-(sizeof(_buildingObject) * MAX_BUILD_OBJECTS)));
//Space used from end of LWRAM: about 256 KB
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Map Normal Table
	normTbl = (void*)((unsigned int)(LWRAM + (128 * 1024))); //In LWRAM // 192KB
// 65536/x table // 128KB // 256KB into RAM
	division_table = (void*)((unsigned int)(normTbl + (192 * 1024)));
// Map Texture Table
	mapTex = (void*)((unsigned int)(division_table + (128 * 1024))); //In LWRAM // 128KB
//Space used from beginning of LWRAM: About 448 KB
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	init_division_table();

//I have detected a 'black spot' in LWRAM, between bytes 512 - 768 of LWRAM /something/ bad happens.
}

