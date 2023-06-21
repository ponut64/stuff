//
//input.c
//this file is compiled separately
//Partially sourced from libyaul
//Directly accesses hardware registers and thus is library agnostic.
//Though I don't start the SMPC or set a peripheral mode. Dear god why is sampling inputs so complex? Ugh!

#include <sl_def.h>
#include "input.h"
#include "def.h"

//There are 32 of these
//These are oddly arranged:
//The first one is at a byte-only alignment. Only byte access is allowed.
//Each following OREG is thusly two bytes ahead of the last one, remaining on an odd byte access.
#define SMPC_OREG_START ((unsigned char *)0x20100021)
#define SMPC_IREG1 ((unsigned char *)0x20100003)
unsigned char * oregs = SMPC_OREG_START;
unsigned char * ireg1 = SMPC_IREG1;

digital_pad pad1 = {.pressed = 0xFFFF,
					.prevPressed = 0xFFFF,
					.frameHeld = 0xFFFF,
					.up = 0,
					.down = 0xFFFF,
					.change = 0,
					.toggle = 0xFFFF};

analog_ext apd1;	
	
void	get_pad1_analog_data(void)
{
	//oregs[8] = ax
	//oregs[10] = ay
	//oregs[12] = left trigger?
	//oregs[14] = right trigger?
	//hopefully?
	//
	// It appears that:
	// 
	// ax: Left-right. 0 is left, 255 is right.
	// ay: up-down. 0 is up, 255 is down.
	// Triggers: I guess assume we have from 128 to 28?
	//
	apd1.dax = apd1.ax - oregs[8];
	apd1.day = apd1.ay - oregs[10];
	apd1.ltd = apd1.lta - oregs[12];
	apd1.rtd = apd1.rta - oregs[14];
	
	apd1.ax = oregs[8];
	apd1.ay = oregs[10];
	apd1.lta = oregs[12];
	apd1.rta = oregs[14];
	
}

//(Put this in vblank)
void	operate_digital_pad1(void)
{
	//oregs[0] = Specifying .. something?
	//oregs[2] = specifying digital pad
	//oregs[4] = beginning of data
	//oregs[6] = end of digital pad data
	
	pad1.pressed = oregs[6] | (oregs[4]<<8); //Holds current frame data
	
	pad1.change = pad1.pressed ^ pad1.prevPressed;
	pad1.frameHeld ^= pad1.change;
	
	pad1.up ^= pad1.change;
	pad1.down = (pad1.change != 0) ? pad1.down ^ pad1.change : pad1.down;
	
	pad1.toggle ^= (pad1.change != 0) ? pad1.up : 0;
	
	pad1.prevPressed = pad1.pressed;
	
	//OREG[2] being '22' specifies 3D Control Pad.
	if(oregs[2] == 22)
	{
	apd1.active = true;
	get_pad1_analog_data();
	} else {
	apd1.active = false;
	}
}

//Specifically for the operation of "is_key_change" to function at 30 hz, 20hz, etc
//Place once anywhere in game loop
void	reset_pad(digital_pad * pad)
{

	pad->frameHeld = 0;

}

int	is_key_pressed(int keyPattern)
{
	if( (pad1.pressed & keyPattern) != keyPattern)
	{
		return 1;
	} else {
		return 0;
	}
}

int	is_key_up(int keyPattern)
{
	if( (pad1.up & keyPattern) != keyPattern)
	{
		return 1;
	} else {
		return 0;
	}
}

int	is_key_down(int keyPattern)
{
	if( (pad1.down & keyPattern) != keyPattern)
	{
		return 1;
	} else {
		return 0;
	}
}


int	is_key_change(int keyPattern)
{
	if( (pad1.frameHeld & keyPattern) == keyPattern)
	{
		return 1;
	} else {
		return 0;
	}
}

int	is_key_toggle(int keyPattern)
{
	if( (pad1.toggle & keyPattern) != keyPattern)
	{
		return 1;
	} else {
		return 0;
	}
}

int is_key_release(int keyPattern)
{
	return (is_key_up(keyPattern) & is_key_change(keyPattern));
}

int is_key_struck(int keyPattern)
{
	return (is_key_down(keyPattern) & is_key_change(keyPattern));
}


