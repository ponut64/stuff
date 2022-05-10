#ifndef __HMAP_H__
# define __HMAP_H__

#include <jo/jo.h>
#include "def.h"
#include "pcmstm.h"
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

//# of textures in the map table, before combined textures are made
extern int map_tex_amt; 
//The last texture # of the map's combined textures
extern int map_last_combined_texno;
//The last texture # of the map's dithered non-combined textures
extern int map_last_dithered_texture;
//The last texture # of the map's dithered combined textures
extern int map_last_dithered_combined_texture;
extern int main_map_total_pix; //Total pixels of map
extern int main_map_x_pix; //X pixels of map
extern int main_map_y_pix; //Z pixels of map
extern int main_map_strata[4]; //Strata Heights of texture tables 0-4 
extern int map_texture_table_numbers[5]; //The first texture # of each ground texture table.
extern bool map_update_complete;
extern bool * sysbool;
extern bool map_chg;
extern _heightmap maps[4];
extern Uint8 * main_map;
extern Uint8 * buf_map;
extern char * normTbl;
extern unsigned short * minimap;
//Notice: Map Tex contains a 10-bit texture number, and two extra bits of flip data.
extern unsigned short * mapTex;

void	init_heightmap(void);
void	chg_map(_heightmap * tmap);

void	p64MapRequest(short levelNo);

void	read_pgm_header(_heightmap * map);
void	process_map_for_normals(void);

void	make_dithered_textures_for_map(void);

void	update_hmap(MATRIX msMatrix);
void	hmap_cluster(void);
//from hmap_col.c
void	generate_cell_from_position(POINT pos, _pquad * cell);
void	divide_cell_return_cfnorms(_pquad quad, POINT cf1, VECTOR norm1, POINT cf2, VECTOR norm2);

#endif

