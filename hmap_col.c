//hmap_col.c
//This file is compiled separately.
//Note: Function/data definitions included in hmap.h

#include <sl_def.h>
#include "def.h"
#include "mymath.h"
#include "mloader.h"
#include "collision.h"
#include "physobjet.h"
//
#include "dspm.h"
//


#include "hmap.h"

#define FIXED_ONE_THIRD (21845)

void	generate_cell_from_position(POINT pos, _pquad * cell)
{
	
	static int cellPos[XY] = {0, 0};
	static int cellX0off = 0;
	static int cellX1off = 0;
	static int cellX2off = 0;
	static int cellX3off = 0;
	
	static int cellY0off = 0;
	static int cellY1off = 0;
	static int cellY2off = 0;
	static int cellY3off = 0;
	
cellPos[X] = (pos[X] > 0) ? ((fxm((INV_CELL_SIZE), pos[X])>>16)) + 1 : ((fxm((INV_CELL_SIZE), pos[X])>>16)); // - 1?
cellPos[Y] = (pos[Z] > 0) ? ((fxm((INV_CELL_SIZE), pos[Z])>>16)) + 1 : ((fxm((INV_CELL_SIZE), pos[Z])>>16)); // ? +1 : - 1
/*
If we are at X1 and Z1, we are on the first polygon in the X+ and Z+ directions.
Importantly, no polygon is "zero". Thus, the map size must be an odd number or else this does not work.
*/
if(pos[X] >= 0){
	cell->verts[0][X] = -((cellPos[X]-1)*CELL_SIZE_INT)<<16;
	cell->verts[1][X] = -((cellPos[X])*CELL_SIZE_INT)<<16;
	cell->verts[2][X] = -((cellPos[X])*CELL_SIZE_INT)<<16;
	cell->verts[3][X] = -((cellPos[X]-1)*CELL_SIZE_INT)<<16;
	cellX0off = (cellPos[X]-1);
	cellX1off = (cellPos[X]);
	cellX2off = (cellPos[X]);
	cellX3off = (cellPos[X]-1);
} else {
	cell->verts[0][X] = -((cellPos[X])*CELL_SIZE_INT)<<16;
	cell->verts[1][X] = -((cellPos[X]+1)*CELL_SIZE_INT)<<16;
	cell->verts[2][X] = -((cellPos[X]+1)*CELL_SIZE_INT)<<16;
	cell->verts[3][X] = -((cellPos[X])*CELL_SIZE_INT)<<16;
	cellX0off = (cellPos[X]);
	cellX1off = (cellPos[X]+1);
	cellX2off = (cellPos[X]+1);
	cellX3off = (cellPos[X]);
}
if(pos[Z] >= 0){
	cell->verts[0][Z] = -((cellPos[Y])*CELL_SIZE_INT)<<16;
	cell->verts[1][Z] = -((cellPos[Y])*CELL_SIZE_INT)<<16;
	cell->verts[2][Z] = -((cellPos[Y]-1)*CELL_SIZE_INT)<<16;
	cell->verts[3][Z] = -((cellPos[Y]-1)*CELL_SIZE_INT)<<16;
	cellY0off =  -((cellPos[Y]-1)*main_map_x_pix);
	cellY1off =  -((cellPos[Y]-1)*main_map_x_pix);
	cellY2off =  -(cellPos[Y]*main_map_x_pix);
	cellY3off =  -(cellPos[Y]*main_map_x_pix);
} else {
	cell->verts[0][Z] = -((cellPos[Y]+1)*CELL_SIZE_INT)<<16;
	cell->verts[1][Z] = -((cellPos[Y]+1)*CELL_SIZE_INT)<<16;
	cell->verts[2][Z] = -((cellPos[Y])*CELL_SIZE_INT)<<16;
	cell->verts[3][Z] = -((cellPos[Y])*CELL_SIZE_INT)<<16;
	cellY0off =  -((cellPos[Y])*main_map_x_pix);
	cellY1off =  -((cellPos[Y])*main_map_x_pix);
	cellY2off =  -((cellPos[Y]+1)*main_map_x_pix);
	cellY3off =  -((cellPos[Y]+1)*main_map_x_pix);
}

	int vert0pix = cellX0off + cellY0off + (main_map_total_pix>>1); //offsets are calculated from map center.
	int vert1pix = cellX1off + cellY1off + (main_map_total_pix>>1); //Thus the pixel that is exactly in the middle of the map is used.
	int vert2pix = cellX2off + cellY2off + (main_map_total_pix>>1); //This pixel # is found simply by dividing the total pixels in half by a bit shift.
	int vert3pix = cellX3off + cellY3off + (main_map_total_pix>>1);
//------------------------------------------------------------------------------------------------	
	// nbg_sprintf(0, 10, "(%i)", vert0pix);
	// nbg_sprintf(0, 11, "(%i)", vert1pix);
	// nbg_sprintf(0, 12, "(%i)", vert2pix);
	// nbg_sprintf(0, 13, "(%i)", vert3pix);
//------------------------------------------------------------------------------------------------		
	//Note: The order of application to vertices is intentionally reversed. [Vert 3 uses Vert 0's offset]
	//Remember: We will NEVER sample a negative pixel. Hitherto, our sampling numbers are unsigned.
	cell->verts[3][Y] = (vert0pix < main_map_total_pix && (JO_ABS(cellX0off)-1 < (main_map_x_pix>>1))) ? -main_map[vert0pix]<<(MAP_V_SCALE) : -(127<<(MAP_V_SCALE));
	cell->verts[2][Y] = (vert1pix < main_map_total_pix && (JO_ABS(cellX1off)-1 < (main_map_x_pix>>1))) ? -main_map[vert1pix]<<(MAP_V_SCALE) : -(127<<(MAP_V_SCALE));	
	cell->verts[1][Y] = (vert2pix < main_map_total_pix && (JO_ABS(cellX2off)-1 < (main_map_x_pix>>1))) ? -main_map[vert2pix]<<(MAP_V_SCALE) : -(127<<(MAP_V_SCALE));	
	cell->verts[0][Y] = (vert3pix < main_map_total_pix && (JO_ABS(cellX3off)-1 < (main_map_x_pix>>1))) ? -main_map[vert3pix]<<(MAP_V_SCALE) : -(127<<(MAP_V_SCALE));	
}

void	divide_cell_return_cfnorms(_pquad * quad, POINT cf1, VECTOR norm1, POINT cf2, VECTOR norm2)
{

	/*
	Trangles
	0 - 1	0
		2	3 - 2
	from
	0 - 1
	3 - 2
	*/
//------------------------------------------------------------------------------------------------------
static POINT tri1p1 = {0, 0, 0};
static POINT tri1p2 = {0, 0, 0};
static POINT tri1p3 = {0, 0, 0};
static VECTOR tri1V1 = {0, 0, 0};
static VECTOR tri1V2 = {0, 0, 0};
static VECTOR crosstri1 = {0, 0, 0};
static VECTOR normaltri1 = {0, 0, 0};
static POINT tri1CF = {0, 0, 0};

tri1p1[X] = (quad->verts[0][X]);
tri1p1[Y] = (quad->verts[0][Y]);
tri1p1[Z] = (quad->verts[0][Z]);
				 
tri1p2[X] = (quad->verts[1][X]);
tri1p2[Y] = (quad->verts[1][Y]);
tri1p2[Z] = (quad->verts[1][Z]);
				 
tri1p3[X] = (quad->verts[2][X]);
tri1p3[Y] = (quad->verts[2][Y]);
tri1p3[Z] = (quad->verts[2][Z]);

tri1CF[X] = fxm(FIXED_ONE_THIRD,tri1p1[X] + tri1p2[X] + tri1p3[X]);
tri1CF[Y] = fxm(FIXED_ONE_THIRD,tri1p1[Y] + tri1p2[Y] + tri1p3[Y]);
tri1CF[Z] = fxm(FIXED_ONE_THIRD,tri1p1[Z] + tri1p2[Z] + tri1p3[Z]);

tri1V1[X] = -(CELL_SIZE);
tri1V1[Y] = (tri1p3[Y]) - (tri1p1[Y]);
tri1V1[Z] = CELL_SIZE;

tri1V2[X] = -(CELL_SIZE);
tri1V2[Y] = (tri1p2[Y]) - (tri1p1[Y]);
tri1V2[Z] = 0;

//Cross 2 - 1. IT IS OPPOSITE IN THE OTHER TRIANGLE.
fxcross(tri1V2, tri1V1, crosstri1);
crosstri1[X] = crosstri1[X]>>8;
crosstri1[Y] = crosstri1[Y]>>8;
crosstri1[Z] = crosstri1[Z]>>8;
normalize(crosstri1, normaltri1);

//Invert its position 
//We also set the point back by its normal as the planar collision detection works with the normal relative to a position, but the plane is on the normal.
cf1[X] = -tri1CF[X] + normaltri1[X];
cf1[Y] = -tri1CF[Y] + normaltri1[Y];
cf1[Z] = -tri1CF[Z] + normaltri1[Z];
norm1[X] = normaltri1[X];
norm1[Y] = normaltri1[Y];
norm1[Z] = normaltri1[Z];

//------------------------------------------------------------------------------------------------------

static POINT tri2p1 = {0, 0, 0};
static POINT tri2p2 = {0, 0, 0};
static POINT tri2p3 = {0, 0, 0};
static VECTOR tri2V1 = {0, 0, 0};
static VECTOR tri2V2 = {0, 0, 0};
static VECTOR crosstri2 = {0, 0, 0};
static VECTOR normaltri2 = {0, 0, 0};
static POINT tri2CF = {0, 0, 0};


tri2p1[X] = (quad->verts[0][X]);
tri2p1[Y] = (quad->verts[0][Y]);
tri2p1[Z] = (quad->verts[0][Z]);
				 
tri2p2[X] = (quad->verts[3][X]);
tri2p2[Y] = (quad->verts[3][Y]);
tri2p2[Z] = (quad->verts[3][Z]);
				 
tri2p3[X] = (quad->verts[2][X]);
tri2p3[Y] = (quad->verts[2][Y]);
tri2p3[Z] = (quad->verts[2][Z]);

tri2CF[X] = fxm(FIXED_ONE_THIRD,tri2p1[X] + tri2p2[X] + tri2p3[X]);
tri2CF[Y] = fxm(FIXED_ONE_THIRD,tri2p1[Y] + tri2p2[Y] + tri2p3[Y]);
tri2CF[Z] = fxm(FIXED_ONE_THIRD,tri2p1[Z] + tri2p2[Z] + tri2p3[Z]);

tri2V1[X] = -(CELL_SIZE);
tri2V1[Y] = (tri2p3[Y]) - (tri2p1[Y]);
tri2V1[Z] = CELL_SIZE;

tri2V2[X] = 0;
tri2V2[Y] = (tri2p2[Y]) - (tri2p1[Y]);
tri2V2[Z] = CELL_SIZE;


//Cross 1 - 2. IT IS OPPOSITE IN THE OTHER TRIANGLE.
fxcross(tri2V1, tri2V2, crosstri2);
crosstri2[X] = crosstri2[X]>>8;
crosstri2[Y] = crosstri2[Y]>>8;
crosstri2[Z] = crosstri2[Z]>>8;
normalize(crosstri2, normaltri2);

//Invert its position
//We also set the point back by its normal as the planar collision detection works with the normal relative to a position, but the plane is on the normal.
cf2[X] = -tri2CF[X] + normaltri2[X];
cf2[Y] = -tri2CF[Y] + normaltri2[Y];
cf2[Z] = -tri2CF[Z] + normaltri2[Z];
norm2[X] = normaltri2[X];
norm2[Y] = normaltri2[Y];
norm2[Z] = normaltri2[Z];

//------------------------------------------------------------------------------------------------------
	
	// slPrintFX(quad->verts[0][X], slLocate(0, 11));
	// slPrintFX(quad->verts[0][Y], slLocate(0, 12));
	// slPrintFX(quad->verts[0][Z], slLocate(0, 13));
					 
	// slPrintFX(quad->verts[1][X], slLocate(12, 11));
	// slPrintFX(quad->verts[1][Y], slLocate(12, 12));
	// slPrintFX(quad->verts[1][Z], slLocate(12, 13));
					 
	// slPrintFX(quad->verts[3][X], slLocate(0, 15));
	// slPrintFX(quad->verts[3][Y], slLocate(0, 16));
	// slPrintFX(quad->verts[3][Z], slLocate(0, 17));
					 
	// slPrintFX(quad->verts[2][X], slLocate(12, 15));
	// slPrintFX(quad->verts[2][Y], slLocate(12, 16));
	// slPrintFX(quad->verts[2][Z], slLocate(12, 17));

}

