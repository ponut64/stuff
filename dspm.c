//dspm.c
//This file is compiled separately.

#include "dspm.h"
#include "DSP/DSP.c"
volatile int * dsp_input_addr = 0;
volatile int * dsp_noti_addr = 0;
volatile int * dsp_output_addr = 0;

/*
Heightmap Local Area Table Generator
See hmap2.asm in DSP folder
*/

Uint32 dsp_prog[] = {
    /* 00 */    0x94C00800, 0x10041F3E, 0x28041C00, 0x00003609,
    /* 04 */    0x00023309, 0x94C00800, 0x10041D02, 0x28041E00,
    /* 08 */    0x00003709, 0x00023309, 0x94C00800, 0x18041F3A,
    /* 0C */    0x28041100, 0x00023309, 0xC0010205, 0x00001E00,
    /* 10 */    0x00001D00, 0x01E21C04, 0x180D940C, 0x21041E01,
    /* 14 */    0x1484150C, 0x14041D02, 0x00023009, 0x94000019,
    /* 18 */    0x14021D02, 0xD218006E, 0x00001C04, 0x02A87504,
    /* 1C */    0x19041F00, 0x18040000, 0x00023009, 0x000894FF,
    /* 20 */    0x01001E04, 0x180C8000, 0x00023409, 0x01001D01,
    /* 24 */    0x18041E01, 0x00023009, 0x00001C05, 0x01801C00,
    /* 28 */    0x18041100, 0x00023009, 0x00003006, 0x00001C00,
    /* 2C */    0x00001D00, 0xA8000001, 0xB0000076, 0x00000000,
    /* 30 */    0x00001D01, 0x00001C07, 0x00023005, 0x00001D03,
    /* 34 */    0x00001100, 0x94000019, 0x14021D03, 0xD3080061,
    /* 38 */    0x00000000, 0x01901C05, 0x19841E03, 0x19A41C08,
    /* 3C */    0x18040000, 0x00023009, 0x00001C08, 0x01801E01,
    /* 40 */    0x18041C07, 0x02088000, 0x01000000, 0x14041C09,
    /* 44 */    0x00023009, 0x00001C08, 0x01C01C06, 0x19C40000,
    /* 48 */    0x18041C0A, 0x00023009, 0x00001E00, 0x00001C0A,
    /* 4C */    0x01868000, 0x14020000, 0x831FFFFF, 0x18021C0A,
    /* 50 */    0x8317FFFF, 0x00001E01, 0x00001C09, 0x01C68000,
    /* 54 */    0x14020000, 0x831FFFFF, 0x18041C0A, 0x8317FFFF,
    /* 58 */    0x00021C0A, 0x00003304, 0x00001D03, 0x01921C08,
    /* 5C */    0x18841501, 0x18041E02, 0x00003109, 0xD0000035,
    /* 60 */    0x00000000, 0x00001F00, 0xC0011319, 0x00001F3F,
    /* 64 */    0x01B00000, 0x18841519, 0x18040000, 0x00023309,
    /* 68 */    0x00001D02, 0x01921C05, 0x18841501, 0x18041E01,
    /* 6C */    0x00003109, 0xD0000017, 0x00000000, 0x00001F3A,
    /* 70 */    0x00003707, 0x00001C00, 0x00001020, 0x00001C00,
    /* 74 */    0xC0011001, 0xF8000000, 0x01C21D00, 0x19C41101,
    /* 78 */    0x00023009, 0x18040000, 0x00023009, 0x00001C02,
    /* 7C */    0x00070000, 0x01C01D00, 0x14021C03, 0xD3180097,
    /* 80 */    0x87100000, 0xD0000087, 0x00061D00, 0x28040000,
    /* 84 */    0x00067009, 0x28041C03, 0x00023109, 0x00061C02,
    /* 88 */    0x01800000, 0x28040000, 0x14041D00, 0xD3100082,
    /* 8C */    0x00021C03, 0x01D00000, 0x00064000, 0x18041C02,
    /* 90 */    0x00023109, 0x00071D00, 0x01800000, 0x14041C00,
    /* 94 */    0x00023009, 0xD0000076, 0x00001C00, 0x00021D00,
    /* 98 */    0x00074000, 0x01901C00, 0x18040000, 0x00023109,
    /* 9C */    0x00001D01, 0xE0000000, 0x00000000
};	//REMEMBER TO CHANGE THE SIZE OF THE LOADED PROGRAM!!


void	load_dsp_prog(void)
{

	///The used DSP write & read address by the SH2 will need to be translated the same way the DSP has done it: >>3, then <<1. Then <<2 again because we need to get back to the actual number.
#pragma GCC push_options
#pragma GCC diagnostic ignored "-Wbad-function-cast"
	dsp_noti_addr = (int*)((((int)jo_malloc(64)+8)>>3)<<3);
	dsp_input_addr = (int*)((((int)jo_malloc(64)+8)>>3)<<3);
	dsp_output_addr = (int*)((((int)jo_malloc((LCL_MAP_PIX * LCL_MAP_PIX)<<2)+8)>>3)<<3);
#pragma GCC pop_options

	dsp_noti_addr = (int*)((unsigned int)dsp_noti_addr | UNCACHE); //In a real program, you will run through enough data to make this not needed.
	dsp_input_addr = (int*)((unsigned int)dsp_input_addr | UNCACHE); //Should only be neccessary for this demo
	dsp_output_addr = (int*)((unsigned int)dsp_output_addr | UNCACHE);

	unsigned int notiCommand = 0x4A<<25 | ((unsigned int)dsp_noti_addr)>>3; //NOTI command is for DSP notification end status.
	unsigned int inputCommand = 0x4A<<25 | ((unsigned int)dsp_input_addr)>>3; //INPUT command is for inputs TO DSP.
	unsigned int outputCommand = 0x4A<<25 | ((unsigned int)dsp_output_addr)>>3; //OUTPUT command is for outputs FROM DSP.
	
	dsp_prog[0] = inputCommand; //Keep in mind which instructions these are, it could change. Look for 0x94C.
	dsp_prog[5] = outputCommand;
	dsp_prog[10] = notiCommand;

	
	dsp_noti_addr[0] = 1; //Initialize as complete
	
	//Program size in long-word is at most 256 / 0xFF.
	DSP_LoadProgram(0, dsp_prog, sizeof(dsp_prog)>>2);
}


void	run_dsp(void){
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
