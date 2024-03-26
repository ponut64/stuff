//dspm.c
//This file is compiled separately.

#include <sl_def.h>
#include "def.h"
#include "dspm.h"
#include "render.h"
#include "DSP/DSP.c"
volatile int * dsp_input_addr = 0;
volatile int * dsp_noti_addr = 0;
volatile int * dsp_output_addr = 0;

//Artbitary number used here.
volatile unsigned int __attribute__ ((aligned (8))) dsp_data_stack[64 + (1024 * 3)];

//Chirality in/out of portal in polygon list
Uint32 winder_prog[] = {
0x00001D00, 0x84C00800, 0x84C00800, 0x84C00800, 0x84C00800, 0x00001D00, 
0x00075F3C, 0x28040000, 0x00003609, 0x00077309, 0x28041E00, 0x00077309, 
0x28041C00, 0x00077309, 0x28041D00, 0x00003309, 0x00021F1E, 0x00003309, 
0xC0014008, 0x00001F3E, 0x00001C00, 0x00001F3D, 0x0006D503, 0x00001F3A, 
0x00003309, 0x00001F3D, 0x18043603, 0x00003709, 0x00003309, 0x00001C00, 
0x01820000, 0x18041501, 0xD31800D0, 0x00000000, 0x14041E00, 0x00023009, 
0xC0014204, 0x00001C01, 0x00061E14, 0x00003209, 0x00001F3E, 0x0006DF3B, 
0x00003309, 0x00001E14, 0x01A21D00, 0x10000000, 0xD30800C3, 0x00069501, 
0x14041F3B, 0x00023209, 0x00003603, 0xC001410E, 0x0006D50E, 0x10040000, 
0x00003309, 0x00001D0D, 0x94000100, 0x00064000, 0x04000000, 0xD308002B, 
0x00000000, 0x00001E02, 0x01968000, 0x14041E01, 0xD31000BD, 0x00069E04, 
0x00003209, 0x00001D00, 0x00065D04, 0x00067209, 0x00003209, 0x00001D01, 
0x00065D10, 0x00003109, 0x00001D03, 0x00065D11, 0x00003109, 0x00001E00, 
0x00069D12, 0x00023109, 0xA8000001, 0xB00000D7, 0x00064000, 0x94000200, 
0x04000000, 0xD308005E, 0x01B20000, 0x18041C04, 0xD2100061, 0x00001E03, 
0x01868000, 0x08040000, 0x00003209, 0xD0000061, 0x01B20000, 0x18040000, 
0xD31000BD, 0x00001E05, 0x00001D03, 0x00065D07, 0x00067209, 0x00003209, 
0x00001D04, 0x00065D10, 0x00003109, 0x00001D06, 0x00065D11, 0x00003109, 
0xA8000001, 0xB00000D7, 0x00064000, 0x94000200, 0x04000000, 0xD308007A, 
0x01B20000, 0x18041C05, 0xD210007D, 0x00001E03, 0x01868000, 0x08040000, 
0x00003209, 0xD000007D, 0x01B20000, 0x18040000, 0xD31000BD, 0x00001E05, 
0x00001D06, 0x00065D0A, 0x00067209, 0x00003209, 0x00001D07, 0x00065D10, 
0x00003109, 0x00001D09, 0x00065D11, 0x00003109, 0xA8000001, 0xB00000D7, 
0x00064000, 0x94000200, 0x04000000, 0xD3080096, 0x01B20000, 0x18041C06, 
0xD2100099, 0x00001E03, 0x01868000, 0x08040000, 0x00003209, 0xD0000099, 
0x01B20000, 0x18040000, 0xD31000BD, 0x00001E05, 0x00001D09, 0x00065D01, 
0x00067209, 0x00003209, 0x00001D0A, 0x00065D10, 0x00003109, 0x00001D00, 
0x00065D11, 0x00003109, 0xA8000001, 0xB00000D7, 0x00064000, 0x94000200, 
0x04000000, 0xD30800B2, 0x01B20000, 0x18041C07, 0xD21000BD, 0x00001E03, 
0x01868000, 0x08040000, 0x00003209, 0xD00000BD, 0x01B20000, 0x18041C03, 
0xD31000BD, 0x00001E03, 0x01868000, 0x08041F1E, 0x00023209, 0x0006D501, 
0x18040000, 0x00003309, 0xD000002B, 0x00001E03, 0x00001C02, 0x01868000, 
0x08040000, 0x00023209, 0xD000002B, 0x00001E03, 0xC0015201, 0x00021F3D, 
0x0006D504, 0x10040000, 0x00003709, 0x00023309, 0x00001F3A, 0x0006D504, 
0x10040000, 0x00003609, 0x00003309, 0xD000001D, 0x00001F3F, 0x00003703, 
0x00001F1D, 0x8C000001, 0x00001F1D, 0xC0015302, 0xF0000000, 0x00001D10, 
0x00021E04, 0x01D79F00, 0x14040000, 0x00023309, 0x01A74000, 0x14041F00, 
0x0008F409, 0x01021E07, 0x18040000, 0x00023209, 0x00001E05, 0x01E64000, 
0x14041D10, 0x00023309, 0x01968000, 0x14041F00, 0x0009F409, 0x01021D0D, 
0x18041501, 0x00023309, 0x00065E07, 0x04001F01, 0xD30800F5, 0x00020000, 
0x01F69C00, 0x14041D0D, 0x00023309, 0xD00000F8, 0x00000000, 0x01A7DC00, 
0x14041D0D, 0x00023309, 0xE0000000, 0x00001F02, 0x00000000
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
	
	//Now, for the winder program.
	void * vertsAddress = (void*)&msh2VertArea[0];
	void * portalAddress = (void*)&used_portals[0];
	vertsAddress = (void*)((unsigned int)vertsAddress | UNCACHE);
	portalAddress = (void*)((unsigned int)portalAddress | UNCACHE);
	
	unsigned int vertsCommand = 0x80<<24 | ((unsigned int)vertsAddress)>>3;
	unsigned int portalCommand = 0x80<<24 | ((unsigned int)portalAddress)>>3;
	notiCommand = 0x80<<24 | ((unsigned int)dsp_noti_addr)>>3; //NOTI command is for DSP notification end status.
	inputCommand = 0x80<<24 | ((unsigned int)dsp_input_addr)>>3; //INPUT command is for inputs TO DSP.
	
	winder_prog[1] = inputCommand; //Keep in mind which instructions these are, it could change. Look for 0x94C.
	winder_prog[2] = vertsCommand;
	winder_prog[3] = portalCommand;
	winder_prog[4] = notiCommand;
	
}

void	load_winder_prog(void)
{
	//Program size in long-word is at most 256 / 0xFF.
	DSP_LoadProgram(0, winder_prog, sizeof(winder_prog)>>2);
}

void	run_winder_prog(void)
{
	// dsp_input_addr[0] = LCL_MAP_PIX * LCL_MAP_PIX;
	// dsp_input_addr[1] = 1; 
	// dsp_input_addr[2] = DSP_CLIP_CHECK;
	// dsp_input_addr[3] = DSP_CLIP_IN;
	// dsp_input_addr[4] = SCRN_CLIP_NY;
	// dsp_input_addr[5] = SCRN_CLIP_X;
	// dsp_input_addr[6] = SCRN_CLIP_Y;
	// dsp_input_addr[7] = SCRN_CLIP_NX;
	// dsp_noti_addr[0] = 0;
	// dsp_noti_addr[1] = 0;
	// DSP_Start(0); 
}

