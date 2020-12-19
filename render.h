#ifndef __RENDER_H__
# define __RENDER_H__

#include "jo/jo.h"
#include "mloader.h"
#include "def.h"
#include "mymath.h"

#define VRAM_TEXTURE_BASE (0x10000) //Matches jo engine specification
#define VDP1_VRAM 0x25C00000
#define MAP_TO_VRAM(sh2map_vram_addr) ((sh2map_vram_addr - VDP1_VRAM)>>3) 
#define INTERNAL_MAX_POLY 2600 //Slave only 1700
#define INTERNAL_MAX_VERTS 2800 //Slave only 2800
#define MAX_SSH2_SENT_POLYS (1200) //SpriteBuf size limitation // thanks VBT for fixing sglarea.o for me
#define MAX_MSH2_SENT_POLYS (800) //SpriteBuf size limitation 
#define SCR_SCALE_X (16)
#define SCR_SCALE_Y (16)
//VDP1 perf limit depends on how many pixels it's drawing.
//Screen clip flags
#define SCRN_CLIP_X (1)
#define SCRN_CLIP_NX (1<<1)
#define SCRN_CLIP_Y (1<<2)
#define SCRN_CLIP_NY (1<<3)
#define CLIP_Z (1<<4)

//////////////////////////////////
// Post-transformed vertice data struct
//////////////////////////////////
typedef struct{
    POINT  pnt;
	char clipFlag;
} vertex_t; //20 bytes each

//////////////////////////////////
// Point light data struct
//////////////////////////////////
typedef struct{
	FIXED * ambient_light;
	int	pos[3];
	unsigned short bright;
	unsigned char pop;
} point_light;

//////////////////////////////////
// Palette Coded (Indexed Color) texture definition
//////////////////////////////////
typedef struct{
	unsigned short SIZE; //VDP1 Size Word
	unsigned short SRCA; //VDP1 Source Address Word (MAP_TO_VRAM)
} paletteCode;

extern int * DVSR;
extern int * DVDNTH;
extern int * DVDNTL;
extern FIXED nearP;
extern FIXED farP;
extern vertex_t ssh2VertArea[500];
extern vertex_t msh2VertArea[650];
extern int * ssh2SentPolys;
extern int * msh2SentPolys;
extern int * transVerts;
extern int * transPolys;
extern paletteCode * pcoTexDefs;

extern point_light * active_lights;

extern FIXED MsScreenDist;
extern FIXED MsZlimit;

FIXED	trans_pt_by_component(POINT ptx, FIXED * normal);
void	SetFixDiv(FIXED dividend, FIXED divisor); //Defined as "dividend / divisor", for fixed points, using division unit
void	ssh2SetCommand(FIXED * p1, FIXED * p2, FIXED * p3, FIXED * p4, Uint16 cmdctrl, Uint16 cmdpmod, Uint16 cmdsrca, Uint16 cmdcolr, Uint16 cmdsize, Uint16 cmdgrda, FIXED drawPrty);
void	msh2SetCommand(FIXED * p1, FIXED * p2, FIXED * p3, FIXED * p4, Uint16 cmdctrl, Uint16 cmdpmod, Uint16 cmdsrca, Uint16 cmdcolr, Uint16 cmdsize, Uint16 cmdgrda, FIXED drawPrty);
void	init_render_area(void);
void	vblank_requirements(void);
void	frame_render_prep(void);
void	update_gamespeed(void);
void	ssh2DrawModel(entity_t * ent, POINT wldPos);
void	msh2DrawModel(entity_t * ent, MATRIX msMatrix, FIXED * lightSrc);
void	ssh2DrawAnimation(animationControl * animCtrl, entity_t * ent, POINT wldPos, bool transplant);
void	sort_master_polys(void);
#endif

