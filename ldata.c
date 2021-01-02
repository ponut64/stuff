//ldata.c
//this file is compiled separately
#include "ldata.h"

bool ldata_ready = false;

void	process_tga_as_ldata(void)
{
	
	unsigned char * readByte = (unsigned char *)dirty_buf;
	
	unsigned char id_field_size = readByte[0];
	unsigned char col_map_type = readByte[1]; 
	unsigned char data_type = readByte[2];	
	
	unsigned char xSizeLoBits = readByte[12];
	unsigned char xSizeHiBits = readByte[13];
	unsigned char ySizeLoBits = readByte[14];
	unsigned char ySizeHiBits = readByte[15];
	unsigned short xSize = xSizeLoBits | (xSizeHiBits<<8);
	unsigned short ySize = ySizeLoBits | (ySizeHiBits<<8);
	unsigned char byteFormat = readByte[16];
	
	if(col_map_type != 0){
		jo_printf(0, 0, "(REJECTED NON-RGB TGA)");
		return;
	}
	
	if(data_type != 2) {
		jo_printf(0, 0, "(REJECTED RLE TGA)");
		return;
	}
	//Color Map Specification Data is ignored.
	
	//X / Y origin data is ignored.
	
	if(byteFormat != 24){
		jo_printf(0, 0, "(TGA NOT 24BPP)");
		return; //File Typing Demands 24 bpp.
	}
	
	
//I want to make a command line executable that can parse an image file into these declarations.
//Conceptual: 24bit RGB TGA
//Bits 0-4: Object type
//Bits 5-8: (Vertical offset)
//Bits 9-11: Rotation sign bits
//Bits 12-15: (x rotation)<<8 [degrees]
//Bits 16-19: (y rotation)<<12
//Bits 20-23: (z rotation)<<16
//Object types: 32 max
//Height increments: 4[<<16] | 60[<<16] max, unsigned
//Rotational increments: 16 degrees / 240 max, signed [via sign bits]
	
	unsigned int imdat = id_field_size + 18;
	//int totalPix = xSize * ySize;
	//int yspot;
	
	//	jo_printf(0, 0, "(%i)", totalPix);
	
	objNEW = 0;
	for(int k = 0; k < 8; k++)
	{
		link_starts[k] = -1; //Re-set link starts to no links conidition
	} 
	
		for(int i = 0; i < ySize; i++)
		{
		//jo_printf(0, 0, "(DECL)"); //Debug ONLY
			//yspot = i * ySize;
			for(int k = 0; k < xSize; k++)
			{
		//If the pixel is all high (white), dont use it.
		if(readByte[imdat] != 0xFF && readByte[imdat+1] != 0xFF && readByte[imdat+2] != 0xFF) 
		{
			//Item location x y z, item type, item rotation x y z.
	declare_object_at_cell(k-(xSize>>1), -((readByte[imdat] & 0xE0) | ((readByte[imdat+1] & 1)<<8)), i-(ySize>>1), readByte[imdat] & 0x1F, 
	(readByte[imdat+1] & 2) ? -(readByte[imdat+1] & 0xF0) : (readByte[imdat+1] & 0xF0),
	(readByte[imdat+1] & 4) ? -(readByte[imdat+2] & 0xF)<<4 : (readByte[imdat+2] & 0xF)<<4,
	(readByte[imdat+1] & 8) ? -(readByte[imdat+2] & 0xF0) : (readByte[imdat+2] & 0xF0));
		}
			imdat += 3;
			}
		}

	declarations();
	ldata_ready = true;

}


