#ifndef __HMAP_H__
# define __HMAP_H__

#include <jo/jo.h>
#include "def.h"
#include "draw.h"
#include "vdp2.h"
#include "minimap.h"
#include "bounder.h"
#include "mymath.h"
#include "physobjet.h"
#include "DSP/DSP.h"

typedef struct{
	POINT verts[4];
} _pquad;

typedef struct {
	bool file_done;
	bool active; //File System Activity
	void * dstAddress;
	Sint8* fid;
	
	Uint16 Xval;
	Uint16 Zval;
	Uint16 totalPix;
} _heightmap;

extern int main_map_total_pix; //Total pixels of map
extern int main_map_x_pix; //X pixels of map
extern int main_map_z_pix; //Z pixels of map
extern int main_map_strata[4]; //Strata Heights of texture tables 0-4 // Eventually, to be loaded from file. But can be manually defined too, I guess.
extern bool map_update_complete;
_heightmap maps[4];
extern Uint8 * main_map;
extern Uint8 * buf_map;
extern unsigned char * local_hmap;
extern unsigned short * minimap;

void * hmap_buf;
//Used in msfs.c
void	read_gmp_header(_heightmap * map);
//
void	init_heightmap(void);
void	update_hmap(void);
void	chg_map(_heightmap tmap);
void	hmap_cluster(void);
//from hmap_col.c
void	generate_cell_from_position(POINT pos, _pquad * cell);
void	divide_cell_return_cfnorms(_pquad quad, POINT cf1, VECTOR norm1, POINT cf2, VECTOR norm2);

#endif
