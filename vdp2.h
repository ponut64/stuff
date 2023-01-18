//
//vdp2.h
//included for dedicated vdp2 program region

#ifndef __VDP2_H__
# define __VDP2_H__


extern unsigned short * back_scrn_colr_addr;
extern unsigned short back_color_setting;

void init_vdp2(short backColor);

#endif

