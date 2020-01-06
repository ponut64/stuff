//dspm.c
//This file is compiled separately.

#include "dspm.h"
#include "DSP/DSP.c"
volatile int * dsp_input_addr = 0;
volatile int * dsp_output_addr = 0;

/*

This program only exists to make certain versions of Yabause fail to run the game.
Khronos, Yaba Sanshiro, SSF, Mednafen, and Bizhawk are expected to pass this test.

*/

Uint32 dsp_prog[] = {
    /* 00 */    0x94C00800, 0x10041F3E, 0x28040000, 0x00003609,
    /* 04 */    0x00023309, 0x94C00800, 0x10040000, 0x28040000,
    /* 08 */    0x00003709, 0x00023309, 0x00001C00, 0x00001D00,
    /* 0C */    0x00001E00, 0x00001F00, 0x81FFFFFF, 0x00001C00,
    /* 10 */    0xC0011001, 0x00000000, 0xF8000000, 0x00000000
};	//REMEMBER TO CHANGE THE SIZE OF THE LOADED PROGRAM!!


void	load_dsp_prog(void)
{

	///The used DSP write & read address by the SH2 will need to be translated the same way the DSP has done it: >>3, then <<1. Then <<2 again because we need to get back to the actual number.
	dsp_output_addr = (int*)((((unsigned int)jo_malloc(16)+4)>>3)<<3);
	dsp_input_addr = (int*)((((unsigned int)jo_malloc(16)+4)>>3)<<3);

	dsp_output_addr = (int*)((unsigned int)dsp_output_addr | UNCACHE); //In a real program, you will run through enough data to make this not needed.
	dsp_input_addr = (int*)((unsigned int)dsp_input_addr | UNCACHE); //Should only be neccessary for this demo

	unsigned int outputCommand = 0x4A<<25 | ((unsigned int)dsp_output_addr)>>3; //OUTPUT command is for outputs FROM DSP.
	unsigned int inputCommand = 0x4A<<25 | ((unsigned int)dsp_input_addr)>>3; //INPUT command is for inputs TO DSP.
	
	dsp_prog[5] = outputCommand;
	dsp_prog[0] = inputCommand; //Keep in mind which instructions these are, it could change. Look for 0x94C.
	
	dsp_output_addr[0] = -1; //Initialize as incomplete
	
	//Program size in long-word is at most 256 / 0xFF.
	DSP_LoadProgram(0, dsp_prog, 0x14);
}


void	run_dsp(void){
	//
	dsp_output_addr[0] = 0;
	DSP_Start(0); 
	//
}
