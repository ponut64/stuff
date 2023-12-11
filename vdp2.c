
#include <sl_def.h>
#include "mymath.h"
#include "render.h"
#include "def.h"
#include "tga.h"

#include "vdp2.h"

/*

*/

//Write-only register! of CRAMoffset[1], bits 6,5,4 control SPR layer offset.
unsigned short * vdp2_CRAMoffsetA = (unsigned short *)(0x1800E4 + VDP2_RAMBASE);
unsigned short * vdp2_CRAMoffsetB = (unsigned short *)(0x1800E6 + VDP2_RAMBASE);
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
unsigned short * back_scrn_colr_addr = (unsigned short *)(VDP2_RAMBASE + 512);
unsigned short	back_color_setting;
//

//Color RAM pointers (for both 16 bit mode and 24-bit mode)
unsigned int * cRAM_24bm = (unsigned int *)0x05F00000;
unsigned short * cRAM_16bm = (unsigned short *)0x05F00000;

//Screen Erasure Settings
	unsigned short * VDP1_EWLR = (unsigned short *) (VDP1_VRAM + 0x100008);
	unsigned short * VDP1_EWRR = (unsigned short *) (VDP1_VRAM + 0x10000A);

void	put_pixel_in_8bpp_layer(unsigned char * layer_address, unsigned short x, unsigned short y, unsigned char color_code, unsigned short line_width)
{
	int index = x + (y * line_width);
	layer_address[index] = color_code;	
}

void	draw_hud_pixel(short x, short y, unsigned char color_code)
{
		if(x < 0) x = 0;
		if(y < 0) y = 0;
        put_pixel_in_8bpp_layer((unsigned char *)NBG1_BITMAP_ADDR, x, y, color_code, 512);
}

//stolen from johannes
void	draw_hud_line(short x0, short y0, short x1, short y1, unsigned char color_code)
{
    int                         dx;
    int                         sx;
    int                         dy;
    int                         sy;
    int                         err;
    int                         e2;
	if(x0 < 0) x0 = 0;
	if(x1 < 0) x1 = 0;
	if(y0 < 0) y0 = 0;
	if(y1 < 0) y1 = 0;

    dx = JO_ABS(x1 - x0);
    sx = x0 < x1 ? 1 : -1;
    dy = JO_ABS(y1 - y0);
    sy = y0 < y1 ? 1 : -1;
    if (dx == dy)
        err = 0;
    else
        err = (dx > dy ? dx>>1 : -(dy>>1));
    for (;;)
    {
        put_pixel_in_8bpp_layer((unsigned char *)NBG1_BITMAP_ADDR, x0, y0, color_code, 512);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = err;
        if (e2 > -dx)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void	vblank_requirements(void)
{
	//nbg_sprintf(0, 15, "(%x)", (int)BACK_CRAM);
	vdp2_CRAMoffsetA[0] = 0x2;	//Offsets indexed colors for NBG0 by 512 entries ((2<<10) / 4)
	vdp2_CRAMoffsetB[0] = 0x10; //Offsets indexed colors for SPR layer by 256 entries ((1<<10) / 4)
	//vdp2_TVmode[0] = 33027; //Set VDP2 to 704x224 [progressive scan, 704 width] - why? VDP2 will sharpen VDP1's output.
	//Note that sprMode also contains data which specifies the rules for color calculation on sprite-layer pixels.
	//"0x0404" specifies sprite type #4, and sprite color calculation condition #4. Whatever that means; seriously idk.
	//When type 4: bits 14,13 are priority. bits 12,11,10 are color calc ratio.
	vdp2_sprMode[0] = 0x0404; //Sprite Data Type Mode (set to 0xF in hi-res mode, 0x4 in standard res mode)

    *VDP1_EWLR = top_left_erase_pt;
    *VDP1_EWRR = btm_rite_erase_pt;
}

extern void * nbg0_page_adr;
extern void * nbg0_char_adr;

void	init_vdp2(short backColor)
{
	slColRAMMode(2); //Set 24bpp

	slBitMapNbg0(COL_TYPE_256, BM_512x512, (void*)NBG0_BITMAP_ADDR);
	slBitMapNbg1(COL_TYPE_256, BM_512x256, (void*)NBG1_BITMAP_ADDR);
	//Clear VRAM A0
	//Dunno what ends up here, but it clearly isn't important for the game. So just purge it. Maybe leftover from BIOS?
	unsigned char * writeByte = (unsigned char *)VDP2_VRAM_A0;
	for(int y = 0; y < (512 * 256); y++)
	{
		writeByte[y] = 0;
	}
	
	////////////////////////////////////////////////////////////////
	// Section to move default SGL setup of text on NBG0 to NBG2
	////////////////////////////////////////////////////////////////
	//These values are stand-ins for SGL's defaults for NBG0.
	//I am using them to put in place the text whilst being able to control NBG0.
	void * mapPtr = (void*)0x25e76000;
	void * colPtr = (void*)0x000;
	void * celPtr = (void*)0x25e60000;
	slCharNbg2(COL_TYPE_256, CHAR_SIZE_1x1);
	slMapNbg2(mapPtr, mapPtr, mapPtr, mapPtr);
	slPlaneNbg2(PL_SIZE_1x1);
	slPageNbg2(celPtr, colPtr, PNB_1WORD);
	////////////////////////////////////////////////////////////////
	
	
	slScrAutoDisp(NBG0ON | NBG1ON | NBG2ON);
	slPriorityNbg0(1); //Put NBG0 just above BACK
	slPriorityNbg1(6); //
	slPriorityNbg2(7);
	slShadowOn(BACKON);
	
	back_color_setting = backColor;
	slBack1ColSet((void*)back_scrn_colr_addr, back_color_setting);
	
	//Referenced from XL2's SONIC Z-TREME source code
    slSpriteCCalcCond(CC_pr_CN);
    slColorCalcOn(CC_RATE | CC_TOP | SPRON | BACKON);

	//These have to do with the order in which color calculation is done?
    slPrioritySpr0(5);
    slPrioritySpr1(3);
    slPrioritySpr2(3);
    slPrioritySpr3(3);
    slPrioritySpr4(3);
    slPrioritySpr5(3);
    slPrioritySpr6(3);
    slPrioritySpr7(3);
	//These are the color calculation ratios for the SPR layer, according to the CC priority set in the pixel?
    slColRateSpr0(2);
    slColRateSpr1(4);
    slColRateSpr2(6);
    slColRateSpr3(9);
    slColRateSpr4(13);
    slColRateSpr5(18);
    slColRateSpr6(23);
    slColRateSpr7(28);

}
