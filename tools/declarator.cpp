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
	*dptr++ = -height;
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
		
		declare_object_at_cell(-200, (2000 - 110), 0, 11 /*start stand*/, 0, 0, 0, 0, 0);
		declare_object_at_cell(0, (2000 - 0), 0, 12 /*testmap*/, 0, 0, 0, 0, 0);
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

