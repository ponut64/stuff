#ifndef __RENDER_H__
# define __RENDER_H__

#define VRAM_TEXTURE_BASE (0x10000) //Matches jo engine specification
#define VDP1_VRAM 0x25C00000
#define MAP_TO_VRAM(sh2map_vram_addr) ((sh2map_vram_addr - VDP1_VRAM)>>3) 
#define INTERNAL_MAX_POLY 2600 //Slave only 1700
#define INTERNAL_MAX_VERTS 2800 //Slave only 2800
#define MAX_SSH2_SENT_POLYS (700) //SpriteBuf size limitation // thanks VBT for fixing sglarea.o for me
#define MAX_MSH2_SENT_POLYS (600) //SpriteBuf size limitation 
//
#define SCR_SCALE_X (16)
#define SCR_SCALE_Y (16)
// Base PMOD: Bit 12 is HSS
#define VDP1_BASE_PMODE (0x1490)
// CMDCTRL = Select Distorted Sprite
#define VDP1_BASE_CMDCTRL (2)
#define VDP1_PRECLIPPING_DISABLE (2048)
//VDP1 perf limit depends on how many pixels it's drawing.
//Screen clip flags
#define SCRN_CLIP_X		(1)
#define SCRN_CLIP_NX	(1<<1)
#define SCRN_CLIP_Y		(1<<2)
#define SCRN_CLIP_NY	(1<<3)
#define CLIP_Z 			(1<<4)
#define LOW_Z 			(1<<5)

#define NEAR_PLANE_DISTANCE (15<<16) //The minimum Z allowed
#define FAR_PLANE_DISTANCE	(1000<<16) //The maximum Z allowed

#define	MAX_DYNAMIC_LIGHTS (2)

#define MAX_SPRITES (64)

/*
Render data flags:
	
	|	0		|	1		|	2		|	3		|	4	-	5 	|	6	-	7 |		
	  Dual-plane	Mesh		Physical	MSB On		Tex. flip	  Sorting rule
*/
	#define GV_FLAG_SINGLE	(0x1) // Zero, dual-plane. One, single-plane.
	#define GV_FLAG_MESH	(0x2) // Zero, no mesh. One, mesh.
	#define GV_FLAG_PHYS	(0x4) // Zero, physical plane (in supported objects). Zero, no collision with plane.
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
	int time;			// Time (in fixed-point seconds) to allow the sprite to persist.
	POINT pos; 			//World-space position for billboard scaled sprites, screenspace top-left coordinate otherwise
	short span; 		//Screenspace X/Y span, if a billboard.
	short texno;		//Texture table number to use
	unsigned char mesh;	//Boolean. 1 enables mesh effect drawing.
	char type; 			//"B" for billboard, "S" for normal sprite.
} _sprite; //22 bytes each

//////////////////////////////////
// Post-transformed vertice data struct
//////////////////////////////////
typedef struct {
    POINT  pnt;
	short clipFlag;
} vertex_t; //13 bytes each

//////////////////////////////////
// Point light data struct
//////////////////////////////////
typedef struct {
	FIXED * ambient_light;
	int	pos[3];
	unsigned short bright;
	unsigned short pop;
} point_light;

//////////////////////////////////
// Palette Coded (Indexed Color) texture definition
//////////////////////////////////
typedef struct{
	unsigned short SIZE; //VDP1 Size Word
	unsigned short SRCA; //VDP1 Source Address Word (MAP_TO_VRAM)
} paletteCode;

extern entity_t * drawn_entity_list[64];
extern short drawn_entity_count;
extern MATRIX global_view_matrix;

extern int * DVSR;
extern int * DVDNTH;
extern int * DVDNTL;
extern vertex_t ssh2VertArea[500];
extern vertex_t msh2VertArea[300];
extern _sprite	sprWorkList[MAX_SPRITES];
extern int * ssh2SentPolys;
extern int * msh2SentPolys;
extern int * transVerts;
extern int * transPolys;
extern paletteCode * pcoTexDefs;

extern point_light * active_lights;

extern FIXED MsScreenDist;
extern FIXED MsZlimit;

void	transform_mesh_point(FIXED * mpt, FIXED * opt, _boundBox * mpara);
void	ssh2BillboardScaledSprite(_sprite * spr);
void	add_to_sprite_list(FIXED * position, short span, short texno, unsigned char mesh, char type, int time);

FIXED	trans_pt_by_component(POINT ptx, FIXED * normal);
void	SetFixDiv(FIXED dividend, FIXED divisor); //Defined as "dividend / divisor", for fixed points, using division unit
void	ssh2SetCommand(FIXED * p1, FIXED * p2, FIXED * p3, FIXED * p4, Uint16 cmdctrl, Uint16 cmdpmod, Uint16 cmdsrca, Uint16 cmdcolr, Uint16 cmdsize, Uint16 cmdgrda, FIXED drawPrty);
void	msh2SetCommand(FIXED * p1, FIXED * p2, FIXED * p3, FIXED * p4, Uint16 cmdctrl, Uint16 cmdpmod, Uint16 cmdsrca, Uint16 cmdcolr, Uint16 cmdsize, Uint16 cmdgrda, FIXED drawPrty);
void	init_render_area(void);
void	vblank_requirements(void);
void	frame_render_prep(void);
void	update_gamespeed(void);
void	ssh2DrawModel(entity_t * ent);
void	msh2DrawModel(entity_t * ent, MATRIX msMatrix, FIXED * lightSrc);
void	ssh2DrawAnimation(animationControl * animCtrl, entity_t * ent, bool transplant);
void	sort_master_polys(void);
#endif

