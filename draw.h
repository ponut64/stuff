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
extern _boundBox world_box;
//Root position of the viewport (with no commuted translation)
extern int viewport_pos[3];

extern animationControl t_idle_pose;
extern animationControl t_dead_pose;
extern animationControl t_point_pose;
extern animationControl t_point_anim;
extern animationControl t_look_anim;
extern animationControl t_move_anim;
extern animationControl t_aggro_anim;
extern animationControl t_attack_anim;
extern animationControl t_dead_anim;

extern int scrn_z_fwd[3];

void	set_camera(void);
void	master_draw_stats(void);
void	scene_draw(void);
void	menu_draw(void);
void	sector_vertex_remittance(void);
void	master_draw(void);

extern spriteAnimation qmark;
////////////////////////////////////////////////////////////////////////////////////////////
//stuff for vwmdlfunc.c
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
	backgroundAnimation * idle_state;
	backgroundAnimation * use_state;
	void * buffer;
	int * slot_in_slot_pointer;
} _viewmodelSlot;

typedef struct {
	int fid;
	int inSlot; // (-1 for not in a slot)
	_viewmodelSlot slot_data;
} _viewmodelData;

extern void * viewmodel_0_workram_copy;
extern void * viewmodel_1_workram_copy;

extern backgroundAnimation * viewmodel_state;

extern backgroundAnimation shorty_idle;
extern backgroundAnimation shorty_fire;
extern backgroundAnimation leverpistol_idle;
extern backgroundAnimation leverpistol_fire;

extern _viewmodelData lever_pistol_vm;
extern _viewmodelData shorty_shotgun_vm;
extern _viewmodelSlot viewmodel_slots[2];

void initialize_viewmodel_data(void);
void load_viewmodel_to_slot(_viewmodelData * type, int slot);
void set_viewmodel_from_slot(int slot);
void use_viewmodel(void);



