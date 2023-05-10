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
	
	switch(levelNo)
	{
		case(0):
		music1 = "TRSC202.MUS";
		music2 = "BLANK.MUS";
		music3 = "DRONK.MUS";
		declare_object_at_cell((0 / 40) + 1, -20, (0 / 40), 51 /*start stand*/, 0, 0, 0, 0, 0);
		
		declare_object_at_cell(-(260 / 40) + 1, -69, (380 / 40), 18 /*post00*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(340 / 40) + 1, -69, (300 / 40), 18 /*post00*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(-(220 / 40) + 1, -69, (180 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);
		declare_object_at_cell(-(140 / 40) + 1, -69, (260 / 40), 18 /*post00*/, 0, 0, 0, 0, 1<<4);
		declare_object_at_cell((220 / 40) + 1, -69, -(180 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);
		declare_object_at_cell((140 / 40) + 1, -69, -(260 / 40), 18 /*post00*/, 0, 0, 0, 0, 2<<4);
		
		declare_object_at_cell((260 / 40), -10, (140 / 40), 1 /*t item*/, 0, 0, 0, 		0, 0);
		declare_object_at_cell((260 / 40), -10, (180 / 40), 2 /*t item*/, 0, 15, 0, 	0, 0);
		declare_object_at_cell((260 / 40), -10, (220 / 40), 3 /*t item*/, 0, 30, 0, 	0, 0);
		declare_object_at_cell((260 / 40), -10, (260 / 40), 4 /*t item*/, 0, 45, 0, 	0, 0);
		declare_object_at_cell((260 / 40), -10, (300 / 40), 5 /*t item*/, 0, 90, 0, 	0, 0);
		declare_object_at_cell((260 / 40), -10, (340 / 40), 6 /*t item*/, 0, 105, 0, 	0, 0);
		declare_object_at_cell((260 / 40), -10, (380 / 40), 7 /*t item*/, 0, 120, 0, 	0, 0);
		
		declare_object_at_cell(-(300 / 40) + 1, -4, -(340 / 40), 53 /*flag stand*/, 0, 0, 0, 0, 0);
		
		declare_object_at_cell((340 / 40) + 1, -5, -(340 / 40), 54 /*goal stand*/, 0, 0, 0, 0, 0);
		
		declare_object_at_cell((120 / 40) + 1, -100, -(0 / 40), 22 /*float03*/, 0, 0, 0, 0, 0);
		return;
		break;
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
	file.open(fname, ios::out);
	
	int name1len = 12 - music1.length();
	int name2len = 12 - music2.length();
	int name3len = 12 - music3.length();
	
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
	
	file << "OBJECTS!";
	dptr = &objects_data_list[0];
	writeUint16(&file, obj_count);
	for(int i = 0; i < adder; i++)
	{
		writeUint16(&file, objects_data_list[i]);
	}
	
	cout << "Success, hopefully!\n";
	int hold;
	cin >> hold;
	
	return 1;
}

