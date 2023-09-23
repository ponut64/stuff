
#ifndef __DRAW_H__
#define __DRAW_H__

#define VIEW_OFFSET_X (0 * 65536)
#define VIEW_OFFSET_Y (8 * 65536)
#define VIEW_OFFSET_Z (-22 * 65536)

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
//Root perspective matrix, including translation and rotation of third-person camera (or first person, if not?)
extern MATRIX perspective_root;

void	display_ztp(entity_t * model);
void	set_camera(void);
void	master_draw_stats(void);
void	object_draw(void);
void	prep_map_mtx(void);
void	map_draw_prep(void);
void	master_draw(void);


extern animationControl idle;
extern animationControl idleB;
extern animationControl stop;
extern animationControl fall;
extern animationControl slideIdle;
extern animationControl slideLln;
extern animationControl slideRln;
extern animationControl airIdle;
extern animationControl airLeft;
extern animationControl airRight;
extern animationControl jump;
extern animationControl hop;
extern animationControl walk;
extern animationControl run;
extern animationControl dbound;
extern animationControl climbIdle;
extern animationControl climbing;

extern animationControl flap;

extern spriteAnimation qmark;
extern spriteAnimation arrow;
extern spriteAnimation check;
extern spriteAnimation goal;

#endif


