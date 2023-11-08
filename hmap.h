#pragma once

#define HMAP_SUBDIVISION_LEVEL (120<<16)
#define HMAP_TINY_TEX_LEVEL (400<<16)

#define DITHER_CROSS 		(0x8000)

typedef struct{
	POINT verts[4];
} _pquad;

typedef struct {
	Bool file_done;
	Bool active; //File System Activity
	void * dstAddress;
	Sint8* fid;
	
	Uint16 Xval;
	Uint16 Yval;
	Uint16 totalPix;
} _heightmap;

//# of textures in the map table, before combined textures are made
extern int map_tex_amt; 
//The texture # after the end of the original, unmodified textures
extern int map_end_of_original_textures;
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
extern Bool map_update_complete;
extern Bool * sysBool;
extern Bool map_chg;
extern _heightmap maps[4];
extern Uint8 * main_map;
extern Uint8 * buf_map;
// lightTbl : currently not a normal table, but instead a light table
extern short * lightTbl;
//Notice: Map Tex contains a 10-bit texture number, and two extra bits of flip data.
extern unsigned short * mapTex;
// The texture file names which are to be, or currently are, loaded from CD to use as textures for the heightmap.
extern char map_tex_tbl_names[5][13];
extern char old_tex_tbl_names[5][13];

void	snargon(void);

void	init_heightmap(void);
void	chg_map(_heightmap * tmap);

void	p64MapRequest(short levelNo);

void	read_pgm_header(_heightmap * map);
void	process_map_for_normals(void);

void	make_dithered_textures_for_map(short regenerate);

void	update_hmap(MATRIX msMatrix);
void	hmap_cluster(void);
//from hmap_col.c
void	generate_cell_from_position(POINT pos, _pquad * cell);
void	divide_cell_return_cfnorms(_pquad * quad, POINT cf1, VECTOR norm1, POINT cf2, VECTOR norm2);


