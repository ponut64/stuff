//dspm.c
//This file is compiled separately.

#include <sl_def.h>
#include "def.h"
#include "hmap.h"
#include "dspm.h"
#include "render.h"
#include "DSP/DSP.c"
volatile int * dsp_input_addr = 0;
volatile int * dsp_noti_addr = 0;
volatile int * dsp_output_addr = 0;

volatile unsigned int __attribute__ ((aligned (8))) dsp_data_stack[64 + (LCL_MAP_PIX * LCL_MAP_PIX)];

/*
Heightmap Local Area Table Generator
See hmap2.asm in DSP folder
*/

Uint32 hmap_prog[] = {
0x94C00800, 0x10041F3E, 0x28041C00, 0x00003609, 0x00023309, 0x94C00800, 
0x10041D02, 0x28041E00, 0x00003709, 0x00023309, 0x94C00800, 0x10041F3A, 
0x28041100, 0x00023309, 0xC0014205, 0x00001E00, 0x00001D00, 0x01E21C04, 
0x180D940A, 0x21041E01, 0x1404150A, 0x14041D02, 0x00023009, 0x94000015, 
0x14021D02, 0xD218006E, 0x00001C04, 0x02287504, 0x19041F00, 0x18040000, 
0x00023009, 0x000894FF, 0x01001E04, 0x180C8000, 0x00023409, 0x01001D01, 
0x18041E01, 0x00023009, 0x00001C05, 0x01801C00, 0x18041100, 0x00023009, 
0x00003006, 0x00001C00, 0x00001D00, 0xA8000001, 0xB0000076, 0x00000000, 
0x00001D01, 0x00001C07, 0x00023005, 0x00001D03, 0x00001100, 0x94000015, 
0x14021D03, 0xD3080061, 0x00000000, 0x01901C05, 0x19841E03, 0x19A41C08, 
0x18040000, 0x00023009, 0x00001C08, 0x01801E01, 0x18041C07, 0x02088000, 
0x01000000, 0x14041C09, 0x00023009, 0x00001C08, 0x01C01C06, 0x19C40000, 
0x18041C0A, 0x00023009, 0x00001E00, 0x00001C0A, 0x01868000, 0x14020000, 
0x831FFFFF, 0x18021C0A, 0x8317FFFF, 0x00001E01, 0x00001C09, 0x01C68000, 
0x14020000, 0x831FFFFF, 0x18041C0A, 0x8317FFFF, 0x00021C0A, 0x00003304, 
0x00001D03, 0x01921C08, 0x18041501, 0x18041E02, 0x00003109, 0xD0000035, 
0x00000000, 0x00001F00, 0xC0011315, 0x00001F3F, 0x01B00000, 0x18041515, 
0x18040000, 0x00023309, 0x00001D02, 0x01921C05, 0x18041501, 0x18041E01, 
0x00003109, 0xD0000017, 0x00000000, 0x00001F3A, 0x00003707, 0x00001C00, 
0x00001020, 0x00001C00, 0xC0015001, 0xF8000000, 0x01C21D00, 0x19C41101, 
0x00023009, 0x18040000, 0x00023009, 0x00001C02, 0x00070000, 0x01C01D00, 
0x14021C03, 0xD3180097, 0x87100000, 0xD0000087, 0x00061D00, 0x28040000, 
0x00067009, 0x28041C03, 0x00023109, 0x00061C02, 0x01800000, 0x28040000, 
0x14041D00, 0xD3100082, 0x00021C03, 0x01D00000, 0x00064000, 0x18041C02, 
0x00023109, 0x00071D00, 0x01800000, 0x14041C00, 0x00023009, 0xD0000076, 
0x00001C00, 0x00021D00, 0x00074000, 0x01901C00, 0x18040000, 0x00023109, 
0x00001D01, 0xE0000000, 0x00000000
};	//REMEMBER TO CHANGE THE SIZE OF THE LOADED PROGRAM!!

Uint32 winder_prog[] = {
0x94C00800, 0x10041F3C, 0x28041C00, 0x00003609, 0x00023309, 0x94C00800, 
0x10041D00, 0x28041E00, 0x00000000, 0x00023309, 0x94C00800, 0x10040000, 
0x28040000, 0x00023309, 0x94C00800, 0x10040000, 0x28040000, 0x00003709, 
0x00023309, 0x00021F1E, 0x00003309, 0xC0014001, 0x00001F3E, 0x00003603, 
0xC0014110, 0x00001C00, 0x00001F3D, 0x0006D503, 0x18043603, 0x00003709, 
0x00003309, 0x00001D00, 0x01820000, 0x18041501, 0xD318007E, 0x00000000, 
0x14041E00, 0x00023009, 0xC0010204, 0x00001E01, 0x00069E04, 0x00003209, 
0x00001D00, 0x00065D04, 0x00067209, 0x00003209, 0x00001D01, 0x00065D10, 
0x00003109, 0x00001D03, 0x00065D11, 0x00003109, 0x00001E00, 0x00069D12, 
0x00023109, 0xA8000001, 0xB0000084, 0x00000000, 0x01B20000, 0x18040000, 
0xD3100077, 0x00001D03, 0x00065D07, 0x00067209, 0x00003209, 0x00001D04, 
0x00065D10, 0x00003109, 0x00001D06, 0x00065D11, 0x00003109, 0xA8000001, 
0xB0000084, 0x00000000, 0x01B20000, 0x18040000, 0xD3100077, 0x00001D06, 
0x00065D0A, 0x00067209, 0x00003209, 0x00001D07, 0x00065D10, 0x00003109, 
0x00001D09, 0x00065D11, 0x00003109, 0xA8000001, 0xB0000084, 0x00000000, 
0x01B20000, 0x18040000, 0xD3100077, 0x00001D09, 0x00065D01, 0x00067209, 
0x00003209, 0x00001D0A, 0x00065D10, 0x00003109, 0x00001D00, 0x00065D11, 
0x00003109, 0xA8000001, 0xB0000084, 0x00000000, 0x01B20000, 0x18040000, 
0xD3100077, 0x00001E03, 0x94000010, 0x00068000, 0x08041F1E, 0x00023209, 
0x00001E03, 0xC0015201, 0x0006D501, 0x18040000, 0x00003309, 0x00001F3D, 
0x0006D504, 0x18040000, 0x00003709, 0x00023309, 0xD0000020, 0x00001C00, 
0x00001F3F, 0x00003703, 0x00001F1E, 0xC0015301, 0xF0000000, 0x00000000, 
0x00001D10, 0x00021E04, 0x01D79F00, 0x14040000, 0x00023309, 0x01A74000, 
0x14041F00, 0x0008F409, 0x01021C02, 0x18040000, 0x00023009, 0x01E64000, 
0x14041D10, 0x00023309, 0x01968000, 0x14041F00, 0x0009F409, 0x01020000, 
0x18041D0D, 0x00023309, 0x94010000, 0x00065E05, 0x04041F01, 0xD30800A2, 
0x00021C02, 0x01F61C00, 0x14041D00, 0x00023309, 0xD00000A5, 0x00000000, 
0x0187DC00, 0x14041D00, 0x00023309, 0xE0000000, 0x00001F02, 0x00000000
};

void	init_dsp_programs(void)
{

	///The used DSP write & read address by the SH2 will need to be translated the same way the DSP has done it:
	///>>3, then <<1. Then <<2 again because we need to get back to the actual number.
	dsp_noti_addr = (int*)((int)&dsp_data_stack[8]);
	dsp_input_addr = (int*)((int)&dsp_data_stack[16]);
	dsp_output_addr = (int*)((int)&dsp_data_stack[32]);

	dsp_noti_addr = (int*)((unsigned int)dsp_noti_addr | UNCACHE); //In a real program, you will run through enough data to make this not needed.
	dsp_input_addr = (int*)((unsigned int)dsp_input_addr | UNCACHE); //Should only be neccessary for this demo
	dsp_output_addr = (int*)((unsigned int)dsp_output_addr | UNCACHE);

	unsigned int notiCommand = 0x4A<<25 | ((unsigned int)dsp_noti_addr)>>3; //NOTI command is for DSP notification end status.
	unsigned int inputCommand = 0x4A<<25 | ((unsigned int)dsp_input_addr)>>3; //INPUT command is for inputs TO DSP.
	unsigned int outputCommand = 0x4A<<25 | ((unsigned int)dsp_output_addr)>>3; //OUTPUT command is for outputs FROM DSP.
	
	hmap_prog[0] = inputCommand; //Keep in mind which instructions these are, it could change. Look for 0x94C.
	hmap_prog[5] = outputCommand;
	hmap_prog[10] = notiCommand;
	
	
	//Now, for the winder program.
	void * vertsAddress = (void*)&msh2VertArea[0];
	vertsAddress = (void*)((unsigned int)vertsAddress | UNCACHE);
	void * portalAddress = (void*)&portals[0];
	portalAddress = (void*)((unsigned int)portalAddress | UNCACHE);
	
	unsigned int vertsCommand = 0x4A<<25 | ((unsigned int)vertsAddress)>>3;
	unsigned int portalCommand = 0x4A<<25 | ((unsigned int)portalAddress)>>3;
	
	winder_prog[0] = inputCommand; //Keep in mind which instructions these are, it could change. Look for 0x94C.
	winder_prog[5] = vertsCommand;
	winder_prog[10] = portalCommand;
	winder_prog[14] = notiCommand;
	
}

void	load_hmap_prog(void)
{
	//Program size in long-word is at most 256 / 0xFF.
	DSP_LoadProgram(0, hmap_prog, sizeof(hmap_prog)>>2);
}

void	load_winder_prog(void)
{
	//Program size in long-word is at most 256 / 0xFF.
	DSP_LoadProgram(0, winder_prog, sizeof(winder_prog)>>2);
}

void	run_winder_prog(void)
{
	dsp_input_addr[0] = LCL_MAP_PIX * LCL_MAP_PIX;
	dsp_noti_addr[0] = 0;
	DSP_Start(0); 
}

void	run_hmap_prog(void)
{
	//
	dsp_input_addr[0] = main_map_total_pix;
	dsp_input_addr[1] = main_map_x_pix;
	dsp_input_addr[2] = main_map_y_pix;
	dsp_input_addr[3] = you.dispPos[X];
	dsp_input_addr[4] = you.dispPos[Y];
	dsp_noti_addr[0] = 0;
	DSP_Start(0); 
	//
}
