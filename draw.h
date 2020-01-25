
#ifndef __DRAW_H__
# define __DRAW_H__

#define VIEW_OFFSET_X (0)
#define VIEW_OFFSET_Y (8)
#define VIEW_OFFSET_Z (-22)

//ZT Folder from XL2 (some slight modifications)
#include "ZT/ZT_COMMON.h"
//
#include "def.h"
#include "bounder.h"
#include "collision.h"
#include "mymath.h"
#include "physobjet.h"
#include "player_phy.h"
#include "hmap.h"
#include "minimap.h"
#include "control.h"
#include "render.h"
//
#include "dspm.h"
//
extern POINT point_0[LCL_MAP_PIX * LCL_MAP_PIX];
extern POLYGON polygon_0[LCL_MAP_PLY * LCL_MAP_PLY];
extern ATTR attribute_0[LCL_MAP_PLY * LCL_MAP_PLY];
extern PDATA polymap[];
extern FIXED hmap_matrix_pos[XYZ];
//Player Model
extern entity_t pl_model;
//Player's Shadow
extern entity_t shadow;
//Texture Tables
extern entity_t txtbl_e[5];

void	display_ztp(entity_t * model);
void	set_camera(void);
void	master_draw_stats(void);
void	object_draw(void);
void	prep_map_mtx(void);
void	master_draw(void);
void	initCamera(void);

#endif


