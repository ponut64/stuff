#pragma once

#include "mloader.h"


//Comment out to enable/disable high res modes
//#define  USE_HI_RES (1)

#ifdef USE_HI_RES
	#define TV_WIDTH (704)
	#define TV_HALF_WIDTH (352)
	#define TV_HEIGHT (448)
	#define TV_HALF_HEIGHT (224)
	
	#define NEAR_PLANE_DISTANCE (8<<16) //The minimum Z allowed
	#define SUPER_NEAR_PLANE	(NEAR_PLANE_DISTANCE>>2) //Eeh... just trying
#else
	#define TV_WIDTH (352)
	#define TV_HALF_WIDTH (176)
	#define TV_HEIGHT (224)
	#define TV_HALF_HEIGHT (112)
	
	#define NEAR_PLANE_DISTANCE (15<<16) //The minimum Z allowed
	#define SUPER_NEAR_PLANE	(NEAR_PLANE_DISTANCE>>2) //Eeh... just trying
#endif

#define VRAM_TEXTURE_BASE (0xA000) //Matches jo engine specification
#define VDP1_VRAM (0x25C00000)
#define MAP_TO_VRAM(sh2map_vram_addr) ((sh2map_vram_addr - VDP1_VRAM)>>3) 
#define INTERNAL_MAX_POLY (2048)
#define INTERNAL_MAX_VERTS (2048)
#define MAX_SSH2_SENT_POLYS (750) //SpriteBuf size limitation 
#define MAX_MSH2_SENT_POLYS (550) //SpriteBuf size limitation 
#define MAX_SSH2_ENTITY_VERTICES (1024) //These are probably oversized, but they need to be at least this big for the subdivision.
#define MAX_MSH2_ENTITY_VERTICES (1024)
#define	MAX_SIMULTANEOUS_ANIMATED_ENTITIES (5) //RAM-wise, can be pretty high. CPU-wise, probably not.
#define MAX_SIMULTANEOUS_SPRITE_ANIMATIONS (64)
// Base PMOD: Bit 12 is HSS
#define VDP1_BASE_PMODE (0x1490)
// CMDCTRL = Select Distorted Sprite
#define VDP1_BASE_CMDCTRL			(0x2)
#define VDP1_POLYLINE_CMDCTRL		(0x5)
#define VDP1_PRECLIPPING_DISABLE 	(2048)
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
////////////////////////////////////
// Warning:
//	These clip flags that the DSP uses will inherently be shifted left by four (more accurately, multiplied by 16).
//	So the reason that the applied values as read here overlap with CLIP_Z and DSP_CLIP_CHECK is because these values are effectively <<4.
#define DSP_PORT_01		((1)<<4)
#define DSP_PORT_12		((1<<1)<<4)
#define DSP_PORT_23		((1<<2)<<4)
#define DSP_PORT_30		((1<<3)<<4)
//
#define CLIP_Z 			(1<<4)
#define DSP_CLIP_IN		(0x0)
#define DSP_CLIP_CHECK	(1<<5) //When the DSP performs portal processing, ALL processed vertices will receive this flag.
#define JUST_CLIP_FLAGS	(0xFFFFFF1F)
#define JUST_PORT_FLAGS	(0xFFFFFF00)

#define DSP_PORT6		(0xF0000000)
#define DSP_PORT5		(0xF000000)
#define DSP_PORT4		(0xF00000)
#define DSP_PORT3		(0xF0000)
#define DSP_PORT2		(0xF000)
#define DSP_PORT1		(0xF00)

#define CLIP_TO_SCRN_X(xcoord) ((xcoord > TV_HALF_WIDTH) ? TV_HALF_WIDTH : (xcoord < -TV_HALF_WIDTH) ? -TV_HALF_WIDTH : xcoord)
#define CLIP_TO_SCRN_Y(ycoord) ((ycoord > TV_HALF_HEIGHT) ? TV_HALF_HEIGHT : (ycoord < -TV_HALF_HEIGHT) ? -TV_HALF_HEIGHT : ycoord)

#define FAR_PLANE_DISTANCE	(2048<<16) //The maximum Z allowed. Check with setCommand functions to verify it will fit in 1kb zbuffer.

#define	MAX_DYNAMIC_LIGHTS (2)

#define MAX_SPRITES (64)

#define MAX_SCENE_PORTALS	(12)
#define MAX_USED_PORTALS	(6)

#define SUBDIVISION_SCALE (64)
#define SUBDIVISION_NEAR_PLANE (15<<16)

#define SCR_SCALE_X (16)
#define SCR_SCALE_Y (16)

#define DEPTH_CUE_OFFSET (200<<16)
#define DEPTH_CUE_CUTOFF (1200<<16)

/*
Render data flags:
	
	Byte 1 of render_data_flags:
	|	0		|	1		|	2		|	3		|	4	-	5 	|		6	-	7 	 	|		
	  Dual-plane	Mesh	 Polyline		MSB On		Tex. flip	  		Sorting rule
	Byte 2 of render_data_flags:
	|	8		|	9		|	10		|	11		|	12	 -	13	|	14		|	15		|		
	Subdivision	  Collision		Ladder	Climbable		Portal Specs	Anim 		On/off
*/
	#define GV_FLAG_SINGLE		(0x1) // Zero, dual-plane. One, single-plane.
	#define GV_FLAG_MESH		(0x2) // Zero, no mesh. One, mesh.
	#define GV_FLAG_POLYLINE	(0x4) // Zero, normal polygon. One, draws with polyline. Could be somewhere else.
	#define GV_FLAG_DARK		(0x8) // Zero, normal light. One, MSB is enabled, making the polygon dark.
	#define GV_FLAG_NDIV		(0x100) // Zero, polygon can subdivide (in supported objects). One, no subdivision.
	#define GV_FLAG_PHYS		(0x200) // One, physical plane (in supported objects). Zero, no collision with plane.
	#define GV_FLAG_LADDER		(0x400) // Boolean. 1 = ladder. 0 = no ladder. Notice: all ladders are climbable.
	#define GV_FLAG_CLIMBABLE	(0x800) //Boolean. 1 = Climbable. 0 = not climbable.
	#define GV_FLAG_PORTAL		(0x1000) //Boolean. 0, not a portal. 1, is a portal.
	#define GV_FLAG_PORT_TYPE	(0x2000) //Boolean. 0, portal OUT processing. 1, portal IN processing.
	#define GV_FLAG_ANIM		(0x4000) //Boolean. 0, texture will not animate. 1, texture is potentially animated.
	#define GV_FLAG_DISPLAY		(0x8000) //Boolean. 1, polygon is treated as polygon. 0, polygon is not rendered. 
	#define GV_SORT_MAX			(0x40)
	#define GV_SORT_CEN			(0x80)
	#define GV_SORT_MIN			(0xC0)
	#define GV_SCTR_BACKFACE	(0xC0) //Unique backfaced flag for sectors, since the drawing system overrides Z sorting rules there.
	#define GV_SCTR_FRNTFACE	(0xFF3F)
	#define GV_FLIP_V			(0x20)
	#define GV_FLIP_H			(0x10)
	#define GV_FLIP_HV			(0x30)
	#define GET_SORT_DATA(n)	(n & 0xC0)
	#define GET_FLIP_DATA(n)	(n & 0x30)
	#define GV_SCTR_MOVER		(0xF0) //Special flag only scanned for on sector data. Marks the surface as belonging to a MOVER.
	
	#define PORTAL_TYPE_ACTIVE	(1)		//Flag applied to active portals (1 = active)
	#define PORTAL_INTERSECTING	(1<<1)	//Flag applied when portal is intersecting view plane
	#define PORTAL_OFFSCREEN	(1<<2)	//Flag applied when portal is off-screen (mutually exclusive with intersecting)

typedef struct {
	union {
		unsigned char raw;
		struct {
			unsigned char drawMode:4;
			unsigned char mesh:1;
			unsigned char drawOnce:1;
			unsigned char sorted:1;
			unsigned char alive:1;
		} info;
	};
} _spr_type_data;
//////////////////////////////////
// Engine's working struct for drawing raw sprites
///////////// /////////////////////
typedef struct {
	int lifetime;		// Time (in fixed-point seconds) to allow the sprite to persist.
	POINT pos; 			//World-space position for billboard scaled sprites, screenspace top-left coordinate otherwise
	short span[XYZ];	//Screenspace X/Y span, if a billboard. 3D XYZ size of lines. Other uses apply.
	short texno;		//Texture table number to use OR color code (depends on draw type)
	short colorBank;	//Color bank to use
	short useClip;		//To clip by system, in user, or outside of user.
	unsigned short extra;	//Operation-specific extra data.
	_spr_type_data type;
} _sprite; //28 bytes each
#define SPRITE_TYPE_BILLBOARD			(0)
#define SPRITE_TYPE_UNSCALED_BILLBOARD	(1)
#define SPRITE_TYPE_3DLINE				(2)
#define SPRITE_TYPE_NORMAL	(3)
#define SPRITE_MESH_STROBE	(4)
#define SPRITE_FLASH_STROBE	(5)
#define SPRITE_BLINK_STROBE	(6)

//////////////////////////////////
// Post-transformed vertice data struct
//////////////////////////////////
typedef struct {
    POINT  pnt;		//x = 0, y = 4, z = 8
	unsigned int clipFlag;	// = 12
} vertex_t; //16 bytes each

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
/*
SIZE parameter:
CMDSIZE of VDP1 Command Table
15		14		13		12		11		10		9		8		7		6		5		4		3		2		1		0
N		N	|				Texture width / 8				||						Texture height						|
13-8: Texture width (in << 3 units)
7-0: Texture height (in integer units)
*/
typedef struct{
	unsigned short SIZE; //VDP1 Size Word
	unsigned short SRCA; //VDP1 Source Address Word (MAP_TO_VRAM)
} paletteCode;


//////////////////////////////////
// Mesh Animation Control Struct
//////////////////////////////////
typedef struct
{
	char reset_enable;
	Uint8 arate[64];
    Uint16 curFrm;
    Uint8 curKeyFrm;
    Uint8 startFrm;
    Uint8 endFrm;
} animationControl;
//////////////////////////////////
// Texture (Sprite) Animation Control Struct
//////////////////////////////////
typedef struct 
{
	int lifetime;
	entity_t * modEnt;
	Uint8 * arates;
	Uint8 * lumas;
	Uint16 sprite_sheet_start;
	Uint16 sprite_sheet_end;
	Uint16 curFrm;
	Uint16 curKeyFrm;
	Uint16 startFrm;
	Uint16 endFrm;
} spriteAnimation;

//////////////////////////////////
// Background layer (NBG1) Spritesheet Animation Control Structs
//////////////////////////////////
typedef struct {
	short wpos[2]; //Window (Screen) Position
	short spos[2]; //Spritesheet Position of keyframe (expressed from top-left coordinate)
	short size[2]; //Keyframe size in spritesheet
} bg_key;

typedef struct {
	int length; //# of keyframes in this animation
	int loop; //Whether this animation loops, or not. 
	bg_key ** keyframes; //Pointer to the keyframes of the animation in the played order
	int * lifetimes; //Pointer to the list of lifetimes of each keyframe in this animation (in fixed-point time units)
	void * sequence; //Pointer to another animation to be played in sequence with this one
} backgroundAnimation;

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

//////////////////////////////////
// Basic Portal/Occluder Stuff
//////////////////////////////////
typedef struct 
{
	int verts[4][3];
	int depth;
	unsigned char sectorA;
	unsigned char sectorB;
	unsigned char type;
	unsigned char backface;
} _portal;

extern _portal * scene_portals;
extern _portal * used_portals;
extern unsigned short * sectorIsAdjacent;
extern unsigned short * sectorIsVisible;
extern unsigned short * visibleSectors;
extern int nearSectorCt;
extern entity_t * drawn_entity_list[64];
extern short drawn_entity_count;
extern MATRIX global_view_matrix;

extern int anims;
extern int scrn_dist;
extern short vert_clip_x;
extern short vert_clip_nx;
extern short vert_clip_y;
extern short vert_clip_ny;
extern unsigned short top_left_erase_pt;
extern unsigned short btm_rite_erase_pt;
extern int send_draw_stats;
extern int hi_res_switch;
extern vertex_t ssh2VertArea[MAX_SSH2_ENTITY_VERTICES+1];
extern vertex_t msh2VertArea[MAX_MSH2_ENTITY_VERTICES+1];
extern _sprite	sprWorkList[MAX_SPRITES];
extern int * ssh2SentPolys;
extern int * msh2SentPolys;
extern int * transVerts;
extern int * transPolys;
extern int * timeComm;
extern int * current_portal_count;
extern int * masterIsDoneDrawing;
extern int * sectorToDrawFrom;
extern paletteCode * pcoTexDefs;

extern point_light * active_lights;

extern int baseAsciiTexno;
extern int sprAsciiHeight;
extern int sprAsciiWidth;
extern _spr_type_data sprite_prep;

extern int animated_texture_list[MAX_SIMULTANEOUS_SPRITE_ANIMATIONS];

//subrender.c
void *	preprocess_planes_to_tiles_for_sector(_sector * sct, void * workAddress);
void	plane_rendering_with_subdivision(entity_t * ent);
void	transform_verts_for_sector(int sector_number, MATRIX * msMatrix);
void	draw_sector(int sector_number, int viewport_sector, MATRIX * msMatrix);
//2drender.c
short	add_to_sprite_list(FIXED * position, short * span, short texno, unsigned short colorBank, _spr_type_data type, short useClip, int lifetime);
void	transform_mesh_point(FIXED * mpt, FIXED * opt, _boundBox * mpara);
void	draw2dSquare(int * firstPt, int * scndPt, unsigned short colorData, unsigned short solid_or_border, unsigned short depth, unsigned short mesh);
void	ssh2BillboardScaledSprite(_sprite * spr);
void	ssh2Line(_sprite * spr);
void	ssh2NormalSprite(_sprite * spr);
void	drawAxis(POINT size);
void	draw_normal_sprite(int xPos, int yPos, unsigned short texno, unsigned short colrBank);
void	spr_print(int xPos, int yPos, char * data);
void	spr_sprintf(int xPos, int yPos, ...);
void	nbg_sprintf(int x, int y,  ...);
void	nbg_sprintf_decimal(int x, int y,  int print_data);
void	spr_sprintf_decimal(int x, int y,  int print_data);
void	nbg_clear_text(void);
short	menu_with_options(__basic_menu * mnu);
//rednerAnim.c
void	clean_sprite_animations(void);
void	operate_texture_animations(void);
void	start_texture_animation(spriteAnimation * anim, entity_t * ent);
void	ssh2DrawAnimation(animationControl * animCtrl, entity_t * ent, Bool transplant);
void	meshAnimProcessing(animationControl * animCtrl, entity_t * ent, Bool transplant);
//render.c
FIXED	trans_pt_by_component(POINT ptx, FIXED * normal);
void	SetFixDiv(FIXED dividend, FIXED divisor); //Defined as "dividend / divisor", for fixed points, using division unit
void	ssh2SetCommand(FIXED * p1, FIXED * p2, FIXED * p3, FIXED * p4, Uint16 cmdctrl, Uint16 cmdpmod, Uint16 cmdsrca, Uint16 cmdcolr, Uint16 cmdsize, Uint16 cmdgrda, FIXED drawPrty);
void	msh2SetCommand(FIXED * p1, FIXED * p2, FIXED * p3, FIXED * p4, Uint16 cmdctrl, Uint16 cmdpmod, Uint16 cmdsrca, Uint16 cmdcolr, Uint16 cmdsize, Uint16 cmdgrda, FIXED drawPrty);
int		process_light(VECTOR lightAngle, FIXED * ambient_light, int * brightness_floor, FIXED * prematrix, char model_purpose);
void	init_render_area(short desired_horizontal_fov);
void	vblank_requirements(void);
void	frame_render_prep(void);
void	collect_portals_from_sector(int sector_number, MATRIX * msMatrix, int * viewport_position);
void	setFramebufferEraseRegion(int xtl, int ytl, int xbr, int ybr);
void	determine_colorbank(unsigned short * colorBank, int * luma);
void	depth_cueing(int * depth, int * cue);
void	preclipping(vertex_t ** ptv, unsigned short * flip, unsigned short * pclp);
void	clipping(vertex_t * pnt, short useClip);
void	setUserClippingAtDepth(int * topLeft, int * btmRight, int zDepthTgt);
void	ssh2DrawModel(entity_t * ent);
void	msh2DrawModel(entity_t * ent, MATRIX msMatrix);
void	sort_master_polys(void);

