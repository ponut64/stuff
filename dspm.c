//dspm.c
//This file is compiled separately.

#include "dspm.h"
#include "DSP/DSP.c"
volatile int * dsp_input_addr = 0;
volatile int * dsp_output_addr = 0;
volatile int * dspNotiEndAddr = 0;
volatile Uint32 * dspCtrlPrt = (Uint32*)0x25FE0080;

Uint32 normal_prog[] = {
    /* 00 */    0x94C00800, 0x10041F3E, 0x28041C00, 0x00003709,
    /* 04 */    0x00023309, 0x94C00800, 0x10041E00, 0x28041D00,
    /* 08 */    0x00003609, 0x00023309, 0x94C00800, 0x10041F3B,
    /* 0C */    0x28040000, 0x00023309, 0xC0010301, 0x00001300,
    /* 10 */    0x00001F3F, 0x0086D501, 0x10041D07, 0x00003609,
    /* 14 */    0x00023309, 0xC0010101, 0x00021D07, 0x00065C00,
    /* 18 */    0x94FF0000, 0x04040000, 0x00023009, 0x00064000,
    /* 1C */    0x940000FF, 0x04041A0F, 0xE8000000, 0x28040000,
    /* 20 */    0x00023009, 0x90010000, 0x9400FF00, 0x18041D08,
    /* 24 */    0x00003109, 0x00001D08, 0x00085D07, 0x01065E00,
    /* 28 */    0x04041A02, 0xE8000000, 0x3C040000, 0x00023009,
    /* 2C */    0x9400FF00, 0x00065D00, 0x04041A07, 0xE8000000,
    /* 30 */    0x28041F00, 0x00023009, 0xD0000081, 0x00021C00,
    /* 34 */    0x00069A05, 0xE8000000, 0x20041C00, 0x00023209,
    /* 38 */    0x00069A05, 0xE8000000, 0x20041D00, 0x00023209,
    /* 3C */    0x00069A05, 0xE8000000, 0x20041F00, 0x00023209,
    /* 40 */    0x00001E03, 0x00001200, 0x00001E00, 0x01A00000,
    /* 44 */    0x18040000, 0xD210004B, 0x95FFFFFF, 0x10041E03,
    /* 48 */    0x0C041201, 0x00001E00, 0x00023209, 0x00021E02,
    /* 4C */    0x01A00000, 0x18040000, 0xD2100056, 0x95FFFFFF,
    /* 50 */    0x10040000, 0x0C040000, 0x00023209, 0x00869502,
    /* 54 */    0x10040000, 0x00023209, 0x00001E03, 0x00069E00,
    /* 58 */    0x24040000, 0x24040000, 0x00099401, 0x01000000,
    /* 5C */    0x08040000, 0x90000400, 0x00098000, 0x01000000,
    /* 60 */    0x08040000, 0x90100000, 0x00098000, 0x01000000,
    /* 64 */    0x08041F3C, 0x00023209, 0x0007C000, 0x01F00000,
    /* 68 */    0x14021F00, 0xD3180079, 0x00001F3D, 0x0086D501,
    /* 6C */    0x18041E03, 0x00023309, 0xC0011201, 0x0086D501,
    /* 70 */    0x18040000, 0x00003309, 0x00023709, 0x0086D501,
    /* 74 */    0x18040000, 0x00003309, 0x00023609, 0xD0000015,
    /* 78 */    0x00001D07, 0x00001F3B, 0x00003707, 0x00001C00,
    /* 7C */    0x00001028, 0x00001C00, 0xC0011001, 0xF8000000,
    /* 80 */    0x00020000, 0x85E70000, 0x01C00000, 0x19C40000,
    /* 84 */    0x14040000, 0x00023109, 0x84190000, 0x88190000,
    /* 88 */    0x01C00000, 0x19C41C00, 0x14041D01, 0x00023209,
    /* 8C */    0x88190000, 0x00001E02, 0x02188000, 0x01001D02,
    /* 90 */    0x18041E01, 0x0002300A, 0x02188000, 0x01001D00,
    /* 94 */    0x18041E01, 0x0002300A, 0x02188000, 0x01001D01,
    /* 98 */    0x18041E00, 0x0002300A, 0x02188000, 0x01001D00,
    /* 9C */    0x18041E00, 0x0002300A, 0x00001C00, 0x01C00000,
    /* A0 */    0x19C40000, 0x14040000, 0xA8000007, 0xE8000000,
    /* A4 */    0x20040000, 0x00023309, 0x8C04E200, 0x01C00000,
    /* A8 */    0x19C40000, 0x14040000, 0xA8000007, 0xE8000000,
    /* AC */    0x20040000, 0x00023309, 0x00001F00, 0x0238C000,
    /* B0 */    0x01001F01, 0x1A3CC000, 0x01001F02, 0x1A3CC000,
    /* B4 */    0x01001F03, 0x18040000, 0x0000330A, 0x00001F03,
    /* B8 */    0x0006DC01, 0x28040000, 0x00023009, 0x00001C00,
    /* BC */    0xD00000C4, 0x80000001, 0x00861501, 0x18040000,
    /* C0 */    0x00023009, 0x00060000, 0x28040000, 0x00023009,
    /* C4 */    0xD22000BE, 0x00021C00, 0x94000011, 0x19840000,
    /* C8 */    0x14040000, 0x20841510, 0x00023009, 0x18041C00,
    /* CC */    0x01800000, 0x14841501, 0x00023A09, 0x18040000,
    /* D0 */    0xE8000000, 0x28041C00, 0x20040000, 0x01B23109,
    /* D4 */    0x18041D00, 0x20040000, 0x00023009, 0x02185C00,
    /* D8 */    0x01000000, 0x18040000, 0x000A340A, 0x01000000,
    /* DC */    0x18041C02, 0x0002300A, 0x94018000, 0x18041C02,
    /* E0 */    0x01800000, 0x14040000, 0x000A7409, 0x01001F00,
    /* E4 */    0x18041C00, 0x0000310A, 0x0002340A, 0x00001D00,
    /* E8 */    0x0009C000, 0x0109C000, 0x190DC000, 0x0002320A,
    /* EC */    0x19040000, 0x0002320A, 0x18040000, 0x0002320A,
    /* F0 */    0xD0000033, 0x00001E00
};	//REMEMBER TO CHANGE THE SIZE OF THE LOADED PROGRAM!!


void	load_dsp_prog(void)
{

	///The used DSP write & read address by the SH2 will need to be translated the same way the DSP has done it: >>3, then <<1. Then <<2 again because we need to get back to the actual number.
	dspNotiEndAddr = (int*)((((unsigned int)jo_malloc(8)+4)>>3)<<3);
	dsp_input_addr = (int*)((((unsigned int)jo_malloc(2400)+4)>>3)<<3);
	dsp_output_addr = (int*)((((unsigned int)jo_malloc(2400)+4)>>3)<<3);

	dspNotiEndAddr = (int*)((unsigned int)dspNotiEndAddr | 0x20000000); //Cache Avoidance?
	dsp_input_addr = (int*)((unsigned int)dsp_input_addr | 0x20000000); //Should only be neccessary for this demo
	dsp_output_addr = (int*)((unsigned int)dsp_output_addr | 0x20000000); //In a real program, you will run through enough data to make this not needed.
	

	unsigned int outputCommand = 0x4A<<25 | ((unsigned int)dsp_output_addr)>>3;
	unsigned int inputCommand = 0x4A<<25 | ((unsigned int)dsp_input_addr)>>3;
	unsigned int dspNotiEndCommand = 0x4A<<25 | ((unsigned int)dspNotiEndAddr)>>3;
	
	normal_prog[0] = outputCommand;
	normal_prog[5] = inputCommand; //Keep in mind which instructions these are, it could change. Look for 0x94C.
	normal_prog[10] = dspNotiEndCommand; 
	
	dspNotiEndAddr[0] = 40; //Initialize as complete so the program can continue for its first frame
	
	//Program size in long-word is at most 256 / 0xFF.
	DSP_LoadProgram(0, normal_prog, 0xF2);
}

void	rt_fail(void){
	if((dspCtrlPrt[0] & 4456448) == 4456448){
		jo_printf(12, 10, "DSP HEARTBEAT FAILURE");
	}
}


void	run_dsp(void){
	//
	dspNotiEndAddr[0] = 0;
	DSP_Start(0); 
	//
}
