/*-------------------------------------------------------------------------*/
/*      Workarea assignment customize file                                 */
/*          for SGL ver. 2.10 (default)                                    */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#include	"sl_def.h"

#define 	HWRAM_END		0x6100000 
/*---- [1.This part must not be modified] ---------------------------------*/
#define		SystemWork		HWRAM_END - 1024			/* System Variable         */

/*---- [2.DO NOT CHANGE STRUCTURE - Changing numbers is OK] ---------------------------------*/

#define		MAX_VERTICES	8		/* number of vertices that can be used */ //Low # defined as SGL renderpath is not used
#define		MAX_POLYGONS	2000	/* number of polygons that can be used */ //High # defined
#define		MAX_EVENTS		1		/* number of events that can be used   */
#define		MAX_WORKS		1		/* number of works that can be used    */

#define		WORK_AREA		SystemWork - (MAX_POLYGONS * 88)	/* SGL Work Area           */

#define		trans_list		SystemWork - (16384)	/* DMA Transfer Table      */
#define		pcmbuf			SoundRAM+0x78000		/* PCM Stream Address      */
#define		PCM_SIZE		0x8						/* PCM Stream Size         */ //Low # defined as SGL soundpath is not used

#define		master_stack	SystemWork				/* MasterSH2 StackPointer  */
#define		slave_stack		0x06001e00				/* SlaveSH2  StackPointer  */

/*---- [3.Macro] ----------------------------------------------------------*/
#define		_Byte_			sizeof(Uint8)
#define		_Word_			sizeof(Uint16)
#define		_LongWord_		sizeof(Uint32)
#define		_Sprite_		(sizeof(Uint16) * 18)

#define		AdjWork(pt,sz,ct)	(pt + (sz) * (ct))

/*---- [4.Work Area] ------------------------------------------------------*/
    enum workarea{
        sort_list  = WORK_AREA ,
        zbuffer    = AdjWork(sort_list , _LongWord_ * 3, MAX_POLYGONS + 6) , // 12 * (MAX_POLYGONS + 6)
        spritebuf  = AdjWork(zbuffer   , _LongWord_, 512) ,					// 2048
        pbuffer    = AdjWork(spritebuf , _Sprite_, (MAX_POLYGONS + 6) * 2) , // 18 * ((MAX_POLYGONS + 6) *2)
        clofstbuf  = AdjWork(pbuffer   , _LongWord_ * 4, MAX_VERTICES) , // 16 * MAX_VERTICES
        commandbuf = AdjWork(clofstbuf , _Byte_ * 32*3, 32) , // 3072
        NextEntry  = AdjWork(commandbuf, _LongWord_ * 8, MAX_POLYGONS) // 32 * MAX_POLYGONS
    } ;
	// Work Area Size : MAX_POLYGONS * (88) (estimate, not exact)
/*---- [5.Variable area ] -------------------------------------------------*/
    const void*   MasterStack   = (void*)(master_stack) ;
    const void*   SlaveStack    = (void*)(slave_stack) ;
    const Uint16  MaxVertices   = MAX_VERTICES ;
    const Uint16  MaxPolygons   = MAX_POLYGONS ;
    const Uint16  EventSize     = sizeof(EVENT) ;
    const Uint16  WorkSize      = sizeof(WORK) ;
    const Uint16  MaxEvents     = MAX_EVENTS ;
    const Uint16  MaxWorks      = MAX_WORKS ;
    const void*   SortList      = (void*)(sort_list) ;
    const Uint32  SortListSize  = (MAX_POLYGONS + 6) * _LongWord_ * 3 ;
    const void*   Zbuffer       = (void*)(zbuffer) ;
    const SPRITE_T*   SpriteBuf     = (void*)(spritebuf) ;
    const Uint32  SpriteBufSize = _Sprite_ * (MAX_POLYGONS + 6) * 2 ;
    const void*   Pbuffer       = (void*)(pbuffer) ;
    const void*   CLOfstBuf     = (void*)(clofstbuf) ;
    const void*   CommandBuf    = (void*)(commandbuf) ;
    const void*   PCM_Work      = (void*)(pcmbuf) ;
    const Uint32  PCM_WkSize    = PCM_SIZE ;
    const void*   TransList     = (void*)(trans_list) ;

    EVENT  EventBuf[MAX_EVENTS] ;
    WORK   WorkBuf[MAX_WORKS] ;
    EVENT* RemainEvent[MAX_EVENTS] ;
    WORK*  RemainWork[MAX_WORKS] ;

/*------------------------------------------------------------------------*/

