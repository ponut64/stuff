#pragma once

#define VIEW_OFFSET_X (0 * 65536)
#define VIEW_OFFSET_Y (0 * 65536)
#define VIEW_OFFSET_Z (0 * 65536)
#define DRAW_MASTER (1)
#define DRAW_SLAVE	(2)

extern POINT point_0[LCL_MAP_PIX * LCL_MAP_PIX];
extern _quad polygon_0[LCL_MAP_PLY * LCL_MAP_PLY];
extern POINT normal_0[1];
extern unsigned char major_axis_0[4];
extern gvAtr attribute_0[1];
extern GVPLY polymap[];
extern FIXED hmap_matrix_pos[XYZ];
extern FIXED hmap_actual_pos[XYZ];
//Player Model
extern entity_t pl_model;
//Player's Shadow
extern entity_t shadow;
//Player Wings
extern entity_t wings;
//Texture Tables
extern entity_t txtbl_e[5];
//Root screen matrix (for view model)
extern MATRIX perspective_root;
//Root world matrix (for world models)
extern MATRIX world_root;

extern int scrn_z_fwd[3];

void	display_ztp(entity_t * model);
void	set_camera(void);
void	master_draw_stats(void);
void	scene_draw(void);
void	menu_draw(void);
void	sector_vertex_remittance(void);
void	master_draw(void);


extern animationControl reload;

extern spriteAnimation qmark;



