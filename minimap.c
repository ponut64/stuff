
#include <sl_def.h>
#include "def.h"
#include "mymath.h"
#include "vdp2.h"
#include "physobjet.h"

#include "minimap.h"

unsigned char oneLineCt = 51;
unsigned short pixCt = 0;
unsigned int MapPixWidth = 0;
unsigned int MapYpixWidth = 0;
unsigned int mmapXScale = 0;
unsigned int mmapYScale = 0;
unsigned int approx_row = 0;
unsigned int raw_pix_data = 0;
unsigned int exact_map_data = 0;
unsigned int xDrawPos = 0;
unsigned int yDrawPos = 0;
unsigned short colorData = 0;
unsigned short minimap[50 * 51];

#define MMAP_BASE_Y (173)
#define MMAP_BASE_X (12)
#define MMAP_CNTR_Y (MMAP_BASE_Y+25)
#define MMAP_CNTR_X (MMAP_BASE_X+25)
#define MMAP_CTR_PIX (1300)
#define MMAP_WIDTH	(50)
#define	MMAP_HEIGHT	(50)

void	init_minimap(void)
{

}

void	add_position_to_minimap(void)
{

}

void	update_mmap_1pass(void)
{

}

void	draw_minimap(void)
{


}

