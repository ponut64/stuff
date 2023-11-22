//
// object_lister
//
// Simple program:
// 1. Takes object declaration function
// a. Writes OBJECTS header
// 2. Writes arguments of function to file, in sequence
// Planned:
// A. Takes object type list
// B. Writes TYPES header
// C. Writes object type list
//
// b. Takes texture loading function 
// c. Writes TEXTURES header
// d. Writes arguments of function, in sequence
//
// 3. Takes model loading function
// e. Writes MODELS header
// 4. Writes arguments of function to file in sequence
//
// 5. Takes sound loading function
// f. Writes SOUNDS header
// 6. Writes arguments of function to file in sequence
//

using namespace std;
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include "tools.cpp"

unsigned short objects_data_list[256 * 1024]; //Few MB or so eh?
unsigned short * dptr;
int adder;
int obj_count;

string music1;
string music2;
string music3;
string texNames[5];
string palName;
string bgName;

void	declare_object_at_cell(short pixX, short height, short pixY, short type, short xrot, short yrot, short zrot, unsigned short more_data, unsigned short eeOrData)
{
	*dptr++ = pixX;
	*dptr++ = height;
	*dptr++ = pixY;
	*dptr++ = type;
	*dptr++ = xrot;
	*dptr++ = yrot;
	*dptr++ = zrot;
	*dptr++ = more_data;
	*dptr++ = eeOrData;
	adder += 9; //Nine arguments...
	obj_count++;
}

void	create_objects(int levelNo)
{
	
	if(levelNo == 0)
	{
		music1 = "BATLOBBY.MUS";
		music2 = "MOVEIT.MUS";
		music3 = "DRONK.MUS";
		texNames[0] = "DIR0.TGA";
		texNames[1] = "DIR1.TGA";
		texNames[2] = "DIR2.TGA";
		texNames[3] = "DIR3.TGA";
		texNames[4] = "DIR4.TGA";
		palName = "PAL0.TGA";
		bgName = "BG0.TGA";
		
		declare_object_at_cell((340 / 40) + 1, -333, (20 / 40), 51 /*start stand*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((0 / 40) + 1, -0, (0 / 40), 60 /*track data*/, 0, 0, 0, (12<<8), 0);
		
		declare_object_at_cell(-(20 / 40) + 1, -269, -(20 / 40), 53 /*flag stand*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((160 / 40) + 1, -133, (880.0 / 40), 54 /*goal stand*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((160 / 40) + 1, -(133 + 15), (880.0 / 40), 58 /*flag goal*/, 0, 0, 0, 10, 0);
		
		declare_object_at_cell((20 / 40) + 1, -227, -(260 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((20 / 40) + 1, -227, (260 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
		
		declare_object_at_cell(-(380 / 40) + 1, -315, -(860 / 40), 39 /*post01*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(180 / 40) + 1, -315, -(860 / 40), 39 /*post01*/, 0, 0, 0, 0, 0);
		
		declare_object_at_cell(-(340 / 40) + 1, -239, (20 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);
		declare_object_at_cell(-(220 / 40) + 1, -239, -(20 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);
		
		declare_object_at_cell(-(380 / 40) + 1, -310, (940 / 40), 40 /*post02*/, 0, 0, 0, 0, 2<<4);
		declare_object_at_cell(-(180 / 40) + 1, -310, (940 / 40), 40 /*post02*/, 0, 0, 0, 0, 2<<4);
		
		declare_object_at_cell((260 / 40) + 1, -431, (20 / 40), 1 /*t item*/, 0, 0, 0, 		0, 0);
		declare_object_at_cell(-(260 / 40) + 1, -159, -(1260 / 40), 2 /*t item*/, 0, 15, 0, 	0, 0);
		declare_object_at_cell((20 / 40) + 1, -230, -(460 / 40), 3 /*t item*/, 0, 30, 0, 	0, 0);
		declare_object_at_cell((20 / 40) + 1, -230, (460 / 40), 4 /*t item*/, 0, 45, 0, 	0, 0);
		declare_object_at_cell(-(260 / 40) + 1, -155, (1260 / 40), 5 /*t item*/, 0, 90, 0, 	0, 0);
		declare_object_at_cell((180 / 40) + 1, -286, -(900 / 40), 6 /*t item*/, 0, 105, 0, 	0, 0);
		declare_object_at_cell(-(20 / 40) + 1, -281, (1220 / 40), 7 /*t item*/, 0, 120, 0, 	0, 0);
		
		declare_object_at_cell((60 / 40) + 1, -281, -(60 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
		//declare_object_at_cell((60 / 40) + 1, -281, -(60 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 19, 0);
		
		declare_object_at_cell((100 / 40) + 1, -145, (820 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
		//declare_object_at_cell((100 / 40) + 1, -145, (820 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 20, 0);
		
		declare_object_at_cell(-(220 / 40) + 1, -184, -(20 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
		//declare_object_at_cell(-(220 / 40) + 1, -184, -(20 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 21, 0);
		
		declare_object_at_cell(-(180 / 40) + 1, -259, -(940 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
		//declare_object_at_cell(-(180 / 40) + 1, -259, -(940 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 22, 0);
		
		declare_object_at_cell(-(300 / 40) + 1, -254, (940 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
		//declare_object_at_cell(-(300 / 40) + 1, -254, (940 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 23, 0);
		
		declare_object_at_cell((180 / 40) + 1, -329, -(20 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
		//declare_object_at_cell((180 / 40) + 1, -329, -(20 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 24, 0);
		
		declare_object_at_cell((20 / 40) + 1, -235, (500 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
		//declare_object_at_cell((20 / 40) + 1, -235, (500 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 25, 0);

		declare_object_at_cell((20 / 40) + 1, -235, -(500 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
		//declare_object_at_cell((20 / 40) + 1, -235, -(500 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 26, 0);
		
		declare_object_at_cell((180 / 40) + 1, -192, -(900 / 40), 14 /*greece03*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(20 / 40) + 1, -187, (1220 / 40), 14 /*greece03*/, 0, 45, 0, 0, 0);
		
		declare_object_at_cell((260 / 40) + 1, -343, (20 / 40), 22 /*float03*/, 0, -45, 0, 0, 0);
		
		// declare_object_at_cell((0 / 40) + 1, -20, (0 / 40), 51 /*start stand*/, 0, 0, 0, 0, 0);
		// declare_object_at_cell((260 / 40), -10, (140 / 40), 1 /*t item*/, 0, 0, 0, 		0, 0);
		// declare_object_at_cell((260 / 40), -10, (180 / 40), 2 /*t item*/, 0, 15, 0, 	0, 0);
		// declare_object_at_cell((260 / 40), -10, (220 / 40), 3 /*t item*/, 0, 30, 0, 	0, 0);
		// declare_object_at_cell((260 / 40), -10, (260 / 40), 4 /*t item*/, 0, 45, 0, 	0, 0);
		// declare_object_at_cell((260 / 40), -10, (300 / 40), 5 /*t item*/, 0, 90, 0, 	0, 0);
		// declare_object_at_cell((260 / 40), -10, (340 / 40), 6 /*t item*/, 0, 105, 0, 	0, 0);
		// declare_object_at_cell((260 / 40), -10, (380 / 40), 7 /*t item*/, 0, 120, 0, 	0, 0);
		
		// declare_object_at_cell(-(300 / 40) + 1, -4, -(340 / 40), 53 /*flag stand*/, 0, 0, 0, 0, 0);
		// declare_object_at_cell((340 / 40) + 1, -5, -(340 / 40), 54 /*goal stand*/, 0, 0, 0, 0, 0);
		
		// declare_object_at_cell((120 / 40) + 1, -100, -(0 / 40), 22 /*float03*/, 0, 0, 0, 0, 0);
		
		// declare_object_at_cell((0 / 40) + 1, -100, (200 / 40), 36 /*tgate0*/, 0, 90, 0, 0, 0);
		
	} else if(levelNo == 1)
	{
		music1 = "DASHY.MUS";
		music2 = "PLAYRUL.MUS";
		music3 = "LONKR.MUS";
		texNames[0] = "DIR1B.TGA";
		texNames[1] = "DIR1B.TGA";
		texNames[2] = "DIR1C.TGA";
		texNames[3] = "DIR1C.TGA";
		texNames[4] = "DIR1A.TGA";
		palName = "PAL0.TGA";
		bgName = "BG1.TGA";
		
		declare_object_at_cell(-(180 / 40) + 1, -120, -(620 / 40), 51 /*start stand*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((0 / 40) + 1, -0, (0 / 40), 60 /*track data*/, 0, 0, 0, (20<<8), 0);

		declare_object_at_cell((540 / 40) + 1, -320, -(180 / 40), 39 /*post01*/, 0, 0, 0, 0, 0);		
		declare_object_at_cell((540 / 40) + 1, -320, -(20 / 40), 39 /*post01*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(1020 / 40) + 1, -320, -(180 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);
		declare_object_at_cell(-(1020 / 40) + 1, -320, -(20 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);
		declare_object_at_cell(-(1420 / 40) + 1, -368, (1180 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);
		declare_object_at_cell(-(1420 / 40) + 1, -368, (1020 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);
		declare_object_at_cell((340 / 40) + 1, -437, (1260 / 40), 18 /*post00*/, 0, 0, 0, 0, 3<<4);
		declare_object_at_cell((340 / 40) + 1, -437, (1100 / 40), 18 /*post00*/, 0, 0, 0, 0, 3<<4);
		declare_object_at_cell((2020 / 40) + 1, -444, (580 / 40), 18 /*post00*/, 0, 0, 0, 0, 4<<4);
		declare_object_at_cell((1860 / 40) + 1, -444, (580 / 40), 18 /*post00*/, 0, 0, 0, 0, 4<<4);
		declare_object_at_cell((1580 / 40) + 1, -434, -(980 / 40), 40 /*post02*/, 0, 0, 0, 0, 5<<4);
		declare_object_at_cell((1740 / 40) + 1, -434, -(980 / 40), 40 /*post02*/, 0, 0, 0, 0, 5<<4);
		
		declare_object_at_cell((2140 / 40) + 1, -510, -(1420 / 40), 1 /*t item*/, 0, 0, 0, 		0, 0);
		declare_object_at_cell((660 / 40) + 1, -256, -(860 / 40), 2 /*t item*/, 0, 15, 0, 	0, 0);
		declare_object_at_cell((1020 / 40) + 1, -112, -(100 / 40), 3 /*t item*/, 0, 30, 0, 	0, 0);
		declare_object_at_cell((700 / 40) + 1, -320, (660 / 40), 4 /*t item*/, 0, 45, 0, 	0, 0);
		declare_object_at_cell(-(260 / 40) + 1, -450, (1340 / 40), 5 /*t item*/, 0, 90, 0, 	0, 0);
		declare_object_at_cell(-(900 / 40) + 1, -203, -(100 / 40), 6 /*t item*/, 0, 105, 0, 	0, 0);
		declare_object_at_cell(-(1380 / 40) + 1, -269, -(220 / 40), 7 /*t item*/, 0, 120, 0, 	0, 0);
		
		declare_object_at_cell(-(1820 / 40) + 1, -275, (940 / 40), 53 /*flag stand*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((580 / 40) + 1, -450, -(1260 / 40), 54 /*goal stand*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((580 / 40) + 1, -(450 + 15), -(1260 / 40), 58 /*flag goal*/, 0, 0, 0, 20, 0);
		
		declare_object_at_cell(-(900 / 40) + 1, -248, -(380 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(620 / 40) + 1, -248, (260 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((340 / 40) + 1, -288, (660 / 40), 10 /*platf00*/, 0, 45, 0, 0, 0);
		declare_object_at_cell((1060 / 40) + 1, -297, (620 / 40), 10 /*platf00*/, 0, 45, 0, 0, 0);
		declare_object_at_cell((1660 / 40) + 1, -316, -(820 / 40), 10 /*platf00*/, 0, 45, 0, 0, 0);
		
		declare_object_at_cell(-(260 / 40) + 1, -368, (1100 / 40), 12 /*greece01*/, 0, 90, 0, 0, 0);
		declare_object_at_cell(-(260 / 40) + 1, -368, (860 / 40), 13 /*greece02*/, 0, 270, 0, 0, 0);
		declare_object_at_cell(-(260 / 40) + 1, -368, (1340 / 40), 13 /*greece02*/, 0, 90, 0, 0, 0);
		
		
		declare_object_at_cell(-(260 / 40) + 1, -302, (580 / 40), 16 /*overhang*/, 0, 90, 0, 0, 0);
		declare_object_at_cell((1540 / 40) + 1, -302, (500 / 40), 16 /*overhang*/, 0, 45, 0, 0, 0);
		declare_object_at_cell((1740 / 40) + 1, -259, (1060 / 40), 16 /*overhang*/, 0, 45, 0, 0, 0);
		declare_object_at_cell((1660 / 40) + 1, -313, -(940 / 40), 16 /*overhang*/, 0, 90, 0, 0, 0);
		
		declare_object_at_cell((660 / 40) + 1, -377, (1180 / 40), 25 /*float01*/, 0, 90, 0, 0, 0);
		
		declare_object_at_cell(-(180 / 40) + 1, -179, -(340 / 40), 33 /*ramp01*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(100 / 40) + 1, -347, -(1260 / 40), 33 /*ramp01*/, 0, -90, 0, 0, 0);
		
		declare_object_at_cell((700 / 40) + 1, -273, (660 / 40), 27 /*hiway01*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((1420 / 40) + 1, -351, (1140 / 40), 27 /*hiway01*/, 0, 0, 0, 0, 0);
		
		declare_object_at_cell(-(180 / 40) + 1, -237, -(100 / 40), 27 /*hiway01*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((540 / 40) + 2, -237, -(100 / 40), 27 /*hiway01*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(900 / 40) + 1, -237, -(100 / 40), 27 /*hiway01*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(1580 / 40) + 1, -237, (340 / 40) + 1, 27 /*hiway01*/, 0, 90, 0, 0, 0);
		
		declare_object_at_cell(-(1260 / 40) + 1, -237, -(100 / 40), 28 /*hiway02*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(540 / 40) + 1, -237, -(100 / 40), 28 /*hiway02*/, 0, 180, 0, 0, 0);
		declare_object_at_cell((180 / 40) + 2, -237, -(100 / 40), 28 /*hiway02*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((900 / 40) + 2, -237, -(100 / 40), 28 /*hiway02*/, 0, 180, 0, 0, 0);
		declare_object_at_cell((1140 / 40) + 2, -237, -(100 / 40), 28 /*hiway02*/, 0, 0, 0, 0, 0);
		
		declare_object_at_cell(-(420 / 40) + 1, -237, -(100 / 40), 29 /*hiway03*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(1380 / 40) + 1, -237, -(100 / 40), 29 /*hiway03*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((60 / 40) + 2, -237, -(100 / 40), 29 /*hiway03*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((1020 / 40) + 2, -237, -(100 / 40), 29 /*hiway03*/, 0, 0, 0, 0, 0);
		
		declare_object_at_cell(-(1020 / 40) + 1, -295, (1100 / 40), 29 /*hiway03*/, 0, 0, 0, 0, 0);
		
		declare_object_at_cell((300 / 40) + 2, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((660 / 40) + 2, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((780 / 40) + 2, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((1260 / 40) + 2, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((1460 / 40) + 2, -237, -(300 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
		declare_object_at_cell(-(60 / 40) + 1, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(300 / 40) + 1, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(660 / 40) + 1, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(780 / 40) + 1, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(1140 / 40) + 1, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(1580 / 40) + 1, -237, (100 / 40) + 1, 30 /*hiway04*/, 0, 90, 0, 0, 0);
		declare_object_at_cell(-(1580 / 40) + 1, -237, (220 / 40) + 1, 30 /*hiway04*/, 0, 90, 0, 0, 0);
		
		declare_object_at_cell(-(1020 / 40) + 1, -237, -(100 / 40), 31 /*hiway04*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((420 / 40) + 2, -237, -(100 / 40), 31 /*hiway04*/, 0, 0, 0, 0, 0);
		
		declare_object_at_cell((1420 / 40) + 2, -179, -(140 / 40), 32 /*hiway06*/, 0, 180, 0, 0, 0);
		declare_object_at_cell(-(1540 / 40) + 1, -179, -(60 / 40), 32 /*hiway06*/, 0, 0, 0, 0, 0);
		
		declare_object_at_cell((1460 / 40) + 2, -237, -(420 / 40), 34 /*hiway07*/, 0, 90, 0, 0, 0);
		declare_object_at_cell((140 / 40) + 2, -405, -(1260 / 40), 34 /*hiway07*/, 0, 180, 0, 0, 0);
		
		declare_object_at_cell(-(1140 / 40) + 1, -295, (1100 / 40), 34 /*hiway07*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(900 / 40) + 1, -295, (1100 / 40), 34 /*hiway07*/, 0, 180, 0, 0, 0);
		
		declare_object_at_cell(-(1660 / 40) + 1, -301, -(140 / 40), 37 /*sign-arrow*/, 0, -45, 0, 0, 0);
		declare_object_at_cell(-(1660 / 40) + 1, -356, (1180 / 40), 38 /*big-arrow*/, 0, 90, 0, 0, 0);
		declare_object_at_cell(-(540 / 40) + 1, -402, (1020 / 40), 37 /*sign-arrow*/, 180, -90, 0, 0, 0);
		declare_object_at_cell((1820 / 40) + 1, -365, (1140 / 40), 38 /*sign-arrow*/, 0, 135, 0, 0, 0);
		declare_object_at_cell((1380 / 40) + 1, -316, -(740 / 40), 38 /*big-arrow*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((1500 / 40) + 1, -296, -(60 / 40), 37 /*sign-arrow*/, 180, 135, 0, 0, 0);
		declare_object_at_cell(-(180 / 40) + 1, -298, (20 / 40), 37 /*sign-arrow*/, 180, 90, 0, 0, 0);
		
	} else if(levelNo == 2)
	{
		// Red Vox - There's a Place, also good action theme.
		music1 = "SOCGIRL.MUS";
		music2 = "EMINEN.MUS";
		music3 = "LONKR.MUS";
		texNames[0] = "DIR2B.TGA";
		texNames[1] = "DIR2A.TGA";
		texNames[2] = "DIR2D.TGA";
		texNames[3] = "DIR2C.TGA";
		texNames[4] = "DIR2B.TGA";
		palName = "PAL1.TGA";
		bgName = "BG2.TGA";
		
	declare_object_at_cell((0 / 40) + 1, -0, (0 / 40), 60 /*track data*/, 0, 0, 0, (20<<8), 0);
		
	declare_object_at_cell((1540 / 40) + 1, -378, -(2940 / 40), 1 /*t item*/, 0, 0, 0, 		0, 0);
	declare_object_at_cell((500 / 40) + 1, -368, -(2500 / 40), 2 /*t item*/, 0, 15, 0, 	0, 0);
	declare_object_at_cell(-(1460 / 40) + 1, -281, -(3020 / 40), 3 /*t item*/, 0, 30, 0, 	0, 0);
	declare_object_at_cell(-(340 / 40) + 1, -260, -(300 / 40), 4 /*t item*/, 0, 45, 0, 	0, 0);
	declare_object_at_cell(-(1180 / 40) + 1, -340, (1500 / 40), 5 /*t item*/, 0, 90, 0, 	0, 0);
	declare_object_at_cell((100 / 40) + 1, -358, (2180 / 40), 6 /*t item*/, 0, 105, 0, 	0, 0);
	declare_object_at_cell((1260 / 40) + 1, -318, (860 / 40), 7 /*t item*/, 0, 120, 0, 	0, 0);
	
	declare_object_at_cell(-(300 / 40) + 1, -243, (3020 / 40), 53 /*flag stand*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1540 / 40) + 1, -138, -(3500 / 40), 54 /*goal stand*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1540 / 40) + 1, -(138 + 15), -(3500 / 40), 58 /*flag goal*/, 0, 0, 0, 20, 0);
	
		declare_object_at_cell((780 / 40) + 1, -313, -(2340 / 40), 39 /*post01*/, 0, 0, 0, 0, 0);		
		declare_object_at_cell((900 / 40) + 1, -317, -(2340 / 40), 39 /*post01*/, 0, 0, 0, 0, 0);
		
		declare_object_at_cell((420 / 40) + 1, -335, -(420 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);		
		declare_object_at_cell((540 / 40) + 1, -335, -(420 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);
		
		declare_object_at_cell((1440 / 40) + 1, -181, (1580 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);		
		declare_object_at_cell((1200 / 40) + 1, -135, (1580 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);
		
		declare_object_at_cell(-(300 / 40) + 1, -220, (3220 / 40), 18 /*post00*/, 0, 0, 0, 0, 3<<4);		
		declare_object_at_cell(-(300 / 40) + 1, -220, (3460 / 40), 18 /*post00*/, 0, 0, 0, 0, 3<<4);
		
		declare_object_at_cell(-(1740 / 40) + 1, -285, (2380 / 40), 18 /*post00*/, 0, 0, 0, 0, 4<<4);		
		declare_object_at_cell(-(1500 / 40) + 1, -285, (2380 / 40), 18 /*post00*/, 0, 0, 0, 0, 4<<4);
		
		declare_object_at_cell(-(380 / 40) + 1, -243, (340 / 40), 18 /*post00*/, 0, 0, 0, 0, 5<<4);		
		declare_object_at_cell(-(740 / 40) + 1, -243, (340 / 40), 18 /*post00*/, 0, 0, 0, 0, 5<<4);
		
		declare_object_at_cell(-(660 / 40) + 1, -121, -(1380 / 40), 18 /*post00*/, 0, 0, 0, 0, 6<<4);		
		declare_object_at_cell(-(1060 / 40) + 1, -133, -(1380 / 40), 18 /*post00*/, 0, 0, 0, 0, 6<<4);
		
		declare_object_at_cell((420 / 40) + 1, -117, -(3280 / 40), 18 /*post00*/, 0, 0, 0, 0, 7<<4);		
		declare_object_at_cell((420 / 40) + 1, -117, -(3520 / 40), 18 /*post00*/, 0, 0, 0, 0, 7<<4);

		declare_object_at_cell((1540 / 40) + 1, -185, -(2060 / 40), 40 /*post02*/, 0, 0, 0, 0, 8<<4);		
		declare_object_at_cell((1780 / 40) + 1, -185, -(2060 / 40), 40 /*post02*/, 0, 0, 0, 0, 8<<4);

		declare_object_at_cell((1180 / 40) + 1, -324, -(1460 / 40), 37 /*sign-arrow*/, 180, -180, 0, 0, 0);
		declare_object_at_cell((380 / 40) + 1, -328, -(220 / 40), 37 /*sign-arrow*/, 0, 0, 0, 0, 0);
		declare_object_at_cell((1020 / 40) + 1, -189, (540 / 40), 38 /*big-arrow*/, 180, -135, 0, 0, 0);
		declare_object_at_cell((1180 / 40) + 1, -175, (2820 / 40), 38 /*big-arrow*/, 180, 135, 0, 0, 0);
		declare_object_at_cell(-(1500 / 40) + 1, -327, (3420 / 40), 38 /*big-arrow*/, 180, 45, 0, 0, 0);
		declare_object_at_cell(-(1740 / 40) + 1, -357, (2100 / 40), 38 /*big-arrow*/, 180, -45, 0, 0, 0);
		declare_object_at_cell(-(620 / 40) + 1, -331, (1100 / 40), 38 /*big-arrow*/, 0, -215, 0, 0, 0);
		declare_object_at_cell(-(340 / 40) + 1, -173, -(580 / 40), 37 /*sign-arrow*/, 0, 180, 0, 0, 0);
		declare_object_at_cell(-(340 / 40) + 1, -173, -(940 / 40), 37 /*sign-arrow*/, 0, 180, 0, 0, 0);
		declare_object_at_cell(-(1580 / 40) + 1, -157, -(2180 / 40), 38 /*big-arrow*/, 180, 0, 0, 0, 0);
		declare_object_at_cell(-(660 / 40) + 1, -202, -(3420 / 40), 38 /*big-arrow*/, 180, -90, 0, 0, 0);
		declare_object_at_cell((1580 / 40) + 1, -282, -(3500 / 40), 38 /*big-arrow*/, 180, -135, 0, 0, 0);
		
	declare_object_at_cell((60 / 40) + 1, -328, (380 / 40), 51 /*start stand*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((1660 / 40) + 1, -159, -(980 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((1060 / 40) + 1, -251, -(2780 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((1060 / 40) + 1, -344, -(1460 / 40), 11 /*bridge1*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(580 / 40) + 1, -177, -(300 / 40), 12 /*greece01*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(820 / 40) + 1, -177, -(300 / 40), 13 /*greece02*/, 0, 180, 0, 0, 0);
	declare_object_at_cell(-(340 / 40) + 1, -177, -(140 / 40), 13 /*greece02*/, 0, 90, 0, 0, 0);
	
	declare_object_at_cell(-(1580 / 40) + 1, -201, -(2860 / 40), 13 /*greece02*/, 0, 45, 0, 0, 0);
	declare_object_at_cell(-(1300 / 40) + 1, -201, -(3140 / 40), 13 /*greece02*/, 0, 45, 0, 0, 0);
	declare_object_at_cell(-(1220 / 40) + 1, -201, -(3220 / 40), 13 /*greece02*/, 0, 45, 0, 0, 0);
	declare_object_at_cell(-(940 / 40) + 1, -201, -(3500 / 40), 13 /*greece02*/, 0, 45, 0, 0, 0);
	
	declare_object_at_cell(-(340 / 40) + 1, -177, -(300 / 40), 14 /*greece03*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(1460 / 40) + 1, -200, -(3020 / 40), 15 /*greece04*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1100 / 40) + 1, -200, -(3380 / 40), 15 /*greece04*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((1660 / 40) + 1, -117, -(1820 / 40), 16 /*overhang*/, 15, 0, 0, 0, 0);
	declare_object_at_cell((1020 / 40) + 1, -146, -(220 / 40), 16 /*overhang*/, 0, 135, 0, 0, 0);
	declare_object_at_cell((60 / 40) + 1, -209, (3180 / 40), 16 /*overhang*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(580 / 40) + 1, -238, (3340 / 40), 16 /*overhang*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((1540 / 40) + 1, -316, -(2940 / 40), 16 /*overhang*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(660 / 40) + 1, -190, -(2220 / 40), 16 /*overhang*/, 0, -45, 15, 0, 0);
	declare_object_at_cell(-(420 / 40) + 1, -180, -(2580 / 40), 16 /*overhang*/, 0, -45, 15, 0, 0);
	
	declare_object_at_cell((740 / 40) + 1, -167, -(60 / 40), 17 /*pier1*/, 0, 45, 0, 0, 0);
	declare_object_at_cell((1180 / 40) + 1, -97, (2300 / 40), 17 /*pier1*/, 0, 45, 0, 0, 0);
	
	declare_object_at_cell(-(1260 / 40) + 1, -304, (1620 / 40), 19 /*tunnel2*/, 0, 125, 0, 0, 0);
	
	declare_object_at_cell((1300 / 40) + 1, -133, (860 / 40), 21 /*wall1*/, 0, 140, 0, 0, 0);
	declare_object_at_cell(-(1420 / 40) + 1, -262, (2620 / 40), 21 /*wall1*/, 0, 180, 0, 0, 0);
	declare_object_at_cell(-(340 / 40) + 1, -226, (740 / 40), 21 /*wall1*/, 0, 180, 0, 0, 0);
	declare_object_at_cell(-(980 / 40) + 1, -208, (380 / 40), 21 /*wall1*/, 0, 40, 0, 0, 0);
	declare_object_at_cell((300 / 40) + 1, -397, -(620 / 40), 21 /*wall1*/, 0, 40, 0, 0, 0);
	declare_object_at_cell((620 / 40) + 1, -369, -(2500 / 40), 21 /*wall1*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(1340 / 40) + 1, -229, (3260 / 40), 24 /*OBSTCL1*/, 0, 135, 0, 0, 0);
	
	declare_object_at_cell(-(1100 / 40) + 1, -258, -(1740 / 40), 27 /*hiway01*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1500 / 40) + 1, -234 , -(2420 / 40), 27 /*hiway01*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((1660 / 40) + 1, -146, -(1340 / 40), 29 /*hiway03*/, 0, 90, 0, 0, 0);
	declare_object_at_cell((1660 / 40) + 1, -146, -(1460 / 40), 34 /*hiway07*/, 0, 90, 0, 0, 0);
	declare_object_at_cell((1660 / 40) + 1, -146, -(1220 / 40), 34 /*hiway07*/, 0, 270, 0, 0, 0);
	
	declare_object_at_cell((100 / 40) + 1, -343, (2180 / 40), 35 /*tower01*/, 0, 0, 0, 0, 0);

	} else if(levelNo == 3)
	{
		music1 = "SHAZFUNK.MUS";
		music2 = "CHOSPITE.MUS";
		music3 = "LONKR.MUS";
		texNames[0] = "DIR3C.TGA";
		texNames[1] = "DIR3B.TGA";
		texNames[2] = "DIR3B.TGA";
		texNames[3] = "DIR3A.TGA";
		texNames[4] = "DIR3A.TGA";
		palName = "PAL2.TGA";
		bgName = "BG3.TGA";
		
	declare_object_at_cell((1060 / 40) + 1, -172, (820 / 40), 51 /*start stand*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((0 / 40) + 1, -0, (0 / 40), 60 /*track data*/, 0, 0, 0, (29<<8), 0);
	
	declare_object_at_cell((1020 / 40) + 1, -260, -(420 / 40), 1 /*t item*/, 0, 0, 0, 		0, 0);
	declare_object_at_cell((1180 / 40) + 1, -214, (1260 / 40), 2 /*t item*/, 0, 15, 0, 	0, 0);
	declare_object_at_cell((540 / 40) + 1, -333, -(1380 / 40), 3 /*t item*/, 0, 30, 0, 	0, 0);
	declare_object_at_cell(-(180 / 40) + 1, -195, (60 / 40), 4 /*t item*/, 0, 45, 0, 	0, 0);
	declare_object_at_cell(-(700 / 40) + 1, -160, (540 / 40), 5 /*t item*/, 0, 90, 0, 	0, 0);
	declare_object_at_cell(-(940 / 40) + 1, -25, -(620 / 40), 6 /*t item*/, 0, 105, 0, 	0, 0);
	declare_object_at_cell(-(700 / 40) + 1, -399, -(1220 / 40), 7 /*t item*/, 0, 120, 0, 	0, 0);
	
	declare_object_at_cell((1380 / 40) + 1, -508, (260 / 40), 53 /*flag stand*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1060 / 40) + 1, -35, (1060 / 40), 54 /*goal stand*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1060 / 40) + 1, -(35 + 15), (1060 / 40), 58 /*goal stand*/, 0, 0, 0, 20, 0);
	
	declare_object_at_cell((20 / 40) + 1, -353, -(980 / 40), 39 /*post01*/, 0, 0, 0, 0, 0);		
	declare_object_at_cell((20 / 40) + 1, -365, -(1220 / 40), 39 /*post01*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((940 / 40) + 1, -465, (100 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);		
	declare_object_at_cell((1140 / 40) + 1, -465, (100 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);
	
	declare_object_at_cell(-(60 / 40) + 1, -382, (980 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);		
	declare_object_at_cell(-(220 / 40) + 1, -382, (1140 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);
	
	declare_object_at_cell(-(1180 / 40) + 1, -266, -(900 / 40), 40 /*post02*/, 0, 0, 0, 0, 3<<4);		
	declare_object_at_cell(-(1340 / 40) + 1, -266, -(900 / 40), 40 /*post02*/, 0, 0, 0, 0, 3<<4);
	
	declare_object_at_cell((1180 / 40) + 1, -373, -(1140 / 40), 38 /*big-arrow*/, 180, 180, 0, 0, 0);
	declare_object_at_cell((1100 / 40) + 1, -473, (340 / 40), 38 /*big-arrow*/, 180, -225, 0, 0, 0);
	declare_object_at_cell(-(180 / 40) + 1, -374, (1220 / 40), 38 /*big-arrow*/, 180, 45, 0, 0, 0);
	declare_object_at_cell(-(1300 / 40) + 1, -283, (60 / 40), 38 /*big-arrow*/, 180, 0, 0, 0, 0);
	declare_object_at_cell(-(1100 / 40) + 1, -232, -(1140 / 40), 38 /*big-arrow*/, 180, -90, 0, 0, 0);
	
	declare_object_at_cell((1020 / 40) + 1, -297, -(420 / 40), 25 /*float01*/, -15, 0, 0, 0, 0);
	
	declare_object_at_cell(-(700 / 40) + 1, -350, -(1020 / 40), 26 /*float02*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(380 / 40) + 1, -325, -(1060 / 40), 10 /*platf00*/, 0, 0, -15, 0, 0);
	declare_object_at_cell((740 / 40) + 1, -369, (500 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1020 / 40) + 1, -181, (220 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((1020 / 40) + 1, -243, -(660 / 40), 10 /*platf00*/, -15, 0, 0, 0, 0);
	declare_object_at_cell((1020 / 40) + 1, -326, -(140 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((260 / 40) + 1, -348, (940 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((540 / 40) + 1, -345, -(1140 / 40), 16 /*overhang*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((500 / 40) + 1, -362, (740 / 40), 19 /*tunnel2*/, 0, -45, 0, 0, 0);
	
	declare_object_at_cell(-(1140 / 40) + 1, -50, -(100 / 40), 21 /*wall1*/, 0, 180, 0, 0, 0);
	declare_object_at_cell(-(1140 / 40) + 1, -82, -(540 / 40), 21 /*wall1*/, 0, 180, 0, 0, 0);
	
	declare_object_at_cell(-(700 / 40) + 1, -107, (540 / 40), 15 /*greece04*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(420 / 40) + 1, -107, (820 / 40), 15 /*greece04*/, 0, 90, 0, 0, 0);
	
	declare_object_at_cell(-(540 / 40) + 1, -107, (660 / 40), 13 /*greece02*/, 0, -45, 0, 0, 0);
	declare_object_at_cell(-(860 / 40) + 1, -107, (420 / 40), 13 /*greece02*/, 0, 135, 0, 0, 0);
		
	} else if(levelNo == 4)
	{
		// Straightaway Runner
		// Move it Boi - pizza tower
		music1 = "TULIPS.MUS";
		music2 = "TEMPLE.MUS";
		music3 = "LONKR.MUS";
		texNames[0] = "DIR4C.TGA";
		texNames[1] = "DIR4B.TGA";
		texNames[2] = "DIR4A.TGA";
		texNames[3] = "DIR4A.TGA";
		texNames[4] = "DIR4D.TGA";
		palName = "PAL3.TGA";
		bgName = "BG4.TGA";
	declare_object_at_cell((460 / 40) + 1, -350, -(5060 / 40), 51 /*start start*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((0 / 40) + 1, -0, (0 / 40), 60 /*track data*/, 0, 0, 0, (20<<8), 0);

	declare_object_at_cell(-(500 / 40) + 1, -145, (1100 / 40), 1 /*t item*/, 0, 0, 0,   0, 0);
	declare_object_at_cell(-(140 / 40) + 1, -299, (3700 / 40), 2 /*t item*/, 0, 15, 0, 	0, 0);
	declare_object_at_cell(-(60 / 40) + 1, -442, (3340 / 40), 3 /*t item*/, 0, 30, 0, 	0, 0);
	declare_object_at_cell((220 / 40) + 1, -140, -(860 / 40), 4 /*t item*/, 0, 45, 0, 	0, 0);
	declare_object_at_cell((100 / 40) + 1, -385, -(2260 / 40), 5 /*t item*/, 0, 90, 0, 	0, 0);
	declare_object_at_cell((20 / 40) + 1, -265, -(3100 / 40), 6 /*t item*/, 0, 105, 0, 	0, 0);
	declare_object_at_cell((260 / 40) + 1, -116, -(4340 / 40), 7 /*t item*/, 0, 120, 0, 0, 0);
	
	declare_object_at_cell(-(340 / 40) + 1, -301, -(3100 / 40), 53 /*flag stand*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((100 / 40) + 1, -153, (2940 / 40), 54 /*goal stand*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((100 / 40) + 1, -(153 + 15), (2940 / 40), 58 /*goal stand*/, 0, 0, 0, 20, 0);

	declare_object_at_cell((340 / 40) + 1, -382, -(4780 / 40), 39 /*post01*/, 0, 0, 0, 0, 0);		
	declare_object_at_cell((540 / 40) + 1, -385, -(4780 / 40), 39 /*post01*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(100 / 40) + 1, -410, -(3100 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);		
	declare_object_at_cell((140 / 40) + 1, -410, -(3100 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);
	
	declare_object_at_cell((20 / 40) + 1, -288, -(1020 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);		
	declare_object_at_cell((300 / 40) + 1, -288, -(1020 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);

	declare_object_at_cell((140 / 40) + 1, -380, (1060 / 40), 18 /*post00*/, 0, 0, 0, 0, 3<<4);		
	declare_object_at_cell((380 / 40) + 1, -380, (1060 / 40), 18 /*post00*/, 0, 0, 0, 0, 3<<4);
	
	declare_object_at_cell(-(60 / 40) + 1, -378, (2580 / 40), 18 /*post00*/, 0, 0, 0, 0, 4<<4);		
	declare_object_at_cell((140 / 40) + 1, -371, (2580 / 40), 18 /*post00*/, 0, 0, 0, 0, 4<<4);
	
	declare_object_at_cell(-(60 / 40) + 1, -391, (4940 / 40), 40 /*post02*/, 0, 0, 0, 0, 5<<4);		
	declare_object_at_cell(-(300 / 40) + 1, -391, (4940 / 40), 40 /*post02*/, 0, 0, 0, 0, 5<<4);
	
	declare_object_at_cell((100 / 40) + 1, -332, -(4460 / 40), 38 /*big-arrow*/, 0, -30, 0, 0, 0);
	declare_object_at_cell((180 / 40) + 1, -413, -(2500 / 40), 38 /*big-arrow*/, 180, -180, 0, 0, 0);
	declare_object_at_cell(-(20 / 40) + 1, -290, -(740 / 40), 38 /*big-arrow*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((420 / 40) + 1, -290, -(740 / 40), 38 /*big-arrow*/, 180, -180, 0, 0, 0);
	declare_object_at_cell((180 / 40) + 1, -385, -(20 / 40), 38 /*big-arrow*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((460 / 40) + 1, -415, (1820 / 40), 38 /*big-arrow*/, 180, 180, 0, 0, 0);
	declare_object_at_cell(-(220 / 40) + 1, -439, (2900 / 40), 38 /*big-arrow*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((140 / 40) + 1, -211,  -(1020 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((180 / 40) + 1, -197,  -(480 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((220 / 40) + 1, -351,  (1060 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((260 / 40) + 1, -270,  -(4340 / 40), 23 /*bridge2*/, 0, 30, 0, 0, 0);
	declare_object_at_cell((300 / 40) + 1, -426,  (860 / 40), 23 /*bridge2*/, 0, 30, 0, 0, 0);
	declare_object_at_cell((300 / 40) + 1, -426,  (1260 / 40), 23 /*bridge2*/, 0, -30, 0, 0, 0);
	
	declare_object_at_cell((140 / 40) + 1, -245, -(4140 / 40), 16 /*overhang*/, 0, 115, 0, 0, 0);
	declare_object_at_cell((20 / 40) + 1, -289, -(3100 / 40), 16 /*overhang*/, 0, 90, 0, 0, 0);
	
	declare_object_at_cell((260 / 40) + 1, -340, (2100 / 40), 16 /*overhang*/, 0, 90, 0, 0, 0);
	
	declare_object_at_cell((180 / 40) + 1, -201, -(1220 / 40), 16 /*overhang*/, 20, 0, 0, 0, 0);
	declare_object_at_cell((100 / 40) + 1, -201, -(1220 / 40), 16 /*overhang*/, 20, 0, 0, 0, 0);
	
	declare_object_at_cell((220 / 40) + 1, -152, -(740 / 40), 16 /*overhang*/, 14, 0, 0, 0, 0);
	declare_object_at_cell((140 / 40) + 1, -152, -(740 / 40), 16 /*overhang*/, 14, 0, 0, 0, 0);
	
	declare_object_at_cell(-(140 / 40) + 1, -381, (3340 / 40), 24 /*OBSTCL1*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((60 / 40) + 1, -381, -(2300 / 40), 24 /*OBSTCL1*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((300 / 40) + 1, -383, (220 / 40), 24 /*OBSTCL1*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((100 / 40) + 1, -389, -(2300 / 40), 9 /*KYOOB*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((260 / 40) + 1, -393, (220 / 40), 9 /*KYOOB*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(20 / 40) + 1, -394, -(1900 / 40), 13 /*greece02*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((340 / 40) + 1, -310, -(300 / 40), 13 /*greece02*/, 0, 180, 0, 0, 0);
	
	declare_object_at_cell(-(20 / 40) + 1, -398, (4420 / 40), 13 /*greece02*/, 0, 45, 0, 0, 0);
	declare_object_at_cell(-(300 / 40) + 1, -398, (4700 / 40), 13 /*greece02*/, 0, 225, 0, 0, 0);
	
	declare_object_at_cell(-(180 / 40) + 1, -398, (4540 / 40), 15 /*greece04*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(140 / 40) + 1, -296, (2940 / 40), 21 /*wall1*/, 0, 0, 60, 0, 0);
	declare_object_at_cell(-(140 / 40) + 1, -277, (3820 / 40), 21 /*wall1*/, -60, -90, 0, 0, 0);
	
	declare_object_at_cell(-(340 / 40) + 1, -329, -(3100 / 40), 22 /*float03*/, 0, 0, 0, 0, 0);
		
	} else if(levelNo == 5)
	{
		
	//Despite the level's size, this is one of the harder ones.
		music1 = "DASHY.MUS";
		music2 = "PLAYRUL.MUS";
		music3 = "LONKR.MUS";
		texNames[0] = "DIR1B.TGA";
		texNames[1] = "DIR1B.TGA";
		texNames[2] = "DIR1C.TGA";
		texNames[3] = "DIR1C.TGA";
		texNames[4] = "DIR1A.TGA";
		palName = "PAL0.TGA";
		bgName = "BG1.TGA";
	declare_object_at_cell(-(280 / 40) + 1, -97, -(20 / 40), 51 /*start start*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((0 / 40) + 1, -0, (0 / 40), 60 /*track data*/, 0, 0, 0, (20<<8), 0);

	declare_object_at_cell(-(1820 / 40) + 1, -360, (1900 / 40), 53 /*flag stand*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(60 / 40) + 1, -467, (180 / 40), 54 /*goal stand*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(60 / 40) + 1, -(467 + 15), (180 / 40), 58 /*flag goal*/, 0, 0, 0, 20, 0);
	
	declare_object_at_cell(-(100 / 40) + 1, -119, (580 / 40), 1 /*t item*/, 0, 0, 0,   0, 0);
	declare_object_at_cell((380 / 40) + 1, -401, -(20 / 40), 2 /*t item*/, 0, 15, 0, 	0, 0);
	declare_object_at_cell((820 / 40) + 1, -147, -(740 / 40), 3 /*t item*/, 0, 30, 0, 	0, 0);
	declare_object_at_cell(-(1540 / 40) + 1, -205, -(1460 / 40), 4 /*t item*/, 0, 45, 0, 	0, 0);
	declare_object_at_cell(-(940 / 40) + 1, -428, -(260 / 40), 5 /*t item*/, 0, 90, 0, 	0, 0);
	declare_object_at_cell(-(860 / 40) + 1, -245, (100 / 40), 6 /*t item*/, 0, 105, 0, 	0, 0);
	declare_object_at_cell((1380 / 40) + 1, -301, (1380 / 40), 7 /*t item*/, 0, 120, 0, 0, 0);
	
	declare_object_at_cell((1340 / 40) + 1, -156, (1260 / 40), 39 /*post00*/, 0, 0, 0, 0, 0);		
	declare_object_at_cell((1260 / 40) + 1, -156, (1340 / 40), 39 /*post00*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(540 / 40) + 1, -126, -(860 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);		
	declare_object_at_cell(-(540 / 40) + 1, -85, -(1220 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);
	
	declare_object_at_cell(-(1180 / 40) + 1, -96, -(140 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);		
	declare_object_at_cell(-(1420 / 40) + 1, -125, -(140 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);
	
	declare_object_at_cell((20 / 40) + 1, -280, (1260 / 40), 18 /*post00*/, 0, 0, 0, 0, 3<<4);		
	declare_object_at_cell((20 / 40) + 1, -284, (1540 / 40), 18 /*post00*/, 0, 0, 0, 0, 3<<4);
	
	declare_object_at_cell((1220 / 40) + 1, -304, -(60 / 40), 18 /*post00*/, 0, 0, 0, 0, 4<<4);		
	declare_object_at_cell((1420 / 40) + 1, -316, -(60 / 40), 18 /*post00*/, 0, 0, 0, 0, 4<<4);
	
	declare_object_at_cell((20 / 40) + 1, -288, -(1220 / 40), 18 /*post00*/, 0, 0, 0, 0, 5<<4);		
	declare_object_at_cell((20 / 40) + 1, -314, -(1540 / 40), 18 /*post00*/, 0, 0, 0, 0, 5<<4);
	
	declare_object_at_cell(-(700 / 40) + 1, -374, (500 / 40), 40 /*post00*/, 0, 0, 0, 0, 6<<4);		
	declare_object_at_cell(-(700 / 40) + 1, -374, (620 / 40), 40 /*post00*/, 0, 0, 0, 0, 6<<4);
	
	declare_object_at_cell((1580 / 40) + 1, -235, (1420 / 40), 37 /*sign-arrow*/, 0, -45, 0, 0, 0);
	declare_object_at_cell((1420 / 40) + 1, -235, (1580 / 40), 37 /*sign-arrow*/, 180, -45, 0, 0, 0);
	declare_object_at_cell((500 / 40) + 1, -240, (1140 / 40), 37 /*sign-arrow*/, 0, 135, 0, 0, 0);
	declare_object_at_cell((1260 / 40) + 1, -277, -(460 / 40), 37 /*sign-arrow*/, 0, -135, 0, 0, 0);
	declare_object_at_cell((500 / 40) + 1, -190, -(1060 / 40), 37 /*sign-arrow*/, 0, 45, 90, 0, 0);
	declare_object_at_cell(-(220 / 40) + 1, -278, -(1460 / 40), 37 /*sign-arrow*/, 0, -45, 0, 0, 0);
	
	declare_object_at_cell((500 / 40) + 1, -144, (460 / 40), 38 /*big-arrow*/, 180, 0, 0, 0, 0);
	declare_object_at_cell((820 / 40) + 1, -116, -(340 / 40), 38 /*big-arrow*/, 0, -135, 0, 0, 0);
	declare_object_at_cell((60 / 40) + 1, -109, -(980 / 40), 38 /*big-arrow*/, 0, -90, 0, 0, 0);
	declare_object_at_cell(-(860 / 40) + 1, -74, -(1260 / 40), 38 /*big-arrow*/, 0, -90, 0, 0, 0);
	declare_object_at_cell(-(1460 / 40) + 1, -135, -(940 / 40), 38 /*big-arrow*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1420 / 40) + 1, -151, (540 / 40), 38 /*big-arrow*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1100 / 40) + 1, -169, (1300 / 40), 38 /*big-arrow*/, 0, 45, 0, 0, 0);
	declare_object_at_cell(-(660 / 40) + 1, -169, (1060 / 40), 38 /*big-arrow*/, 180, -135, 0, 0, 0);
	declare_object_at_cell(-(660 / 40) + 1, -169, (1460 / 40), 38 /*big-arrow*/, 0, 90, 0, 0, 0);
	
	declare_object_at_cell((740 / 40) + 1, -148, (740 / 40), 16 /*overhang*/, 0, 45, 0, 0, 0);
	declare_object_at_cell(-(1180 / 40) + 1, -126, (1020 / 40), 16 /*overhang*/, 0, 45, 0, 0, 0);
	declare_object_at_cell((1740 / 40) + 1, -399, (1780 / 40), 16 /*overhang*/, 0, 45, 0, 0, 0);
	
	declare_object_at_cell(-(1060 / 40) + 1, -336, -(260 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(500 / 40) + 1, -334,  (580 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(340 / 40) + 1, -346,  (380 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((1020 / 40) + 1, -196,  (460 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((860 / 40) + 1, -22, -(700 / 40), 21 /*wall1*/, 0, -135, 0, 0, 0);
	declare_object_at_cell((340 / 40) + 1, -181, -(1340 / 40), 21 /*wall1*/, 75, 90, 0, 0, 0);
	
	declare_object_at_cell((1380 / 40) + 1, -284, (1380 / 40), 23 /*bridge2*/, 0, -135, 0, 0, 0);
	declare_object_at_cell((660 / 40) + 1, -173, -(900 / 40), 23 /*bridge2*/, 0, -45, 0, 0, 0);
	
	declare_object_at_cell(-(420 / 40) + 1, -164, -(1020 / 40), 17 /*pier1*/, 0, 120, 0, 0, 0);
	
	declare_object_at_cell(-(860 / 40) + 1, -310, -(100 / 40), 20 /*tunnl3*/, 0, 90, 0, 0, 0);
	
	declare_object_at_cell((300 / 40) + 1, -321, -(20 / 40), 14 /*greece03*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(60 / 40) + 1, -370, (180 / 40), 25 /*float01*/, 0, 45, 0, 0, 0);

	} else if(levelNo == 6)
	{
		
// "This is your consolation, always a next year" - E40 by SGAP - GREAT sound to play if people don't make par time.
// If I forget, path is:
// South -> North -> West -> East -> South (Ramp) -> West (Tunnel) -> North (Hill) -> East (NE Goal Corner)
// This level's gonna be HARD.
		music1 = "SOCGIRL.MUS";
		music2 = "EMINEN.MUS";
		music3 = "LONKR.MUS";
		texNames[0] = "DIR2B.TGA";
		texNames[1] = "DIR2A.TGA";
		texNames[2] = "DIR2D.TGA";
		texNames[3] = "DIR2C.TGA";
		texNames[4] = "DIR2B.TGA";
		palName = "PAL1.TGA";
		bgName = "BG2.TGA";
	declare_object_at_cell(-(380 / 40) + 1, -276, -(1620 / 40), 51 /*start start*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((0 / 40) + 1, -0, (0 / 40), 60 /*track data*/, 0, 0, 0, (20<<8), 0);
	
	declare_object_at_cell(-(100 / 40) + 1, -278, -(60 / 40), 53 /*flag stand*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1500 / 40) + 1, -59, (1540 / 40), 54 /*goal stand*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1500 / 40) + 1, -(59 + 15), (1540 / 40), 58 /*flag goal*/, 0, 0, 0, 20, 0);
	
	declare_object_at_cell((500 / 40) + 1, -212, (980 / 40), 1 /*t item*/, 0, 0, 0,   0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -281, (820 / 40), 2 /*t item*/, 0, 15, 0, 	0, 0);
	declare_object_at_cell((1540 / 40) + 1, -229, (460 / 40), 3 /*t item*/, 0, 30, 0, 	0, 0);
	declare_object_at_cell(-(140 / 40) + 1, -124, -(940 / 40), 4 /*t item*/, 0, 45, 0, 	0, 0);
	declare_object_at_cell(-(1660 / 40) + 1, -249, -(260 / 40), 5 /*t item*/, 0, 90, 0, 	0, 0);
	declare_object_at_cell(-(1340 / 40) + 1, -389, (900 / 40), 6 /*t item*/, 0, 105, 0, 	0, 0);
	declare_object_at_cell((1500 / 40) + 1, -394, -(660 / 40), 7 /*t item*/, 0, 120, 0, 0, 0);
	
	declare_object_at_cell(-(20 / 40) + 1, -313, -(1540 / 40), 39 /*post00*/, 0, 0, 0, 0, 0);		
	declare_object_at_cell(-(180 / 40) + 1, -313, -(1540 / 40), 39 /*post00*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(20 / 40) + 1, -301, (1460 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);		
	declare_object_at_cell(-(180 / 40) + 1, -301, (1460 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);
	
	declare_object_at_cell(-(1580 / 40) + 1, -304, (20 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);		
	declare_object_at_cell(-(1580 / 40) + 1, -304, -(140 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);
	
	declare_object_at_cell((1340 / 40) + 1, -308, (20 / 40), 18 /*post00*/, 0, 0, 0, 0, 3<<4);		
	declare_object_at_cell((1340 / 40) + 1, -308, -(140 / 40), 18 /*post00*/, 0, 0, 0, 0, 3<<4);
	
	declare_object_at_cell((780 / 40) + 1, -175, -(60 / 40), 18 /*post00*/, 0, 0, 0, 0, 4<<4);		
	declare_object_at_cell((1060 / 40) + 1, -175, -(60 / 40), 18 /*post00*/, 0, 0, 0, 0, 4<<4);
	
	declare_object_at_cell(-(100 / 40) + 1, -154, -(1020 / 40), 18 /*post00*/, 0, 0, 0, 0, 5<<4);		
	declare_object_at_cell(-(100 / 40) + 1, -154, -(1220 / 40), 18 /*post00*/, 0, 0, 0, 0, 5<<4);
	
	declare_object_at_cell(-(1340 / 40) + 1, -210, -(1020 / 40), 18 /*post00*/, 0, 0, 0, 0, 6<<4);		
	declare_object_at_cell(-(1340 / 40) + 1, -210, -(1180 / 40), 18 /*post00*/, 0, 0, 0, 0, 6<<4);
	
	declare_object_at_cell(-(100 / 40) + 1, -175, (860 / 40), 18 /*post00*/, 0, 0, 0, 0, 7<<4);		
	declare_object_at_cell(-(100 / 40) + 1, -175, (1140 / 40), 18 /*post00*/, 0, 0, 0, 0, 7<<4);
	
	declare_object_at_cell((1140 / 40) + 1, -183, (1340 / 40), 40 /*post00*/, 0, 0, 0, 0, 8<<4);		
	declare_object_at_cell((1140 / 40) + 1, -194, (1580 / 40), 40 /*post00*/, 0, 0, 0, 0, 8<<4);
	
	declare_object_at_cell(-(100 / 40) + 1, -370, -(460 / 40), 37 /*sign-arrow*/, 0, 0, -90, 0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -370, (340 / 40), 37 /*sign-arrow*/, 0, 0, -90, 0, 0);
	declare_object_at_cell(-(500 / 40) + 1, -370, -(60 / 40), 37 /*sign-arrow*/, 0, 90, -90, 0, 0);
	declare_object_at_cell((260 / 40) + 1,  -370, -(60 / 40), 37 /*sign-arrow*/, 0, 90, -90, 0, 0);
	declare_object_at_cell(-(1540 / 40) + 1, -185, -(1020 / 40), 37 /*sign-arrow*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1540 / 40) + 1, -185, -(1140 / 40), 37 /*sign-arrow*/, 180, 0, 0, 0, 0);
	
	declare_object_at_cell(-(180 / 40) + 1, -300, (1660 / 40), 38 /*big-arrow*/, 180, 90, 0, 0, 0);
	declare_object_at_cell(-(780 / 40) + 1, -300, (1620 / 40), 38 /*big-arrow*/, 180, 45, 0, 0, 0);
	declare_object_at_cell(-(1660 / 40) + 1, -287, -(180 / 40), 38 /*big-arrow*/, 180, -90, 0, 0, 0);
	declare_object_at_cell((1540 / 40) + 1, -257, (380 / 40), 38 /*big-arrow*/, 180, 60, 0, 0, 0);
	declare_object_at_cell((620 / 40) + 1, -176, -(740 / 40), 38 /*big-arrow*/, 0, -130, -90, 0, 0);
	declare_object_at_cell(-(500 / 40) + 1, -160, -(980 / 40), 38 /*big-arrow*/, 180, 90, 0, 0, 0);
	declare_object_at_cell(-(660 / 40) + 1, -160, -(1420 / 40), 38 /*big-arrow*/, 180, 180, 0, 0, 0);
	declare_object_at_cell(-(660 / 40) + 1, -176, (620 / 40), 38 /*big-arrow*/, 0, 45, -90, 0, 0);

	declare_object_at_cell(-(820 / 40) + 1, -128, -(1060 / 40), 10 /*platf00*/, -15, 45, -15, 0, 0);

	declare_object_at_cell(-(1120 / 40) + 1, -160, -(1080 / 40), 20 /*tunnel3*/, 0, 0, 0, 0, 0);

	declare_object_at_cell((500 / 40) + 1, -148, (980 / 40), 16 /*overhang*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(1340 / 40) + 1, -359, (900 / 40), 25 /*float01*/, 0, 45, 0, 0, 0);
	
	declare_object_at_cell(-(100 / 40) + 1, -247, -(460 / 40), 34 /*hiway07*/, 0, 270, 0, 0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -247, -(1420 / 40), 34 /*hiway07*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -247, (340 / 40), 34 /*hiway07*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -247, (1300 / 40), 34 /*hiway07*/, 0, 270, 0, 0, 0);
	declare_object_at_cell((260 / 40) + 1, -247, -(60 / 40), 34 /*hiway07*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((1220 / 40) + 1, -247, -(60 / 40), 34 /*hiway07*/, 0, 180, 0, 0, 0);
	declare_object_at_cell(-(500 / 40) + 1, -247, -(60 / 40), 34 /*hiway07*/, 0, 180, 0, 0, 0);
	declare_object_at_cell(-(1460 / 40) + 1, -247, -(60 / 40), 34 /*hiway07*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((500 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((620 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((860 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((980 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(740 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(860 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1100 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1220 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(100 / 40) + 1, -247, -(700 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -247, -(820 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -247, -(1060 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -247, -(1180 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
	
	declare_object_at_cell(-(100 / 40) + 1, -247, (580 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -247, (700 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -247, (940 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -247, (1060 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
	
	declare_object_at_cell(-(100 / 40) + 1, -247, -(580 / 40), 29 /*hiway03*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -247, -(1300 / 40), 29 /*hiway03*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -247, (460 / 40), 29 /*hiway03*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -247, (1180 / 40), 29 /*hiway03*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(620 / 40) + 1, -247, -(60 / 40), 29 /*hiway03*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1340 / 40) + 1, -247, -(60 / 40), 29 /*hiway03*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((380 / 40) + 1, -247, -(60 / 40), 29 /*hiway03*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((1100 / 40) + 1, -247, -(60 / 40), 29 /*hiway03*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((740 / 40) + 1, -247, -(60 / 40), 27 /*hiway01*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(980 / 40) + 1, -247, -(60 / 40), 27 /*hiway01*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -247, -(940 / 40), 27 /*hiway01*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(100 / 40) + 1, -247, (820 / 40), 27 /*hiway01*/, 0, 90, 0, 0, 0);
		
	} else if(levelNo == 7)
	{
		// MGMT Siberian Breaks ~7:40 "i hope i die before i get sold" nice
		music1 = "SHAZFUNK.MUS";
		music2 = "CHOSPITE.MUS";
		music3 = "LONKR.MUS";
		texNames[0] = "DIR3C.TGA";
		texNames[1] = "DIR3B.TGA";
		texNames[2] = "DIR3B.TGA";
		texNames[3] = "DIR3A.TGA";
		texNames[4] = "DIR3A.TGA";
		palName = "PAL2.TGA";
		bgName = "BG3.TGA";
	declare_object_at_cell((20 / 40) + 1, -215, -(20 / 40), 51 /*start start*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((0 / 40) + 1, -0, (0 / 40), 60 /*track data*/, 0, 0, 0, (20<<8), 0);
	
	declare_object_at_cell(-(1340 / 40) + 1, -458, (2260 / 40), 53 /*flag stand*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((2460 / 40) + 1, -102, -(1100 / 40), 54 /*goal stand*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((2460 / 40) + 1, -(102 + 15), -(1100 / 40), 58 /*flag goal*/, 0, 0, 0, 20, 0);
	
	declare_object_at_cell(-(300 / 40) + 1, -369, (1860 / 40), 1 /*t item*/, 0, 0, 0,   0, 0);
	declare_object_at_cell(-(20 / 40) + 1, -545, (20 / 40), 2 /*t item*/, 0, 15, 0, 	0, 0);
	declare_object_at_cell((20 / 40) + 1, -252, -(2180 / 40), 3 /*t item*/, 0, 30, 0, 	0, 0);
	declare_object_at_cell((1540 / 40) + 1, -162, -(1260 / 40), 4 /*t item*/, 0, 45, 0, 	0, 0);
	declare_object_at_cell((1260 / 40) + 1, -215, (1420 / 40), 5 /*t item*/, 0, 90, 0, 	0, 0);
	declare_object_at_cell(-(1500 / 40) + 1, -391, (2820 / 40), 6 /*t item*/, 0, 105, 0, 	0, 0);
	declare_object_at_cell(-(1700 / 40) + 1, -453, (20 / 40), 7 /*t item*/, 0, 120, 0, 0, 0);
	
	declare_object_at_cell((300 / 40) + 1, -278, -(2260 / 40), 39 /*post00*/, 0, 0, 0, 0, 0);		
	declare_object_at_cell((300 / 40) + 1, -278, -(2100 / 40), 39 /*post00*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((2580 / 40) + 1, -179, -(2300 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);		
	declare_object_at_cell((2260 / 40) + 1, -179, -(2300 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);
	
	declare_object_at_cell((940 / 40) + 1, -336, (20 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);		
	declare_object_at_cell((1140 / 40) + 1, -336, (20 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);
	
	declare_object_at_cell((100 / 40) + 1, -242, (2140 / 40), 18 /*post00*/, 0, 0, 0, 0, 3<<4);		
	declare_object_at_cell((100 / 40) + 1, -242, (2460 / 40), 18 /*post00*/, 0, 0, 0, 0, 3<<4);
	
	declare_object_at_cell(-(2300 / 40) + 1, -182, (1700 / 40), 18 /*post00*/, 0, 0, 0, 0, 4<<4);		
	declare_object_at_cell(-(2540 / 40) + 1, -182, (1700 / 40), 18 /*post00*/, 0, 0, 0, 0, 4<<4);
	
	declare_object_at_cell(-(1220 / 40) + 1, -390, -(220 / 40), 18 /*post00*/, 0, 0, 0, 0, 5<<4);		
	declare_object_at_cell(-(940 / 40) + 1, -390, -(220 / 40), 18 /*post00*/, 0, 0, 0, 0, 5<<4);
	
	declare_object_at_cell(-(20 / 40) + 1, -150, -(2300 / 40), 40 /*post00*/, 0, 0, 0, 0, 6<<4);		
	declare_object_at_cell(-(20 / 40) + 1, -150, -(2060 / 40), 40 /*post00*/, 0, 0, 0, 0, 6<<4);

	declare_object_at_cell((2300 / 40) + 1, -165, -(2620 / 40), 37 /*sign-arrow*/, 180, -135, 0, 0, 0);
	declare_object_at_cell((2460 / 40) + 1, -199, -(2020 / 40), 37 /*sign-arrow*/, 180, 135, 0, 0, 0);
	declare_object_at_cell((1260 / 40) + 1, -99, (1420 / 40), 37 /*sign-arrow*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((1060 / 40) + 1, -103, (1780 / 40), 37 /*sign-arrow*/, 0, -45, 0, 0, 0);
	declare_object_at_cell(-(2340 / 40) + 1, -197, (2260 / 40), 37 /*sign-arrow*/, 180, 0, 0, 0, 0);
	declare_object_at_cell(-(1300 / 40) + 1, -97, -(1260 / 40), 37 /*sign-arrow*/, 0, -180, 0, 0, 0);
	declare_object_at_cell(-(1060 / 40) + 1, -115, -(1780 / 40), 37 /*sign-arrow*/, 0, -225, 0, 0, 0);

	declare_object_at_cell((860 / 40) + 1, -269, -(740 / 40), 38 /*big-arrow*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((1020 / 40) + 1, -292, (700 / 40), 38 /*big-arrow*/, 0, 45, 0, 0, 0);
	declare_object_at_cell(-(500 / 40) + 1, -420, -(1780 / 40), 38 /*big-arrow*/, 180, -45, 0, 0, 0);
	declare_object_at_cell((1020 / 40) + 1, -144, -(2140 / 40), 38 /*big-arrow*/, 0, 135, 0, 0, 0);
	declare_object_at_cell((180 / 40) + 1, -238, (2140 / 40), 38 /*big-arrow*/, 0, -90, 0, 0, 0);
	declare_object_at_cell((180 / 40) + 1, -238, (2460 / 40), 38 /*big-arrow*/, 180, 90, 0, 0, 0);
	declare_object_at_cell(-(2100 / 40) + 1, -178, (860 / 40), 38 /*big-arrow*/, 180, -45, 0, 0, 0);
	declare_object_at_cell(-(980 / 40) + 1, -234, (820 / 40), 38 /*big-arrow*/, 0, 180, 0, 0, 0);
	declare_object_at_cell(-(1060 / 40) + 1, -319, -(460 / 40), 38 /*big-arrow*/, 0, -135, 0, 0, 0);
	
	declare_object_at_cell((100 / 40) + 1, -405,  -(100 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(20 / 40) + 1, -405,  -(20 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(700 / 40) + 1, -357,  (1020 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(580 / 40) + 1, -347,  (1300 / 40), 10 /*platf00*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((300 / 40) + 1, -219,  -(2180 / 40), 10 /*platf00*/, 0, 45, 0, 0, 0);
	
	declare_object_at_cell(-(460 / 40) + 1, -273, -(1220 / 40), 11 /*bridge1*/, 0, 90, 0, 0, 0);
	
	declare_object_at_cell(-(980 / 40) + 1, -242, (980 / 40), 13 /*greece02*/, 0, 225, 0, 0, 0);
	declare_object_at_cell(-(1540 / 40) + 1, -342, (300 / 40), 13 /*greece02*/, 0, 45, 0, 0, 0);
	declare_object_at_cell(-(980 / 40) + 1, -242, (980 / 40), 13 /*greece02*/, 0, 225, 0, 0, 0);
	declare_object_at_cell((1260 / 40) + 1, -127, (1420 / 40), 13 /*greece02*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((1060 / 40) + 1, -131, (1780 / 40), 13 /*greece02*/, 0, 45, 0, 0, 0);
	
	declare_object_at_cell((900 / 40) + 1, -170, -(2420 / 40), 15 /*greece04*/, 0, 90, 0, 0, 0);
	declare_object_at_cell((1140 / 40) + 1, -170, -(2180 / 40), 15 /*greece04*/, 0, 90, 0, 0, 0);
	declare_object_at_cell((2420 / 40) + 1, -170, -(2580 / 40), 15 /*greece04*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((2180 / 40) + 1, -170, -(2340 / 40), 15 /*greece04*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((180 / 40) + 1, -465, (20 / 40), 16 /*overhang*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(140 / 40) + 1, -465, -(100 / 40), 16 /*overhang*/, 0, 90, 0, 0, 0);
	declare_object_at_cell((1020 / 40) + 1, -277, -(540 / 40), 16 /*overhang*/, 0, 90, 0, 0, 0);
	
	declare_object_at_cell((500 / 40) + 1, -245, (2300 / 40), 16 /*overhang*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(300 / 40) + 1, -245, (2300 / 40), 16 /*overhang*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((1020 / 40) + 1, -254, (900 / 40), 17 /*pier1*/, 0, 270, 0, 0, 0);
	
	declare_object_at_cell(-(1260 / 40) + 1, -47, -(1260 / 40), 21 /*wall1*/, 0, 180, 0, 0, 0);
	declare_object_at_cell(-(1020 / 40) + 1, -63, -(1740 / 40), 21 /*wall1*/, 0, 225, 0, 0, 0);
	
	declare_object_at_cell((1540 / 40) + 1, -100, -(1260 / 40), 24 /*OBSTCL1*/, 0, 45, 0, 0, 0);
	
	declare_object_at_cell(-(980 / 40) + 1, -352, -(540 / 40), 25 /*float01*/, 0, 45, 0, 0, 0);
	declare_object_at_cell((540 / 40) + 1, -359, -(1220 / 40), 25 /*float01*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(420 / 40) + 1, -318, (1580 / 40), 27 /*hiway01*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((20 / 40) + 1, -221, -(2180 / 40), 29 /*hiway03*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell(-(1340 / 40) + 1, -434, (2620 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(1340 / 40) + 1, -434, (2500 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(1340 / 40) + 1, -434, (2380 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(1340 / 40) + 1, -434, (2260 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(1340 / 40) + 1, -434, (2140 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(1340 / 40) + 1, -434, (2020 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
	declare_object_at_cell(-(1340 / 40) + 1, -434, (1900 / 40), 30 /*hiway04*/, 0, 90, 0, 0, 0);
	
	declare_object_at_cell(-(1380 / 40) + 1, -377, (2780 / 40), 32 /*hiway06*/, 0, 180, 0, 0, 0);
	
	declare_object_at_cell(-(1340 / 40) + 1, -377, (1660 / 40), 33 /*ramp01*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(1660 / 40) + 1, -377, (2820 / 40), 33 /*ramp01*/, 0, 270, 0, 0, 0);
	
	declare_object_at_cell(-(140 / 40) + 1, -221, -(2180 / 40), 34 /*hiway07*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((140 / 40) + 1, -221, -(2180 / 40), 34 /*hiway07*/, 0, 180, 0, 0, 0);
	
	declare_object_at_cell(-(2340 / 40) + 1, -270, -(2420 / 40), 35 /*tower01*/, 0, 0, 0, 0, 0);

	} else if(levelNo == 8)
	{
	//experimental...
	//Mario? Extreme Skii?
		music1 = "TULIPS.MUS";
		music2 = "TEMPLE.MUS";
		music3 = "LONKR.MUS";
		texNames[0] = "DIR4C.TGA";
		texNames[1] = "DIR4B.TGA";
		texNames[2] = "DIR4A.TGA";
		texNames[3] = "DIR4A.TGA";
		texNames[4] = "DIR4D.TGA";
		palName = "PAL0.TGA";
		bgName = "BG4.TGA";
	// declare_object_at_cell((100 / 40) + 1, -120, (340 / 40), 51 /*start start*/, 0, 0, 0, 0, 0);
	
	// declare_object_at_cell(-(0 / 40) + 1, -70, -(160 / 40), 41 /*m-mapA*/, 0, 0, 0, 0, 0);
	// declare_object_at_cell(-(0 / 40) + 1, -70, -(160 / 40), 42 /*m-mapB*/, 0, 0, 0, 0, 0);
	
	declare_object_at_cell((0 / 40) + 1, -2000, (0 / 40), 44 /*Tuti*/,  0, 0, 0, 0, 0);
	
	declare_object_at_cell((960 / 40) + 1, -(83 + 2000), -(360 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((960 / 40) + 1, -(83 + 2000), -(360 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 22, 0);
	
	declare_object_at_cell((960 / 40) + 1, -(107 + 2000), -(120 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((960 / 40) + 1, -(107 + 2000), -(120 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 23, 0);
	
	declare_object_at_cell((960 / 40) + 1, -(123 + 2000), (160 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((960 / 40) + 1, -(123 + 2000), (160 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 24, 0);
	
	declare_object_at_cell((920 / 40) + 1, -(123 + 2000), (320 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((920 / 40) + 1, -(123 + 2000), (320 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 25, 0);
	
	declare_object_at_cell((920 / 40) + 1, -(203 + 2000), (480 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((920 / 40) + 1, -(203 + 2000), (480 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 26, 0);
	
	declare_object_at_cell((920 / 40) + 1, -(264 + 2000), (200 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((920 / 40) + 1, -(264 + 2000), (200 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 27, 0);
	
	declare_object_at_cell((960 / 40) + 1, -(279 + 2000), -(120 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
	declare_object_at_cell((960 / 40) + 1, -(279 + 2000), -(120 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 28, 0);
	
	declare_object_at_cell(-(920 / 40) + 1, -(2000 - 295), -(120 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(920 / 40) + 1, -(2000 - 295), -(120 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 29, 0);
	
	declare_object_at_cell(-(920 / 40) + 1, -(2000 - 295), -(0 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(920 / 40) + 1, -(2000 - 295), -(0 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 30, 0);
	
	declare_object_at_cell(-(920 / 40) + 1, -(2000 - 295), (120 / 40), 36 /*sign*/, 0, 0, 0, 0, 0);
	declare_object_at_cell(-(920 / 40) + 1, -(2000 - 295), (120 / 40), 49 /*sign0 trigger*/, 0, 0, 0, 31, 0);
	
	declare_object_at_cell((880 / 40) + 1, -(240 + 2000), (480 / 40), 37 /*sign-arrow*/, 0, 180, 0, 0, 0);
	declare_object_at_cell((960 / 40) + 1, -(306 + 2000), -(240 / 40), 37 /*sign-arrow*/, 0, -90, 0, 0, 0);
	declare_object_at_cell(-(960 / 40) + 1, -(2000 - 280), -(280 / 40), 37 /*sign-arrow*/, 0, 0, 0, 0, 0);

	//declare_object_at_cell((680 / 40) + 1, -2880, (0 / 40), 51 /*start start*/, 0, 0, 0, 0, 0);
	//declare_object_at_cell((120 / 40) + 1, -2680, (0 / 40), 43 /*longbox*/,   0, 0, -10, 0, 0);
	//declare_object_at_cell(-(840 / 40) + 1, -2480, (0 / 40), 43 /*longbox*/,  0, 0, -15, 0, 0);
	//declare_object_at_cell(-(1800 / 40) + 1, -2160, (0 / 40), 43 /*longbox*/, 0, 0, -20, 0, 0);
	//declare_object_at_cell(-(2720 / 40) + 1, -1800, (0 / 40), 43 /*longbox*/, 0, 0, -20, 0, 0);
	//declare_object_at_cell(-(3640 / 40) + 1, -1480, (0 / 40), 43 /*longbox*/, 0, 0, -20, 0, 0);
	//
	//declare_object_at_cell(-(4560 / 40) + 1, -1160, (0 / 40), 43 /*longbox*/, 0, 0, -20, 0, 0);
	//declare_object_at_cell(-(5440 / 40) + 1, -840, (0 / 40), 43 /*longbox*/, 0, 0, -20, 0, 0);
	//
	//declare_object_at_cell(-(5720 / 40) + 1, -640, (0 / 40), 43 /*longbox*/, 0, 0, 0, 0, 0);
	//declare_object_at_cell(-(6240 / 40) + 1, -640, (0 / 40), 43 /*longbox*/, 0, 0, 30, 0, 0);

	// declare_object_at_cell(-(3540 / 40) + 1, -193, (3900 / 40), 53 /*flag stand*/, 0, 0, 0, 0, 0);
	// declare_object_at_cell((2780 / 40) + 1, -164, -(1660 / 40), 54 /*goal stand*/, 0, 0, 0, 0, 0);
	
	// declare_object_at_cell((3860 / 40) + 1, -401, (3900 / 40), 1 /*t item*/, 0, 0, 0,   0, 0);
	// declare_object_at_cell((1980 / 40) + 1, -295, (1340 / 40), 2 /*t item*/, 0, 15, 0, 	0, 0);
	// declare_object_at_cell(-(100 / 40) + 1, -363, (620 / 40), 3 /*t item*/, 0, 30, 0, 	0, 0);
	// declare_object_at_cell((3700 / 40) + 1, -131, -(60 / 40), 4 /*t item*/, 0, 45, 0, 	0, 0);
	// declare_object_at_cell(-(3180 / 40) + 1, -370, -(1260 / 40), 5 /*t item*/, 0, 90, 0, 	0, 0);
	// declare_object_at_cell(-(2140 / 40) + 1, -382, (3140 / 40), 6 /*t item*/, 0, 105, 0, 	0, 0);
	// declare_object_at_cell(-(3220 / 40) + 1, -218, -(3700 / 40), 7 /*t item*/, 0, 120, 0, 0, 0);

		
	}
	
}


int		main(void)
{
	
	dptr = &objects_data_list[0];
	
	cout << "Please, tell me the level # you want to generate. \n";
	int levelNo;
	string fname;
	cin >> levelNo;
	
	fname = std::to_string(levelNo);
	if(fname.length() == 1)
	{
		fname = "0" + fname;
	}
	
	fname = "LEVEL" + fname + ".LDS";
	
	create_objects(levelNo);
	
	ofstream file;
	file.open(fname, ios::out | ios::binary);
	
	int name1len = 12 - music1.length();
	int name2len = 12 - music2.length();
	int name3len = 12 - music3.length();
	
	file << "MUSIC!";
	
	file << music1;
	if(name1len > 0)
	{
		for(int i = 0; i < name1len; i++)
		{
			file << " ";
		}
	}
	file << music2;
	if(name2len > 0)
	{
		for(int i = 0; i < name2len; i++)
		{
			file << " ";
		}
	}
	file << music3;
	if(name3len > 0)
	{
		for(int i = 0; i < name3len; i++)
		{
			file << " ";
		}
	}
	
	file << "MAPTEX";
	
	int namelen;
	for(int i = 0; i < 5; i++)
	{
		namelen = 12 - texNames[i].length();
		file << texNames[i];
		if(namelen > 0)
		{
			for(int j = 0; j < namelen; j++)
			{
				file << " ";
			}
		}
	}
	
	file << "PALTEX";
	
		namelen = 12 - palName.length();
		file << palName;
		if(namelen > 0)
		{
			for(int j = 0; j < namelen; j++)
			{
				file << " ";
			}
		}
		
	file << "BACKTX";
	
		namelen = 12 - bgName.length();
		file << bgName;
		if(namelen > 0)
		{
			for(int j = 0; j < namelen; j++)
			{
				file << " ";
			}
		}
	
	file << "OBJECTS!";
	dptr = &objects_data_list[0];
	writeUint16(&file, obj_count);
	cout << adder << "\n"; 
	for(int i = 0; i < adder; i++)
	{
		writeUint16(&file, objects_data_list[i]);
	}
	
	cout << "Success, hopefully!\n";
	int hold;
	cin >> hold;
	
	return 1;
}

