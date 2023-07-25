
#ifndef __MENU_H__
# define __MENU_H__

#define HUD_LAYER_START		(0)
#define HUD_LAYER_DEBUG		(1)
#define HUD_LAYER_LEVEL		(2)
#define HUD_LAYER_OPTION_1	(3)
#define HUD_LAYER_OPTION_2	(4)
#define HUD_LAYER_TEXVIEWER	(5)

#define HUD_EVENT_TYPES (32)
#define HUD_EVENT_START (1)
#define HUD_EVENT_RUN	(2)
#define HUD_EVENT_DONE	(3)
#define HUD_EVENT_CLOSE	(0)
#define EVENT_NO_STROBE		(0)
#define EVENT_STROBE_MESH	('M')
#define EVENT_STROBE_FLASH	('F')
#define EVENT_STROBE_BLINK	('O')

#define RING1_EVENT				(1)
#define RING2_EVENT				(2)
#define RING3_EVENT				(3)
#define RING4_EVENT				(4)
#define RING5_EVENT				(5)
#define	RING6_EVENT				(6)
#define RING7_EVENT				(7)
#define RINGS_ALL_EVENT			(0)
#define GATE_DISCOVERY_EVENT	(8)
#define TRACK_DISCOVERED_EVENT	(9)
#define GATE_PASSED_EVENT		(10)
#define TRACK_FAILED_EVENT		(11)
#define TRACK_WIN_EVENT			(12)

#define FLAG_TAKEN_EVENT		(13)
#define FLAG_CAPTURED_EVENT		(14)
#define FLAG_RETURNED_EVENT		(15)
#define FLAG_OPEN_EVENT			(16)
//More events for track win, track fail, flag taken, flag return, flag cap, etc
	// HUD Event
	// What do I need it to do?
	// 1. Spawn sprite
	// 2. Move sprite from (start pos) to (end pos)
	// 3. Possibly strobe the sprite (mesh on/off or sprite on/off)
	// 4. Play sound at (time) or (end)	--------	(i need a way to arbitrate this to either ADX Stream or from-ram)
	// 5. Possibly do other things after that??? but not needed right now.
typedef struct {
	int startPos[3];
	int endPos[3];
	int eventTime;
	int spriteTime;
	int screenStep;
	short status;
	short soundType;
	short soundNum;
	short volume;
	short strobe_type;
	unsigned short strobe_interval; //Note: Being a short, it rolls over before 1 second (65535). Use ~62,000 as 1s.
	short texno;
	short colorBank;
	int * printedData;
	char * text;
	_sprite * spr;
} _hudEvent;

extern int viewInfoTxt;
extern int baseRingMenuTexno;
extern _hudEvent hudEvents[HUD_EVENT_TYPES];

void	start_menu(void);
void	init_hud_events(void);
void	start_hud_event(short eventNum);
void	hud_menu(void);


#endif

