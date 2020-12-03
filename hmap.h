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
//
#include "dspm.h"
//

typedef struct{
	POINT verts[4];
} _pquad;

typedef struct {
	bool file_done;
	bool active; //File System Activity
	void * dstAddress;
	Sint8* fid;
	
	Uint16 Xval;
	Uint16 Yval;
	Uint16 totalPix;
} _heightmap;

extern int main_map_total_pix; //Total pixels of map
extern int main_map_x_pix; //X pixels of map
extern int main_map_y_pix; //Z pixels of map
extern int main_map_strata[4]; //Strata Heights of texture tables 0-4 // Eventually, to be loaded from file. But can be manually defined too, I guess.
extern bool map_update_complete;
extern bool * sysbool;
extern bool map_chg;
extern _heightmap maps[4];
extern Uint8 * main_map;
extern Uint8 * buf_map;
extern char * normTbl;
extern unsigned short * minimap;

//Used in msfs.c
void	read_pgm_header(_heightmap * map);
void	process_map_for_normals(void);
//
void	init_heightmap(void);
void	update_hmap(MATRIX msMatrix);
void	chg_map(_heightmap * tmap);
void	hmap_cluster(void);
//from hmap_col.c
void	generate_cell_from_position(POINT pos, _pquad * cell);
void	divide_cell_return_cfnorms(_pquad quad, POINT cf1, VECTOR norm1, POINT cf2, VECTOR norm2);

#endif

