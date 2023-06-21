#ifndef __INPUT_H__
# define __INPUT_H__

#define TYPE_DIGITAL    0x00

#define DIGI_L            0x0008
#define DIGI_Z            0x0010
#define DIGI_Y            0x0020
#define DIGI_X            0x0040
#define DIGI_R            0x0080
#define DIGI_B            0x0100
#define DIGI_C            0x0200
#define DIGI_A            0x0400
#define DIGI_START        0x0800
#define DIGI_UP           0x1000
#define DIGI_DOWN         0x2000
#define DIGI_LEFT         0x4000
#define DIGI_RIGHT        0x8000
#define DIGI_DIRECTIONS   0xF000
#define DIGI_ALL          0x0FFF

void	operate_digital_pad1(void);

typedef struct {
	unsigned short pressed;
	unsigned short prevPressed;
	unsigned short frameHeld;
	unsigned short up;
	unsigned short down;
	unsigned short change;
	unsigned short toggle;
} digital_pad;

typedef struct {
	unsigned char active; //Is analogue input active, or not?
	unsigned char ax; // Stick X-Axis
	unsigned char ay; // Stick Y-Axis
	unsigned char lta; // Left trigger
	unsigned char rta; // Right trigger
	unsigned char dax; // Delta X-Axis
	unsigned char day; // Delta Y-Axis
	unsigned char ltd; // Left trigger delta
	unsigned char rtd; // Right trigger delta
	unsigned char tax; // X-Axis Tolerance 
	unsigned char tay; // Y-Axis Tolerance
	unsigned char taz; // Trigger' tolerance
} analog_ext;

extern digital_pad pad1;
extern analog_ext apd1;

void	reset_pad(digital_pad * pad);
int		is_key_pressed(int keyPattern);
int		is_key_up(int keyPattern);
int		is_key_down(int keyPattern);
int		is_key_change(int keyPattern);
int		is_key_toggle(int keyPattern);
int		is_key_release(int keyPattern);
int		is_key_struck(int keyPattern);

#endif

