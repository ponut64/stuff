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
volatile unsigned int __attribute__ ((aligned (8))) dsp_data_stack[128];

//Chirality in/out of portal in polygon list
Uint32 winder_prog[] = {
0x00001D00, 0x84C00800, 0x00001D00, 0x00075F3C, 0x28041E00, 0x00003609, 
0x94000003, 0x10041C00, 0x00003309, 0xC0010303, 0x00021F1E, 0x00003309, 
0xC0014008, 0x00001F3E, 0x00001C00, 0x00001F3D, 0x0006D503, 0x00001F3A, 
0x00003309, 0x00001F3D, 0x18043603, 0x00003709, 0x00003309, 0x00001C00, 
0x01820000, 0x18041501, 0xD31800C3, 0x00000000, 0x14041E00, 0x00023009, 
0xC0014204, 0x00001E03, 0x00069D15, 0x00003109, 0x00001C01, 0x00061E14, 
0x00003209, 0x00001F3E, 0x0006DF3B, 0x00003309, 0x00001E14, 0x01A21D00, 
0x10000000, 0xD30800B1, 0x00069501, 0x14041F3B, 0x00023209, 0x00003603, 
0xC001410E, 0x0006D50E, 0x10040000, 0x00003309, 0x00001D0D, 0x94000100, 
0x00064000, 0x04000000, 0xD3080028, 0x00001D14, 0x84000000, 0x00001E01, 
0x00069E04, 0x00003209, 0x00001D00, 0x00065D04, 0x00067209, 0x00003209, 
0x00001D01, 0x00065D10, 0x00003109, 0x00001D03, 0x00065D11, 0x00003109, 
0x00001E00, 0x00069D12, 0x00023109, 0xA8000001, 0xB00000CA, 0x00000000, 
0x01B20000, 0x18041C04, 0xD2180055, 0x00001E03, 0x01868000, 0x08040000, 
0x00003209, 0x01B21D14, 0x18041501, 0xD318005B, 0x00064000, 0x10040000, 
0x00003109, 0x00001E05, 0x00001D03, 0x00065D07, 0x00067209, 0x00003209, 
0x00001D04, 0x00065D10, 0x00003109, 0x00001D06, 0x00065D11, 0x00003109, 
0xA8000001, 0xB00000CA, 0x00000000, 0x01B20000, 0x18041C05, 0xD2180070, 
0x00001E03, 0x01868000, 0x08040000, 0x00003209, 0x01B21D14, 0x18041501, 
0xD3180076, 0x00064000, 0x10040000, 0x00003109, 0x00001E05, 0x00001D06, 
0x00065D0A, 0x00067209, 0x00003209, 0x00001D07, 0x00065D10, 0x00003109, 
0x00001D09, 0x00065D11, 0x00003109, 0xA8000001, 0xB00000CA, 0x00000000, 
0x01B20000, 0x18041C06, 0xD218008B, 0x00001E03, 0x01868000, 0x08040000, 
0x00003209, 0x01B21D14, 0x18041501, 0xD3180091, 0x00064000, 0x10040000, 
0x00003109, 0x00001E05, 0x00001D09, 0x00065D01, 0x00067209, 0x00003209, 
0x00001D0A, 0x00065D10, 0x00003109, 0x00001D00, 0x00065D11, 0x00003109, 
0xA8000001, 0xB00000CA, 0x00000000, 0x01B20000, 0x18041C07, 0xD21800A6, 
0x00001E03, 0x01868000, 0x08040000, 0x00003209, 0x01B21D14, 0x18041501, 
0xD3180028, 0x00064000, 0x10041504, 0x00003109, 0x14040000, 0xD3100028, 
0x00000000, 0x00065E03, 0x00003209, 0x00001E03, 0x00001C02, 0x01868000, 
0x08040000, 0x00023209, 0x00001E03, 0xC0015201, 0x00021F3D, 0x0006D504, 
0x10040000, 0x00003709, 0x00023309, 0x00001F3A, 0x0006D504, 0x10040000, 
0x00003609, 0x00003309, 0xD0000017, 0x00001F3F, 0x00003703, 0x00001F1D, 
0x8C000001, 0x00001F1D, 0xC0015302, 0xF0000000, 0x00001D10, 0x00021E04, 
0x01D79F00, 0x14040000, 0x00023309, 0x01A74000, 0x14041F00, 0x0008F409, 
0x01021E07, 0x18040000, 0x00023209, 0x00001E05, 0x01E64000, 0x14041D10, 
0x00023309, 0x01968000, 0x14041F00, 0x0009F409, 0x01021D0D, 0x18041501, 
0x00023309, 0x00065E07, 0x04001F01, 0xD30800E8, 0x00020000, 0x01F69C00, 
0x14041D0D, 0x00023309, 0xD00000EB, 0x00000000, 0x01A7DC00, 0x14041D0D, 
0x00023309, 0xE0000000, 0x00001F02, 0x00000000
};

void	init_dsp_programs(void)
{

	///The used DSP write & read address by the SH2 will need to be translated the same way the DSP has done it:
	///>>3, then <<1. Then <<2 again because we need to get back to the actual number.
	dsp_noti_addr = (int*)((int)&dsp_data_stack[8]);
	dsp_input_addr = (int*)((int)&dsp_data_stack[32]);
	dsp_output_addr = (int*)((int)&dsp_data_stack[64]);

	dsp_noti_addr = (int*)((unsigned int)dsp_noti_addr | UNCACHE); //In a real program, you will run through enough data to make this not needed.
	dsp_input_addr = (int*)((unsigned int)dsp_input_addr | UNCACHE); //Should only be neccessary for this demo
	dsp_output_addr = (int*)((unsigned int)dsp_output_addr | UNCACHE);

	//We, uh, create an instruction to do this. Bit whacky. (move immediate instruction) 
	unsigned int inputCommand = 0x80<<24 | ((unsigned int)dsp_input_addr)>>3;
	winder_prog[1] = inputCommand; //Keep in mind which instructions these are, it could change. Look for 0x94C.
	
	DSP_LoadProgram(0, winder_prog, sizeof(winder_prog)>>2);
}

void	load_winder_prog(void)
{
	//Program size in long-word is at most 256 / 0xFF.
	DSP_LoadProgram(0, winder_prog, sizeof(winder_prog)>>2);
}

void	run_winder_prog(int num_verts, int * num_portals, void * vertices)
{
	dsp_input_addr[0] = ((unsigned int)vertices)>>2;
	dsp_input_addr[1] = ((unsigned int)used_portals)>>2;
	dsp_input_addr[2] = ((unsigned int)&dsp_data_stack[8])>>2;
	dsp_input_addr[3] = num_verts;
	dsp_input_addr[4] = *num_portals; 
	dsp_input_addr[5] = DSP_CLIP_CHECK;
	dsp_input_addr[6] = DSP_CLIP_IN;
	dsp_input_addr[7] = DSP_PORT_01;
	dsp_input_addr[8] = DSP_PORT_12;
	dsp_input_addr[9] = DSP_PORT_23;
	dsp_input_addr[10] = DSP_PORT_30;
	dsp_noti_addr[0] = 0;
	dsp_noti_addr[1] = 0;
	DSP_Start(0); 
}

