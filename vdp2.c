
#include <jo/jo.h>

void	init_vdp2(void)
{
	slColRAMMode(2); //Set 24bpp
	slZoomNbg0(32768, 65536);
	slZoomNbg1(32768, 65536);
    slScrAutoDisp(NBG0ON | NBG1ON);
	slPriorityNbg1(7); //Put NBG1 on top.
}
