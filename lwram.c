//lwram.c
//this file is included in main.c

void	init_division_table(void)
{
	for(int i = 0; i < 65536; i++)
	{
		division_table[i] = 65536 / i;
	}
}

void	init_lwram(void)
{
	currentAddress = (void*)LWRAM; //Initialize loading pointer
//Overwritten loading buffer
	dirty_buf = (void*)(LWRAM+1048576)-65536; 
//Actual Main Map
	main_map =  (void*)(dirty_buf-65536);
//Ready Loaded Map
	buf_map = (void*)(main_map-65536);
//Textue Definitions
	//In LWRAM because why use HWRAM for it? It is frequently cached data, anyhow. // 2048 bytes
	pcoTexDefs = (void*)((unsigned int)(buf_map-(sizeof(paletteCode) * 1024)));//|UNCACHE); 
//Sound Control Data Table
	RBBs = (void*)((unsigned int)(pcoTexDefs-(sizeof(_boundBox) * MAX_PHYS_PROXY))|UNCACHE); //In LWRAM // 
//Object Table
	dWorldObjects = (void*)((unsigned int)(RBBs-(sizeof(_declaredObject) * MAX_WOBJS)));//^UNCACHE);//|UNCACHE); //In LWRAM // 12KBish
//Building (Source Data) Object Table
	BuildingPayload = (void*)((unsigned int)(dWorldObjects-(sizeof(_buildingObject) * MAX_BUILD_OBJECTS)));
//Map Normal Table
	normTbl = (void*)((unsigned int)(LWRAM+(512 * 1024)));//^UNCACHE);//|UNCACHE); //In LWRAM // 192KB
// 65536/x table // 128KB // 256KB into RAM
	division_table = (void*)((unsigned int)(LWRAM+(256 * 1024)));

	init_division_table();

//I have detected a 'black spot' in LWRAM, between 256 and 512KB from the end of LWRAM is what seems to be an illegal area.
}

