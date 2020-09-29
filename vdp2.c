
#include <jo/jo.h>
#include "render.h"
#include "def.h"

/*
the biggest reason we're here is to integrate flat shaded render path using color calculation with line color screen
and the first way to do that, is to set up the line color screen
and along the way, i wanted to set up other layers
nbg0: 16bpp bitmap [sprite copy layer] [ext. color calculation with SPR layer + LNCL per pixel, or just cc with spr?] [512x256]
nbg1: 8bpp bitmap [HUD graphic layer] [512x256]
rbg0: 8bpp bitmap [water layer?] [512x256] --
must implement sprite text draw library to liberate from having to use character map layer

res: normal

theoretical layout:
NBG0: VRAM A0, spans into VRAM A1
NBG1: VRAM B0 [half of it, 64KB]
RBG0: VRAM B1 [half of it, 64KB]


BACK screen data: in the off-screen area of NBG1 in VRAM B0 [112 KB in to B0]
LNCL screen data: after the BACK screen data
Coefficient Parameters A: after the LNCL screen data

CYCLE PATTERN THEORIES:
|	T0	|	T1	|	T2	|	T3	|	T4	|	T5	|	T6	|	T7	|
////////////////////////	VRAM A0		/////////////////////////
|	0x0	|	0x4	|	0x4	|	0x4	|	0x4	|	0x16|	0x14|	0x14| NBG0 as 512x256 16bpp
|	N0P	|	N0B |	N0B |	N0B |	N0B |	NA |	CPU |	CPU	|
////////////////////////	VRAM A1		/////////////////////////
|	0x0	|	0x4	|	0x4	|	0x4	|	0x4	|	0x16|	0x14|	0x14| [spills into A1]
|	N0P	|	N0B |	N0B |	N0B |	N0B |	NA |	CPU |	CPU	|
////////////////////////	VRAM B0		/////////////////////////
|	0x1	|	0x5	|	0x5	|	0x16|	0x16|	0x16|	0x14|	0x14| NBG1 as 512x256 8bpp
|	N1P	|	N1B |	N1B |	NA |	NA |	NA |	CPU |	CPU	|
////////////////////////	VRAM B1		/////////////////////////
|						DOMINATED BY RBG0						| RBG0 as 512x256 8bpp [or 8bpp, tbh, no difference]

pg 149, vdp2 manual, RBG0 RAM data bank select information
ROTATION DATA BANK SELECT REGISTER SETTING: |= 192 [Use VRAM B1 for bitmap pattern, nothing else is needed.]
													[Coefficient table will not be used per line]
*/

//Write-only register! of CRAMoffset[1], bits 6,5,4 control SPR layer offset.
unsigned short * vdp2_CRAMoffset = (unsigned short *)(0x1800E4 + VDP2_RAMBASE);
//
// Display System Data
unsigned short * vdp2_TVmode = (unsigned short *)(0x180000 + VDP2_RAMBASE);
// Shadow Setting Register
unsigned short * vdp2_shadow = (unsigned short *)(0x1800E2 + VDP2_RAMBASE);
// Sprite Layer Data Type Register 
unsigned short * vdp2_sprMode = (unsigned short *)(0x1800E0 + VDP2_RAMBASE);
// Layer Display Register
unsigned short * vdp2_dispCtrl = (unsigned short *)(0x180020 + VDP2_RAMBASE); //pg 48, vdp2 manual
// Layer Color and Data Type Register
unsigned short * vdp2_layerTypeA = (unsigned short *)(0x180028 + VDP2_RAMBASE);	//pg 60 vdp2 manual
unsigned short * vdp2_layerTypeB = (unsigned short *)(0x18002A + VDP2_RAMBASE);	//pg 60 vdp2 manual
// Layer, bitmap mode, palette number register - pg 112 vdp2 manual
//												pg 95 - additional bitmap information
//												pg 87 - map offset register - THESE CONTROL THE BITMAP DATA LOCATION
//					** WARNING ** Changing bitmap / character map location necessitates VRAM cycle pattern change
//										SGL has set these and likely sets them at VBLANK, so be careful.
//										SGL probably also tells the system how to partition VRAM.
// Back Screen RAM Data Location Registers // Bit 15 of "hi" is 0 for single-color, 1 for color per line.
unsigned short * vdp2BackHi = (unsigned short *)(0x1800AC + VDP2_RAMBASE); //
unsigned short * vdp2BackLo = (unsigned short *)(0x1800AE + VDP2_RAMBASE); // Data at address is to be 15-bit RGB data.
// Line Color Screen RAM Data Location Registers // Bit 15 of "hi" is 0 for single-color, 1 for color per line.
unsigned short * vdp2LineColHi = (unsigned short *)(0x1800A8 + VDP2_RAMBASE); //
unsigned short * vdp2LineColLo = (unsigned short *)(0x1800AA + VDP2_RAMBASE); // Data at address is to be palette code [CRAM addr].
//
unsigned short * vdp2_stackptr = (unsigned short *)VDP2_RAMBASE;
//
unsigned short * lcl_backscrnptr = (unsigned short *)VDP2_RAMBASE;
//

unsigned short backColor = 0xE726;

void	vblank_requirements(void)
{
	//jo_printf(0, 15, "(%x)", (int)BACK_CRAM);
	vdp2_CRAMoffset[1] = 16; //Moves SPR layer color banks up in color RAM by 256 entries.
	//vdp2_TVmode[0] = 33027; //Set VDP2 to 704x224 [progressive scan, 704 width] - why? VDP2 will sharpen VDP1's output.
	vdp2_sprMode[0] = 4; //Sprite Data Type Mode
}

void	init_vdp2(void)
{
	slColRAMMode(2); //Set 24bpp
	//slZoomNbg0(32768, 65536);
	//slZoomNbg1(32768, 65536);
	//slScrAutoDisp(NBG0ON | NBG1ON);
	slScrAutoDisp(NBG0ON | NBG1ON);
	slPriorityNbg1(7); //Put NBG1 on top.
	slShadowOn(BACKON);
	
}
