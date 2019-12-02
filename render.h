#ifndef __RENDER_H__
# define __RENDER_H__

#include "jo/jo.h"
#include "ZT/ZT_COMMON.h"
#include "def.h"
#include "mymath.h"

typedef struct{
    POINT  pnt;
	char clipFlag;
	short vbright;
    FIXED  inverseZ;
} vertex_t; //20 bytes each

extern int * ssh2SentPolys;
extern int * msh2SentPolys;
extern int * transVerts;
extern int * transPolys;

extern FIXED MsScreenDist;
extern FIXED MsZlimit;

void	init_render_area(void);
void	vblank_requirements(void);
void	frame_render_prep(void);
void	update_gamespeed(void);
void	ssh2DrawModel(entity_t * ent, POINT lightSrc);
void	msh2DrawModel(entity_t * ent, MATRIX msMatrix, FIXED * lightSrc);
void	ssh2DrawAnimation(animationControl * animCtrl, entity_t * ent, POINT lightSrc);
void	sort_master_polys(void);
#endif

