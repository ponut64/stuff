//lwram.c
//this file is included in main.c


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
	dWorldObjects = (void*)((unsigned int)(RBBs-(sizeof(_declaredObject) * 512))|UNCACHE); //In LWRAM // 11264 bytes
//Total consumed LWRAM: about 215 KB

}

