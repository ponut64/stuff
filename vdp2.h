//
//vdp2.h
//included for dedicated vdp2 program region

#pragma once
//(you'll note the comments are pasted from jo engine)
/** @brief VDP2 VRAM (512 kilobytes) = A0 + A1 + B0 + B1 */
# define VDP2_VRAM               (0x25E00000)

#define NBG0_BITMAP_ADDR		(VDP2_VRAM_A0)
#define NBG1_BITMAP_ADDR		(VDP2_VRAM_B0)

extern unsigned short * back_scrn_colr_addr;
extern unsigned short back_color_setting;

extern unsigned int *	cRAM_24bm;
extern unsigned short *	cRAM_16bm;

void	draw_hud_pixel(short x, short y, unsigned char color_code);
void	draw_hud_line(short x0, short y0, short x1, short y1, unsigned char color_code);
void	init_vdp2(short backColor);

