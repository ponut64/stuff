#ifndef __RENDER_H__
# define __RENDER_H__

#include "mloader.h"
#include "bounder.h"

//Comment out to enable/disable high res modes
//#define  USE_HI_RES (1)

#ifdef USE_HI_RES
	#define TV_WIDTH (704)
	#define TV_HALF_WIDTH (352)
	#define TV_HEIGHT (448)
	#define TV_HALF_HEIGHT (224)
	
	#define NEAR_PLANE_DISTANCE (8<<16) //The minimum Z allowed
#else
	#define TV_WIDTH (352)
	#define TV_HALF_WIDTH (176)
	#define TV_HEIGHT (224)
	#define TV_HALF_HEIGHT (112)
	
	#define NEAR_PLANE_DISTANCE (15<<16) //The minimum Z allowed
#endif

#define VRAM_TEXTURE_BASE (0x10000) //Matches jo engine specification
#define VDP1_VRAM (0x25C00000)
#define MAP_TO_VRAM(sh2map_vram_addr) ((sh2map_vram_addr - VDP1_VRAM)>>3) 
#define INTERNAL_MAX_POLY 2600 //Slave only 1700
#define INTERNAL_MAX_VERTS 2800 //Slave only 2800
#define MAX_SSH2_SENT_POLYS (700) //SpriteBuf size limitation // thanks VBT for fixing sglarea.o for me
#define MAX_MSH2_SENT_POLYS (600) //SpriteBuf size limitation 
#define MAX_SSH2_ENTITY_VERTICES (500)
#define MAX_MSH2_ENTITY_VERTICES (300)
#define	MAX_SIMULTANEOUS_ANIMATED_ENTITIES (5) //RAM-wise, can be pretty high. CPU-wise, probably not.
// Base PMOD: Bit 12 is HSS
#define VDP1_BASE_PMODE (0x1490)
// CMDCTRL = Select Distorted Sprite
#define VDP1_BASE_CMDCTRL (2)
#define VDP1_PRECLIPPING_DISABLE (2048)
//VDP1 perf limit depends on how many pixels it's drawing.
//User Clipping Settings
#define SYS_CLIPPING (0)
#define USER_CLIP_INSIDE (1)
#define USER_CLIP_OUTSIDE (2)
//Screen clip flags
#define SCRN_CLIP_X		(1)
#define SCRN_CLIP_NX	(1<<1)
#define SCRN_CLIP_Y		(1<<2)
#define SCRN_CLIP_NY	(1<<3)
#define SCRN_CLIP_FLAGS	(0xF)
#define CLIP_Z 			(1<<4)
#define LOW_Z 			(1<<5)

#define CLIP_TO_SCRN_X(xcoord) ((xcoord > TV_HALF_WIDTH) ? TV_HALF_WIDTH : (xcoord < -TV_HALF_WIDTH) ? -TV_HALF_WIDTH : xcoord)
#define CLIP_TO_SCRN_Y(ycoord) ((ycoord > TV_HALF_HEIGHT) ? TV_HALF_HEIGHT : (ycoord < -TV_HALF_HEIGHT) ? -TV_HALF_HEIGHT : ycoord)

#define FAR_PLANE_DISTANCE	(1000<<16) //The maximum Z allowed

#define	MAX_DYNAMIC_LIGHTS (2)

#define MAX_SPRITES (64)

#define SCR_SCALE_X (16)
#define SCR_SCALE_Y (16)

/*
Render data flags:
	
	Byte 1 of render_data_flags:
	|	0		|	1		|	2		|	3		|	4	-	5 	|	6	-	7 |		
	  Dual-plane	Mesh		Physical	MSB On		Tex. flip	  Sorting rule
	Byte 2 of render_data_flags:
	|	0		|	1		|	2		|	3		|	4	-	5 	|	6	-	7 |		
							 Subdivision
*/
	#define GV_FLAG_SINGLE	(0x1) // Zero, dual-plane. One, single-plane.
	#define GV_FLAG_MESH	(0x2) // Zero, no mesh. One, mesh.
	#define GV_FLAG_PHYS	(0x4) // Zero, physical plane (in supported objects). One, no collision with plane.
	#define GV_FLAG_DARK	(0x8) // Zero, normal light. One, MSB is enabled, making the polygon dark.
	#define GV_SORT_MAX		(0x40)
	#define GV_SORT_CEN		(0x80)
	#define GV_SORT_MIN		(0xC0)
	#define GV_FLIP_V		(0x20)
	#define GV_FLIP_H		(0x10)
	#define GV_FLIP_HV		(0x30)
	#define GET_SORT_DATA(n)	(n & 0xC0)
	#define GET_FLIP_DATA(n)	(n & 0x30)

//////////////////////////////////
// Engine's working struct for drawing raw sprites
///////////// /////////////////////
typedef struct {
	int lifetime;		// Time (in fixed-point seconds) to allow the sprite to persist.
	POINT pos; 			//World-space position for billboard scaled sprites, screenspace top-left coordinate otherwise
	short span[XYZ]; 		//Screenspace X/Y span, if a billboard. 3D XYZ size of lines.
	short texno;		//Texture table number to use OR color code (depends on draw type)
	short useClip;		//To clip by system, in user, or outside of user.
	unsigned char mesh;	//Boolean. 1 enables mesh effect drawing.
	char type; 			//"B" for billboard, "S" for normal sprite.
} _sprite; //22 bytes each

//////////////////////////////////
// Post-transformed vertice data struct
//////////////////////////////////
typedef struct {
    POINT  pnt;
	short clipFlag;
} vertex_t; //14 bytes each

//////////////////////////////////
// Point light data struct
//////////////////////////////////
typedef struct {
	FIXED * ambient_light;
	int	pos[3];
	int min_bright; // Flat amount of brightness to add (or subtract). Make note that it is shifted left once in post.
	short bright; //(signed) brightness. You can unbright with a light source if this is negative.
	unsigned short pop;
} point_light;

//////////////////////////////////
// Palette Coded (Indexed Color) texture definition
//////////////////////////////////
typedef struct{
	unsigned short SIZE; //VDP1 Size Word
	unsigned short SRCA; //VDP1 Source Address Word (MAP_TO_VRAM)
} paletteCode;


//////////////////////////////////
// Animation Control Struct
//////////////////////////////////
typedef struct
{
	char reset_enable;
	Uint8 arate[64];
    Uint16 currentFrm;
    Uint8 currentKeyFrm;
    Uint8 startFrm;
    Uint8 endFrm;
} animationControl;

//////////////////////////////////
// Basic Menu Stuff
//////////////////////////////////
typedef struct
{
	int topLeft[XY];
	short scale[XY];
	short option_grid[XY];
	short num_option;
	short backColor;
	short optionColor;
	short selection;
	char ** option_text;	
	int * btmRight; //Note: Only writes to this, does not read from it.
} __basic_menu;


extern entity_t * drawn_entity_list[64];
extern short drawn_entity_count;
extern MATRIX global_view_matrix;

extern int * DVSR;
extern int * DVDNTH;
extern int * DVDNTL;
extern int scrn_dist;
extern short vert_clip_x;
extern short vert_clip_nx;
extern short vert_clip_y;
extern short vert_clip_ny;
extern unsigned short top_left_erase_pt;
extern unsigned short btm_rite_erase_pt;
extern int send_draw_stats;
extern int hi_res_switch;
extern vertex_t ssh2VertArea[500];
extern vertex_t msh2VertArea[300];
extern _sprite	sprWorkList[MAX_SPRITES];
extern int * ssh2SentPolys;
extern int * msh2SentPolys;
extern int * transVerts;
extern int * transPolys;
extern paletteCode * pcoTexDefs;

extern point_light * active_lights;

extern int baseAsciiTexno;
extern int sprAsciiHeight;
extern int sprAsciiWidth;

void	add_to_sprite_list(FIXED * position, short * span, short texno, unsigned char mesh, char type, short useClip, int lifetime);
void	transform_mesh_point(FIXED * mpt, FIXED * opt, _boundBox * mpara);
void	draw2dSquare(int * firstPt, int * scndPt, unsigned short colorData, unsigned short solid_or_border);
void	ssh2BillboardScaledSprite(_sprite * spr);
void	ssh2Line(_sprite * spr);
void	drawAxis(POINT size);
void	spr_print(int xPos, int yPos, char * data);
void	spr_sprintf(int xPos, int yPos, ...);
void	nbg_sprintf(void * scrnAddr /*to be returned from slLocate or other such function*/,  ...);
short	menu_with_options(__basic_menu * mnu);

FIXED	trans_pt_by_component(POINT ptx, FIXED * normal);
void	SetFixDiv(FIXED dividend, FIXED divisor); //Defined as "dividend / divisor", for fixed points, using division unit
void	ssh2SetCommand(FIXED * p1, FIXED * p2, FIXED * p3, FIXED * p4, Uint16 cmdctrl, Uint16 cmdpmod, Uint16 cmdsrca, Uint16 cmdcolr, Uint16 cmdsize, Uint16 cmdgrda, FIXED drawPrty);
void	msh2SetCommand(FIXED * p1, FIXED * p2, FIXED * p3, FIXED * p4, Uint16 cmdctrl, Uint16 cmdpmod, Uint16 cmdsrca, Uint16 cmdcolr, Uint16 cmdsize, Uint16 cmdgrda, FIXED drawPrty);
void	init_render_area(short desired_horizontal_fov);
void	vblank_requirements(void);
void	frame_render_prep(void);
void	setFramebufferEraseRegion(int xtl, int ytl, int xbr, int ybr);
void	determine_colorbank(unsigned short * colorBank, int * luma);
void	preclipping(vertex_t ** ptv, unsigned short * flip, unsigned short * pclp);
void	clipping(vertex_t * pnt, short useClip);
void	setUserClippingAtDepth(int * topLeft, int * btmRight, int zDepthTgt);
void	ssh2DrawModel(entity_t * ent);
void	msh2DrawModel(entity_t * ent, MATRIX msMatrix, FIXED * lightSrc);
void	ssh2DrawAnimation(animationControl * animCtrl, entity_t * ent, Bool transplant);
void	sort_master_polys(void);
#endif

