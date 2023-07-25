//subrender.c
//This file compiled separately

#include <SL_DEF.H>
#include "def.h"
#include "mymath.h"
#include "render.h"

	/*
	
	TEXTURE ID MATRIX:
	1	- full size
	2	- full y, 1/2 x
	3	- full y, 1/4 x
	4	- full y, 1/8 x
	5	- 1/2 y, full x
	6	- 1/2 y, 1/2 x
	7	- 1/2 y, 1/4 x
	8	- 1/2 y, 1/8 x
	9	- 1/4 y, full x
	10	- 1/4 y, 1/2 x
	11	- 1/4 y, 1/4 x
	12	- 1/4 y, 1/8 x
	13	- 1/8 y, full x
	14	- 1/8 y, 1/2 x
	15	- 1/8 y, 1/4 x
	16	- 1/8 y, 1/8 x
	This needs to move to a texture coordinate system...
	
	SUBDIVISION TABLES
	+ rule:
	1 | 2
	- + -
	3 | 4
	
	- rule:
	1
	-
	2
	
	| rule:
	1	|	2
	
	How will the tables work?
	t_rules[224][4];
	Put in the texture # being subdivided, it spits out the texture # of the subdivisions.
	So, thusly, if you assign the texture # correctly, you can just walk that down to the final textures all the time.
	Now I just have to write this table back into C.
	
	In this system, I also want to allow tiled textures, in addition to UV cut textures.
	but I have to get the UV cut textures working properly first
	
	For UV cuts from 64x64
	1 = 8 (in u and v)
	
	Starting from:
	x1y1,x8y8 = 1 (downscaled),		+++	(64x64)
		1+:	30
		1+:	31
		1+:	32
		1+:	33
			
	x1y1,x4y8 = 2 (downscaled),		-++	(32x64)
		1-:	30
		1-: 32
			
	x5y1,x8y8 = 3 (downscaled),		-++
		1-: 31
		1-: 33
			
	x1y1,x8y4 = 4 (downscaled),		|++	(64x32)
		1|:	30
		1|:	31
			
	x1y5,x8y8 = 5 (downscaled),		|++
		1|:	32
		1|:	33
			
	x1y1,x2y8 = 6 (downscaled),		--+	(16x64)
		1-:	34
		1-:	38
			
	x3y1,x4y8 = 7 (downscaled),		--+
		1-:	32
		1-:	39
			
	x5y1,x6y8 = 8 (downscaled),		--+
		1-:	31
		1-:	36
			
	x7y1,x8y8 = 9 (downscaled),		--+
		1-:	37
		1-:	41
			
	x1y1,x8y2 = 10 (downscaled,		||+	(64x16)
		1|:	42
		1|:	44
	
	x1y3,x8y4 = 11 (downscaled),	||+
		1|:	43
		1|:	45
	
	x1y5,x8y6 = 12 (downscaled),	||+
		1|:	46
		1|:	48
	
	x1y7,x8y8 = 13 (downscaled),	||+
		1|:	47
		1|:	49
	
	x1y1,x1y8 = 14 (downscaled),	---	(8x64)
		1-: 50
		1-: 58
	x2y1,x2y8 = 15 (downscaled),	---
		1-: 51
		1-: 59
	x3y1,x3y8 = 16 (downscaled),	---
		1-: 52
		1-: 60
	x4y1,x4y8 = 17 (downscaled),	---
		1-: 53
		1-: 61
	x5y1,x5y8 = 18 (downscaled),	---
		1-: 54
		1-: 62
	x6y1,x6y8 = 19 (downscaled),	---
		1-: 55
		1-: 63
	x7y1,x7y8 = 20 (downscaled),	---
		1-: 56
		1-: 64
	x8y1,x8y8 = 21 (downscaled),	---	
		1-: 57
		1-: 65
	
	x1y1,x8y1 = 22 (downscaled),	|||	(64x8)
		1|: 66
		1|: 70
	x1y2,x8y2 = 23 (downscaled),	|||
		1|: 67
		1|: 71
	x1y3,x8y3 = 24 (downscaled),	|||
		1|: 68
		1|: 72
	x1y4,x8y4 = 25 (downscaled),	|||
		1|: 69
		1|: 73
	x1y5,x8y5 = 26 (downscaled),	|||
		1|: 74
		1|: 78
	x1y6,x8y6 = 27 (downscaled),	|||
		1|: 75
		1|: 79
	x1y7,x8y7 = 28 (downscaled),	|||
		1|: 76
		1|: 80
	x1y8,x8y8 = 29 (downscaled),	|||
		1|: 77
		1|: 81
	
	x1y1,x4y4 = 30,					++	(32x32)
		1+:	82
		1+: 83
		1+: 86
		1+: 87

	x5y1,x8y4 = 31,					++
		1+: 84
		1+: 85
		1+: 88
		1+: 89
	
	x1y5,x4y8 = 32,					++
		1+: 90
		1+: 91
		1+: 94
		1+: 95
	
	x5y5,x8y8 = 33,					++
		1+: 92
		1+: 93
		1+: 96
		1+: 97
	
	x1y1,x2y4 = 34,					-+	(16x32)
		1-: 82
		1-: 86
	x3y1,x4y4 = 35,					-+
		1-: 83
		1-: 87
	x5y1,x6y4 = 36,					-+
		1-: 84
		1-: 88
	x7y1,x8y4 = 37,					-+
		1-: 85
		1-: 89
	x1y5,x2y8 = 38,					-+
		1-: 90
		1-: 94
	x3y5,x4y8 = 39,					-+
		1-: 91
		1-: 95
	x5y5,x6y8 = 40,					-+
		1-: 92
		1-: 96
	x7y5,x8y8 = 41,					-+
		1-: 93
		1-: 97
	
	-------------------------------------------
	(alerta: zona de accidentes)
	x1y1,x4y2 = 42,					|+	(32x16)
		1|: 82
		1|: 83
	x1y3,x4y4 = 43,					|+
		1|: 86
		1|: 87
	x5y1,x8y2 = 44,					|+
		1|: 84
		1|: 85
	x5y3,x8y4 = 45,					|+
		1|: 88
		1|: 89
	x1y5,x4y6 = 46,					|+
		1|: 90
		1|: 91
	x1y7,x4y8 = 47,					|+
		1|: 94
		1|: 95
	x5y5,x8y6 = 48,					|+
		1|: 92
		1|: 93
	x5y7,x8y8 = 49,					|+
		1|: 96
		1|: 97
	--------------------------------------------
	x1y1,x1y4 = 50,					--	(8x32)
		1-: 98
		1-: 99
	x2y1,x2y4 = 51,					--
		1-: 100
		1-: 101
	x3y1,x3y4 = 52,					--
		1-: 102
		1-: 103
	x4y1,x4y4 = 53,					--
		1-: 104
		1-: 105
	x5y1,x5y4 = 54,					--
		1-: 106
		1-: 107
	x6y1,x6y4 = 55,					--
		1-: 108
		1-: 109
	x7y1,x7y4 = 56,					--
		1-: 110
		1-: 111
	x8y1,x8y4 = 57,					--
		1-: 112
		1-: 113
	x1y5,x1y8 = 58,					--
		1-: 114
		1-: 115
    x2y5,x2y8 = 59,					--
		1-: 116
		1-: 117
    x3y5,x3y8 = 60,					--
		1-: 118
		1-: 119
    x4y5,x4y8 = 61,					--
		1-: 120
		1-: 121
    x5y5,x5y8 = 62,					--
		1-: 122
		1-: 123
    x6y5,x6y8 = 63,					--
		1-: 124
		1-: 125
    x7y5,x7y8 = 64,					--
		1-: 126
		1-: 127
    x8y5,x8y8 = 65,					--
		1-: 128
		1-: 129
	--------------------------------------------
	alerta: zona de accidentes
	x1y1,x4y1 = 66,					||	(32x8)
		1|: 130
		1|: 132
	x1y2,x4y2 = 67,					||
		1|: 131
		1|: 133
	x1y3,x4y3 = 68,					||
		1|: 138
		1|: 140
	x1y4,x4y4 = 69,					||
		1|: 139
		1|: 141
	
	x5y1,x8y1 = 70,					||
		1|: 134
		1|: 136
	x5y2,x8y2 = 71,					||
		1|: 135
		1|: 137
	x5y3,x8y3 = 72,					||
		1|: 142
		1|: 144
	x5y4,x8y4 = 73,					||
		1|: 143
		1|: 145
	
	x1y5,x4y5 = 74,					||
		1|: 146
		1|: 148
    x1y6,x4y6 = 75,					||
		1|: 147
		1|: 149
    x1y7,x4y7 = 76,					||
		1|: 154
		1|: 156
    x1y8,x4y8 = 77,					||
		1|: 155
		1|: 157
	
    x5y5,x8y5 = 78,					||
		1|: 150
		1|: 152
    x5y6,x8y6 = 79,					||
		1|: 151
		1|: 153
    x5y7,x8y7 = 80,					||
		1|: 158
		1|: 160
    x5y8,x8y8 = 81,					||	
		1|: 159
		1|: 161
	--------------------------------------------
	x1y1,x2y2 = 82					+	(16x16)
	162, 166, 163, 167
	x3y1,x4y2 = 83					+
	170, 174, 171, 175
	x5y1,x6y2 = 84					+
	178, 182, 179, 183
	x7y1,x8y2 = 85					+
	186, 190, 187, 191
	x1y3,x2y4 = 86					+	
	164, 168, 165, 169
	x3y3,x4y4 = 87					+
	172, 176, 173, 177
	x5y3,x6y4 = 88					+
	180, 184, 181, 185
	x7y3,x8y4 = 89					+
	188, 192, 189, 193
	x1y5,x2y6 = 90					+	
	194, 198, 195, 199
	x3y5,x4y6 = 91					+
	202, 206, 203, 207
	x5y5,x6y6 = 92					+
	210, 214, 211, 215
	x7y5,x8y6 = 93					+
	218, 222, 219, 223
	x1y7,x2y8 = 94					+	
	196, 200, 197, 201
	x3y7,x4y8 = 95					+
	204, 208, 205, 209
	x5y7,x6y8 = 96					+
	212, 216, 213, 217
	x7y7,x8y8 = 97					+
	220, 224, 221, 225
	--------------------------------------------
	alerta: zona de accidentes
	
	x1y1,x1y2 = 98,					-	(8x16)
	162, 163
	x1y3,x1y4 = 99,					-
	164, 165
	x2y1,x2y2 = 100,				-
	166, 167
	x2y3,x2y4 = 101,				-	
	168, 169
	x3y1,x3y2 = 102,				-
	170, 171
	x3y3,x3y4 = 103,				-	
	172, 173
	x4y1,x4y2 = 104,				-
	174, 175
	x4y3,x4y4 = 105,				-	
	176, 177
	x5y1,x5y2 = 106,				-
	178, 179
	x5y3,x5y4 = 107,				-
	180, 181
	x6y1,x6y2 = 108,				-
	182, 183
	x6y3,x6y4 = 109,				-
	184, 185
	x7y1,x7y2 = 110,				-
	186, 187
	x7y3,x7y4 = 111,				-
	188, 189
	x8y1,x8y2 = 112,				-
	190, 191
	x8y3,x8y4 = 113,				-
	192, 193
	
	x1y5,x1y6 = 114,				-	
	194, 195
	x1y7,x1y8 = 115,				-
	196, 197
	x2y5,x2y6 = 116,				-
	198, 199
	x2y7,x2y8 = 117,				-	
	200, 201
	x3y5,x3y6 = 118,				-
	202, 203
	x3y7,x3y8 = 119,				-	
	204, 205
	x4y5,x4y6 = 120,				-
	206, 207
	x4y7,x4y8 = 121,				-	
	208, 209
	x5y5,x5y6 = 122,				-
	210, 211
	x5y7,x5y8 = 123,				-
	212, 213
	x6y5,x6y6 = 124,				-
	214, 215
	x6y7,x6y8 = 125,				-
	216, 217
	x7y5,x7y6 = 126,				-
	218, 219
	x7y7,x7y8 = 127,				-
	220, 221
	x8y5,x8y6 = 128,				-
	222, 223
	x8y7,x8y8 = 129,				-
	224, 225
	--------------------------------------------
	mora zona de accidentes
	
	x1y1,x2y1 = 130,				|	(16x8)
	162, 166
	x1y2,x2y2 = 131,				|
	163, 167
	x3y1,x4y1 = 132,				|
	170, 174
	x3y2,x4y2 = 133,				|	
	171, 175
	x5y1,x6y1 = 134,				|
	178, 182
	x5y2,x6y2 = 135,				|
	179, 183
	x7y1,x8y1 = 136,				|
	186, 190
	x7y2,x8y2 = 137,				|	
	187, 191

	x1y3,x2y3 = 138,				|
	164, 168
	x1y4,x2y4 = 139,				|
	165, 169
	x3y3,x4y3 = 140,				|
	172, 176
	x3y4,x4y4 = 141,				|
	173, 177
	x5y3,x6y3 = 142,				|
	180, 184
	x5y4,x6y4 = 143,				|
	181, 185
	x7y3,x8y3 = 144,				|
	188, 192
	x7y4,x8y4 = 145,				|
	189, 193

	x1y5,x2y5 = 146,				|
	194, 198
	x1y6,x2y6 = 147,				|
	195, 199
	x3y5,x4y5 = 148,				|
	202, 206
	x3y6,x4y6 = 149,				|
	203, 207
	x5y5,x6y5 = 150,				|
	210, 214
	x5y6,x6y6 = 151,				|
	211, 215
	x7y5,x8y5 = 152,				|
	218, 222
	x7y6,x8y6 = 153,				|
	219, 223
	
	x1y7,x2y7 = 154,				|
	196, 200
	x1y8,x2y8 = 155,				|
	197, 201
	x3y7,x4y7 = 156,				|
	204, 208
	x3y8,x4y8 = 157,				|
	205, 209
	x5y7,x6y7 = 158,				|
	212, 216
	x5y8,x6y8 = 159,				|
	213, 217
	x7y7,x8y7 = 160,				|
	220, 224
	x7y8,x8y8 = 161,				|
	221, 225
	
	--------------------------------------------
	mora zona de accidentes
	
	x1y1 = 162,
	x1y2 = 163,
	x1y3 = 164,
	x1y4 = 165,
	x2y1 = 166,
	x2y2 = 167,
	x2y3 = 168,
	x2y4 = 169,
	x3y1 = 170,
	x3y2 = 171,
	x3y3 = 172,
	x3y4 = 173,
	x4y1 = 174,
	x4y2 = 175,
	x4y3 = 176,
	x4y4 = 177,
	x5y1 = 178,
	x5y2 = 179,
	x5y3 = 180,
	x5y4 = 181,
	x6y1 = 182
	x6y2 = 183
	x6y3 = 184
	x6y4 = 185
	x7y1 = 186
	x7y2 = 187
	x7y3 = 188
	x7y4 = 189
	x8y1 = 190
	x8y2 = 191
	x8y3 = 192
	x8y4 = 193
	
	x1y5 = 194
	x1y6 = 195
	x1y7 = 196
	x1y8 = 197
	x2y5 = 198
	x2y6 = 199
	x2y7 = 200
	x2y8 = 201
	x3y5 = 202
	x3y6 = 203
	x3y7 = 204
	x3y8 = 205
	x4y5 = 206
	x4y6 = 207
	x4y7 = 208
	x4y8 = 209
	x5y5 = 210
	x5y6 = 211
	x5y7 = 212
	x5y8 = 213
	x6y5 = 214
	x6y6 = 215
	x6y7 = 216
	x6y8 = 217
	x7y5 = 218
	x7y6 = 219
	x7y7 = 220
	x7y8 = 221
	x8y5 = 222
	x8y6 = 223
	x8y7 = 224
	x8y8 = 225
	
	*/


unsigned short texIDs_cut_from_texID[225][4] = {
	{30, 31, 32, 33}, // +++
	{30, 32, 0, 0}, // -++
	{31, 33, 0, 0}, // -++
	{30, 31, 0, 0}, // |++
	{32, 33, 0, 0}, // |++
	{34, 38, 0, 0}, // --+
	{32, 39, 0, 0}, // --+
	{31, 36, 0, 0}, // --+
	{37, 41, 0, 0}, // --+
	{42, 44, 0, 0}, // ||+
	{43, 45, 0, 0}, // ||+
	{46, 48, 0, 0}, // ||+
	{47, 49, 0, 0}, // ||+
	{50, 58, 0, 0}, // ---
	{51, 59, 0, 0}, // ---
	{52, 60, 0, 0}, // ---
	{53, 61, 0, 0}, // ---
	{54, 62, 0, 0}, // ---
	{55, 63, 0, 0}, // ---
	{56, 64, 0, 0}, // ---
	{57, 65, 0, 0}, // ---
	{66, 70, 0, 0}, // |||
	{67, 71, 0, 0}, // |||
	{68, 72, 0, 0}, // |||
	{69, 73, 0, 0}, // |||
	{74, 78, 0, 0}, // |||
	{75, 79, 0, 0}, // |||
	{76, 80, 0, 0}, // |||
	{77, 81, 0, 0}, // |||
	{82, 83, 86, 86}, // ++
	{84, 85, 88, 89}, // ++
	{90, 91, 94, 95}, // ++
	{92, 93, 96, 97}, // ++
	{82, 86, 0, 0}, // -+
	{83, 87, 0, 0}, // -+
	{84, 88, 0, 0}, // -+
	{85, 89, 0, 0}, // -+
	{90, 94, 0, 0}, // -+
	{91, 95, 0, 0}, // -+
	{92, 96, 0, 0}, // -+
	{93, 97, 0, 0}, // -+
	{82, 83, 0, 0}, // |+
	{86, 87, 0, 0}, // |+
	{84, 85, 0, 0}, // |+
	{88, 89, 0, 0}, // |+
	{90, 91, 0, 0}, // |+
	{94, 95, 0, 0}, // |+
	{92, 93, 0, 0}, // |+
	{96, 97, 0, 0}, // |+
	{98, 99, 0, 0}, // --
	{100, 101, 0, 0}, // --
	{102, 103, 0, 0}, // --
	{104, 105, 0, 0}, // --
	{106, 107, 0, 0}, // --
	{108, 109, 0, 0}, // --
	{110, 111, 0, 0}, // --
	{112, 113, 0, 0}, // --
	{114, 115, 0, 0}, // --
	{116, 117, 0, 0}, // --
	{118, 119, 0, 0}, // --
	{120, 121, 0, 0}, // --
	{122, 123, 0, 0}, // --
	{124, 125, 0, 0}, // --
	{126, 127, 0, 0}, // --
	{128, 129, 0, 0}, // --
	{130, 132, 0, 0}, // ||
	{131, 133, 0, 0}, // ||
	{138, 140, 0, 0}, // ||
	{139, 141, 0, 0}, // ||
	{134, 136, 0, 0}, // ||
	{135, 137, 0, 0}, // ||
	{142, 144, 0, 0}, // ||
	{143, 145, 0, 0}, // ||
	{146, 148, 0, 0}, // ||
	{147, 149, 0, 0}, // ||
	{154, 156, 0, 0}, // ||
	{155, 157, 0, 0}, // ||
	{150, 152, 0, 0}, // ||
	{151, 153, 0, 0}, // ||
	{158, 160, 0, 0}, // ||
	{159, 161, 0, 0}, // ||
	{162, 166, 163, 167}, // +
	{170, 174, 171, 175}, // +
	{178, 182, 179, 183}, // +
	{186, 190, 187, 191}, // +
	{164, 168, 165, 169}, // +
	{172, 176, 173, 177}, // +
	{180, 184, 181, 185}, // +
	{188, 192, 189, 193}, // +
	{194, 198, 195, 199}, // +
	{202, 206, 203, 207}, // +
	{210, 214, 211, 215}, // +
	{218, 222, 219, 223}, // +
	{196, 200, 197, 201}, // +
	{204, 208, 205, 209}, // +
	{212, 216, 213, 217}, // +
	{220, 224, 221, 225}, // +
	{162, 163, 0, 0}, // -
	{164, 165, 0, 0}, // -
	{166, 167, 0, 0}, // -
	{168, 169, 0, 0}, // -
	{170, 171, 0, 0}, // -
	{172, 173, 0, 0}, // -
	{174, 175, 0, 0}, // -
	{176, 177, 0, 0}, // -
	{178, 179, 0, 0}, // -
	{180, 181, 0, 0}, // -
	{182, 183, 0, 0}, // -
	{184, 185, 0, 0}, // -
	{186, 187, 0, 0}, // -
	{188, 189, 0, 0}, // -
	{190, 191, 0, 0}, // -
	{192, 193, 0, 0}, // -
	{194, 195, 0, 0}, // -
	{196, 197, 0, 0}, // -
	{198, 199, 0, 0}, // -
	{200, 201, 0, 0}, // -
	{202, 203, 0, 0}, // -
	{204, 205, 0, 0}, // -
	{206, 207, 0, 0}, // -
	{208, 209, 0, 0}, // -
	{210, 211, 0, 0}, // -
	{212, 213, 0, 0}, // -
	{214, 215, 0, 0}, // -
	{216, 217, 0, 0}, // -
	{218, 219, 0, 0}, // -
	{220, 221, 0, 0}, // -
	{222, 223, 0, 0}, // -
	{224, 225, 0, 0}, // -
	{162, 166, 0, 0}, // |
	{163, 167, 0, 0}, // |
	{170, 174, 0, 0}, // |
	{171, 175, 0, 0}, // |
	{178, 182, 0, 0}, // |
	{179, 183, 0, 0}, // |
	{186, 190, 0, 0}, // |
	{187, 191, 0, 0}, // |
	{164, 168, 0, 0}, // |
	{165, 169, 0, 0}, // |
	{172, 176, 0, 0}, // |
	{173, 177, 0, 0}, // |
	{180, 184, 0, 0}, // |
	{181, 185, 0, 0}, // |
	{188, 192, 0, 0}, // |
	{189, 193, 0, 0}, // |
	{194, 198, 0, 0}, // |
	{195, 199, 0, 0}, // |
	{202, 206, 0, 0}, // |
	{203, 207, 0, 0}, // |
	{210, 214, 0, 0}, // |
	{211, 215, 0, 0}, // |
	{218, 222, 0, 0}, // |
	{219, 223, 0, 0}, // |
	{196, 200, 0, 0}, // |
	{197, 201, 0, 0}, // |
	{204, 208, 0, 0}, // |
	{205, 209, 0, 0}, // |
	{212, 216, 0, 0}, // |
	{213, 217, 0, 0}, // |
	{220, 224, 0, 0}, // |
	{221, 225, 0, 0}  // |
	//(remaining values do not subdivide)
};

	
	#define SUBDIVIDE_W		(1)
	#define SUBDIVIDE_H		(2)
	#define SUBDIVIDE_HV	(3)
	
	short		rule_to_texture[4] = {0, 1, 4, 5};

	#define TEXTS_GENERATED_PER_TEXTURE_LOADED (16)
	#define SUBDIVISION_NEAR_PLANE (15<<16)
	#define SUBDIVISION_SCALE (50)
	
	// What I know from other heightmap engines is that a CPU-efficient way to improve rendering speed
	// is by the addition of "occlusion planes" - in other words, polygons on the other side of the plane,
	// when viewed through the plane, are discarded.
	// That is effectively an anti-portal...

		POINT	subdivided_points[512];
		short	subdivided_polygons[512][4]; //4 Vertex IDs of the subdivided_points
		short	used_textures[512];
		short	sub_poly_cnt = 0;
		short	sub_vert_cnt = 0;
		short	subdivision_rules[4]	= {0, 0, 0, 0};
		short	texture_rules[4]		= {16, 16, 16, 0};
		// **really** trying to squeeze the performance here
		int		z_rules[4]				= {300<<16, 66<<16, 33<<16, 0};

void	subdivide_plane(short start_point, short overwritten_polygon, short num_divisions, short total_divisions)
{

	//"Load" the original points (code shortening operation)
	FIXED * ptv[4];
	static int max_z;
	static char new_rule;

	ptv[0] = &subdivided_points[subdivided_polygons[overwritten_polygon][0]][X];
	ptv[1] = &subdivided_points[subdivided_polygons[overwritten_polygon][1]][X];
	ptv[2] = &subdivided_points[subdivided_polygons[overwritten_polygon][2]][X];
	ptv[3] = &subdivided_points[subdivided_polygons[overwritten_polygon][3]][X];
	
	//if(ptv[0][Z] < 0 && ptv[1][Z] < 0 && ptv[2][Z] < 0 && ptv[3][Z] < 0) return;
	new_rule = subdivision_rules[total_divisions];

	used_textures[overwritten_polygon] = texture_rules[total_divisions];
	if(num_divisions == 0 || subdivision_rules[total_divisions] == 0)
	{
		return;
	}

	//////////////////////////////////////////////////////////////////
	// Warning: There is no extreme polygon smallness exit.
	// It will mess with texture assignment so I got rid of it.
	// Unfortunately I think this needs to come back in some way to handle trapezoids.
	//////////////////////////////////////////////////////////////////
	//if(minLen <= 50 && maxLen <= 50)
	//{
		//return;
	//} 
	//////////////////////////////////////////////////////////////////
	// Quick check: If we are subdividing a polygon above the z level, stop further subdivision.
	// This is mostly useful in cases where a large polygon is being recursively subdivided and parts of it may be far away.
	//////////////////////////////////////////////////////////////////
	int polygon_minimum = JO_MIN(JO_MIN(ptv[0][Z], ptv[1][Z]),  JO_MIN(ptv[2][Z], ptv[3][Z]));
	if(polygon_minimum > z_rules[total_divisions])
	{
		return;
	}

	short tgt_pnt = start_point;
	
	short poly_a = overwritten_polygon; //Polygon A is a polygon ID we will overwrite (replace the original polygon)
	short poly_b = sub_poly_cnt;
	short poly_c = sub_poly_cnt+1;
	short poly_d = sub_poly_cnt+2;
	
	if(new_rule == SUBDIVIDE_HV) 
	{
	//////////////////////////////////////////////////////////////////
	// Subdivide by all rules / Subdivide polygon into four new quads
	//Turn 4 points into 9 points
	//Make the 4 new polygons
	//////////////////////////////////////////////////////////////////
	/*
	//Why break chirality? to comply with the texture coordinate system (more easily, anyway)
	0A			1A | 0B			1B
							
			A				B
							
	3A			2A | 3B			2B		
	
	0D			1D | 0C			1C
	
			C				D

	3D			2D | 3C			2C
	*/
	// Initial Conditions
	subdivided_polygons[poly_a][0] = subdivided_polygons[overwritten_polygon][0];
	subdivided_polygons[poly_b][1] = subdivided_polygons[overwritten_polygon][1];
	subdivided_polygons[poly_d][2] = subdivided_polygons[overwritten_polygon][2];
	subdivided_polygons[poly_c][3] = subdivided_polygons[overwritten_polygon][3];

	// Center
	// 
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[1][X] + 
										ptv[2][X] + ptv[3][X])>>2;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[1][Y] + 
										ptv[2][Y] + ptv[3][Y])>>2;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[1][Z] + 
										ptv[2][Z] + ptv[3][Z])>>2;
	

	subdivided_polygons[poly_a][2] = tgt_pnt;
	subdivided_polygons[poly_b][3] = tgt_pnt;
	subdivided_polygons[poly_d][0] = tgt_pnt;
	subdivided_polygons[poly_c][1] = tgt_pnt;

	tgt_pnt++;
	// 0 -> 1
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[1][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[1][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[1][Z])>>1;
	
	subdivided_polygons[poly_a][1] = tgt_pnt;
	subdivided_polygons[poly_b][0] = tgt_pnt;
	tgt_pnt++;
	// 1 -> 2
	subdivided_points[tgt_pnt][X] = (ptv[2][X] + ptv[1][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[2][Y] + ptv[1][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[2][Z] + ptv[1][Z])>>1;
	
	subdivided_polygons[poly_b][2] = tgt_pnt;
	subdivided_polygons[poly_d][1] = tgt_pnt;
	tgt_pnt++;
	// 3 -> 2
	subdivided_points[tgt_pnt][X] = (ptv[2][X] + ptv[3][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[2][Y] + ptv[3][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[2][Z] + ptv[3][Z])>>1;
	
	subdivided_polygons[poly_d][3] = tgt_pnt;
	subdivided_polygons[poly_c][2] = tgt_pnt;
	tgt_pnt++;
	// 3 -> 0
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[3][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[3][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[3][Z])>>1;
	
	subdivided_polygons[poly_a][3] = tgt_pnt;
	subdivided_polygons[poly_c][0] = tgt_pnt;
	tgt_pnt++;
	sub_vert_cnt = tgt_pnt;
	sub_poly_cnt += 3; //Only add 3, as there was already 1 polygon. It was split into four.
	
		if(num_divisions > 0)
		{
		///////////////////////////////////////////
		//	Recursively subdivide the polygon.
		//	Check the maximum Z of every new polygon.
		// 	If the maximum Z is less than zero, it's not on screen. No point in subdividing it any further.
		///////////////////////////////////////////
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][0]][Z], subdivided_points[subdivided_polygons[poly_a][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][2]][Z], subdivided_points[subdivided_polygons[poly_a][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_a, num_divisions-1, total_divisions+1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][0]][Z], subdivided_points[subdivided_polygons[poly_b][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][2]][Z], subdivided_points[subdivided_polygons[poly_b][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_b, num_divisions-1, total_divisions+1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_c][0]][Z], subdivided_points[subdivided_polygons[poly_c][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_c][2]][Z], subdivided_points[subdivided_polygons[poly_c][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_c, num_divisions-1, total_divisions+1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_d][0]][Z], subdivided_points[subdivided_polygons[poly_d][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_d][2]][Z], subdivided_points[subdivided_polygons[poly_d][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_d, num_divisions-1, total_divisions+1);
		}
	
	} else if(new_rule == SUBDIVIDE_H) 
	{
	//////////////////////////////////////////////////////////////////
	// Subdivide between the edges 0->1 and 3->2 (""Vertically"")
	// (Splits the polygon such that new vertices are created between 0->3 and 1->2)
	//Turn 4 points into 6 points
	//Make the 2 new polygons
	//////////////////////////////////////////////////////////////////
	/*
	0A							1A		
					A
	3A--------------------------2A		
	0B--------------------------1B
					B
	3B							2B
	*/
	// Initial Conditions
	subdivided_polygons[poly_a][0] = subdivided_polygons[overwritten_polygon][0];
	subdivided_polygons[poly_a][1] = subdivided_polygons[overwritten_polygon][1];
	subdivided_polygons[poly_b][2] = subdivided_polygons[overwritten_polygon][2];
	subdivided_polygons[poly_b][3] = subdivided_polygons[overwritten_polygon][3];
	
	// 1 -> 2
	subdivided_points[tgt_pnt][X] = (ptv[2][X] + ptv[1][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[2][Y] + ptv[1][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[2][Z] + ptv[1][Z])>>1;
	
	subdivided_polygons[poly_a][2] = tgt_pnt;
	subdivided_polygons[poly_b][1] = tgt_pnt;
	tgt_pnt++;
	// 3 -> 0
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[3][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[3][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[3][Z])>>1;
	
	subdivided_polygons[poly_a][3] = tgt_pnt;
	subdivided_polygons[poly_b][0] = tgt_pnt;
	tgt_pnt++;
		
	sub_vert_cnt = tgt_pnt;
	sub_poly_cnt += 1; //Only add 1, as there was already 1 polygon. It was split in two.
	
		if(num_divisions > 0)
		{
		///////////////////////////////////////////
		//	Recursively subdivide the polygon.
		//	Check the maximum Z of every new polygon.
		// 	If the maximum Z is less than zero, it's not on screen. No point in subdividing it any further.
		///////////////////////////////////////////
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][0]][Z], subdivided_points[subdivided_polygons[poly_a][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][2]][Z], subdivided_points[subdivided_polygons[poly_a][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_a, num_divisions-1, total_divisions+1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][0]][Z], subdivided_points[subdivided_polygons[poly_b][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][2]][Z], subdivided_points[subdivided_polygons[poly_b][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_b, num_divisions-1, total_divisions+1);
		}
		
	} else if(new_rule == SUBDIVIDE_W)
	{
	//////////////////////////////////////////////////////////////////
	// Subdivide between the edges 0->3 and 1->2 (""Horizontally"")
	// (Splits the polygon such that new vertices are created between 0->1 and 3->2)
	//Turn 4 points into 6 points
	//Make the 2 new polygons
	//////////////////////////////////////////////////////////////////
	/*
	0A			 1A | 0B			1B
					|		
			A		|		B
					|
	3A			 2A | 3B			2B
	*/
	// Initial Conditions
	subdivided_polygons[poly_a][0] = subdivided_polygons[overwritten_polygon][0];
	subdivided_polygons[poly_a][3] = subdivided_polygons[overwritten_polygon][3];
	subdivided_polygons[poly_b][1] = subdivided_polygons[overwritten_polygon][1];
	subdivided_polygons[poly_b][2] = subdivided_polygons[overwritten_polygon][2];
		
	// 0 -> 1
	subdivided_points[tgt_pnt][X] = (ptv[0][X] + ptv[1][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[0][Y] + ptv[1][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[0][Z] + ptv[1][Z])>>1;
	
	subdivided_polygons[poly_a][1] = tgt_pnt;
	subdivided_polygons[poly_b][0] = tgt_pnt;
	tgt_pnt++;
	// 3 -> 2
	subdivided_points[tgt_pnt][X] = (ptv[2][X] + ptv[3][X])>>1;
	subdivided_points[tgt_pnt][Y] = (ptv[2][Y] + ptv[3][Y])>>1;
	subdivided_points[tgt_pnt][Z] = (ptv[2][Z] + ptv[3][Z])>>1;
	
	subdivided_polygons[poly_a][2] = tgt_pnt;
	subdivided_polygons[poly_b][3] = tgt_pnt;
	tgt_pnt++;
	
	sub_vert_cnt = tgt_pnt;
	sub_poly_cnt += 1; //Only add 1, as there was already 1 polygon. It was split in two.
	
		if(num_divisions > 0)
		{
		///////////////////////////////////////////
		//	Recursively subdivide the polygon.
		//	Check the maximum Z of every new polygon.
		// 	If the maximum Z is less than zero, it's not on screen. No point in subdividing it any further.
		///////////////////////////////////////////
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][0]][Z], subdivided_points[subdivided_polygons[poly_a][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_a][2]][Z], subdivided_points[subdivided_polygons[poly_a][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_a, num_divisions-1, total_divisions+1);
			
			max_z = JO_MAX(
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][0]][Z], subdivided_points[subdivided_polygons[poly_b][1]][Z]),
	JO_MAX(subdivided_points[subdivided_polygons[poly_b][2]][Z], subdivided_points[subdivided_polygons[poly_b][3]][Z])
					);
			if(max_z > 0) subdivide_plane(sub_vert_cnt, poly_b, num_divisions-1, total_divisions+1);
		}
	}
	
}

void	plane_rendering_with_subdivision(entity_t * ent)
{
	///////////////////////////////////////////
	// If the file is not yet loaded, do not try and render it.
	// If the entity type is not 'B' for BUILDING, do not try and render it as it won't have proper textures.
	///////////////////////////////////////////
	if(ent->file_done != true) return;
	if(ent->type != 'B') return;
	GVPLY * mesh = ent->pol;
	
	sub_poly_cnt = 0;
	sub_vert_cnt = 0;
	unsigned short	colorBank = 0;
	int		inverseZ = 0;
	int 	luma = 0;
	int 	zDepthTgt = 0;
	unsigned short	flags = 0;
	unsigned short	flip = 0;
	unsigned short	pclp = 0;

	POINT	pl_pts[4];

    static MATRIX newMtx;
	static FIXED m0x[4];
	static FIXED m1y[4];
	static FIXED m2z[4];
	
	//These can't have an orienation... eh, we'll do it anyway.
	slMultiMatrix((POINT *)ent->prematrix);
    slGetMatrix(newMtx);
	
	m0x[0] = newMtx[X][X];
	m0x[1] = newMtx[Y][X];
	m0x[2] = newMtx[Z][X];
	m0x[3] = newMtx[3][X];
	
	m1y[0] = newMtx[X][Y];
	m1y[1] = newMtx[Y][Y];
	m1y[2] = newMtx[Z][Y];
	m1y[3] = newMtx[3][Y];
	
	m2z[0] = newMtx[X][Z];
	m2z[1] = newMtx[Y][Z];
	m2z[2] = newMtx[Z][Z];
	m2z[3] = newMtx[3][Z];

	/**
	Rendering Planes
	With polygon subdivision based on the Z (depth)

	Load up a plane.
	Transform its vertices by the matrix, but don't explicitly screenspace transform it.
	These vertices will be "screen-centered", so now we can subdivide the plane.
	Check the span of the plane to see if it is large.
	If it large in one of two particular ways, subdivide it by its longitude or its latitude (make two from one).
	If it is large in both ways, subdivide it both ways (make four from one).
	At this point, subdivision will occur recursively up to a limit arbitrated from the plane's Z,
	will cease subdivision on polygons with a high Z, and continue subdivision on polygons with a low Z.
	
	All but the lowest subdivision of a polygon will receive a combined texture.
	The texture is either a X*2, Y*2, or an X*2 & Y*2 combination -- the same as the subdivision pattern of the polygon.
	**/

	int max_z = 0;
	//int min_z = 0;
	
	int specific_texture = 0;
	int dual_plane = 0;
	////////////////////////////////////////////////////
	// Transform each light source position by the matrix parameters.
	////////////////////////////////////////////////////
	// POINT relative_light_pos = {0, 0, 0};
	// static POINT tx_light_pos[MAX_DYNAMIC_LIGHTS];
	// FIXED * mesh_position = &ent->prematrix[9];
	// int inverted_proxima;
	
	// for(int l = 0; l < MAX_DYNAMIC_LIGHTS; l++)
	// {
		// if(active_lights[l].pop == 1)
		// {
			// relative_light_pos[X] = -active_lights[l].pos[X] - mesh_position[X];
			// relative_light_pos[Y] = -active_lights[l].pos[Y] - mesh_position[Y];
			// relative_light_pos[Z] = -active_lights[l].pos[Z] - mesh_position[Z];
			// tx_light_pos[l][X] = trans_pt_by_component(relative_light_pos, m0x);
			// tx_light_pos[l][Y] = trans_pt_by_component(relative_light_pos, m1y);
			// tx_light_pos[l][Z] = trans_pt_by_component(relative_light_pos, m2z);
		// }
	// }

for(unsigned int i = 0; i < mesh->nbPolygon; i++)
{
		sub_vert_cnt = 0;
		sub_poly_cnt = 0;
		
	flags = mesh->attbl[i].render_data_flags;
		
	for(int u = 0; u < 4; u++)
	{
	//////////////////////////////////////////////////////////////
	// Load the points
	//////////////////////////////////////////////////////////////
		pl_pts[u][X] = (mesh->pntbl[mesh->pltbl[i].vertices[u]][X]);
		pl_pts[u][Y] = (mesh->pntbl[mesh->pltbl[i].vertices[u]][Y]);
		pl_pts[u][Z] = (mesh->pntbl[mesh->pltbl[i].vertices[u]][Z]);
	//////////////////////////////////////////////////////////////
	// Matrix transformation of the plane's points
	// Note: Does not yet transform to screenspace, clip by screen or portal, or push out to near plane.
	//////////////////////////////////////////////////////////////
        subdivided_points[u][Z] = trans_pt_by_component(pl_pts[u], m2z);
        subdivided_points[u][Y] = trans_pt_by_component(pl_pts[u], m1y);
        subdivided_points[u][X] = trans_pt_by_component(pl_pts[u], m0x);

		subdivided_polygons[0][u] = u;
	//////////////////////////////////////////////////////////////
	// Early screenspace transform to throw out off-screen planes
	//////////////////////////////////////////////////////////////
		//Push to near-plane
		ssh2VertArea[u].pnt[Z] = (subdivided_points[u][Z] > SUBDIVISION_NEAR_PLANE) ? subdivided_points[u][Z] : SUBDIVISION_NEAR_PLANE;
		//Get 1/z
		inverseZ = fxdiv(scrn_dist, ssh2VertArea[u].pnt[Z]);
        //Transform to screen-space
        ssh2VertArea[u].pnt[X] = fxm(subdivided_points[u][X], inverseZ)>>SCR_SCALE_X;
        ssh2VertArea[u].pnt[Y] = fxm(subdivided_points[u][Y], inverseZ)>>SCR_SCALE_Y;
        //Screen Clip Flags for on-off screen decimation
		ssh2VertArea[u].clipFlag = ((ssh2VertArea[u].pnt[X]) > TV_HALF_WIDTH) ? SCRN_CLIP_X : 0; 
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[X]) < -TV_HALF_WIDTH) ? SCRN_CLIP_NX : ssh2VertArea[u].clipFlag; 
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[Y]) > TV_HALF_HEIGHT) ? SCRN_CLIP_Y : ssh2VertArea[u].clipFlag;
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[Y]) < -TV_HALF_HEIGHT) ? SCRN_CLIP_NY : ssh2VertArea[u].clipFlag;
		ssh2VertArea[u].clipFlag |= ((ssh2VertArea[u].pnt[Z]) <= SUBDIVISION_NEAR_PLANE) ? CLIP_Z : ssh2VertArea[u].clipFlag;
		// clipping(&ssh2VertArea[u], USER_CLIP_INSIDE);
	}
		 if(ssh2VertArea[0].clipFlag
		 & ssh2VertArea[1].clipFlag
		 & ssh2VertArea[2].clipFlag
		 & ssh2VertArea[3].clipFlag) continue;
		 
	///////////////////////////////////////////
	//	Check the maximum Z of every new polygon.
	// 	This is the first polygon. So, if its maximum Z is too low, just discard it.
	///////////////////////////////////////////
	max_z = JO_MAX(JO_MAX(subdivided_points[subdivided_polygons[0][0]][Z], subdivided_points[subdivided_polygons[0][1]][Z]),
			JO_MAX(subdivided_points[subdivided_polygons[0][2]][Z], subdivided_points[subdivided_polygons[0][3]][Z]));
	if(max_z <= SUBDIVISION_NEAR_PLANE) continue;
	
	//////////////////////////////////////////////////////////////
	// Portal stuff
	// This plane rendering really has a lot of garbage in it, doesn't it?
	//////////////////////////////////////////////////////////////
	if(flags & GV_FLAG_PORTAL && current_portal_count < MAX_PORTALS)
	{
		for(int u = 0; u < 4; u++)
		{
		portals[current_portal_count].verts[u][X] = ssh2VertArea[u].pnt[X];
		portals[current_portal_count].verts[u][Y] = ssh2VertArea[u].pnt[Y];
		portals[current_portal_count].verts[u][Z] = ssh2VertArea[u].pnt[Z];
		}
		current_portal_count++;
	}
	if(!(flags & GV_FLAG_DISPLAY)) continue;
		 
	//////////////////////////////////////////////////////////////
	// Screen-space back face culling segment. Will also avoid if the plane is flagged as dual-plane.
	//////////////////////////////////////////////////////////////
	if(flags & GV_FLAG_SINGLE)
	{
		 int cross0 = (ssh2VertArea[1].pnt[X] - ssh2VertArea[3].pnt[X])
							* (ssh2VertArea[0].pnt[Y] - ssh2VertArea[2].pnt[Y]);
		 int cross1 = (ssh2VertArea[1].pnt[Y] - ssh2VertArea[3].pnt[Y])
							* (ssh2VertArea[0].pnt[X] - ssh2VertArea[2].pnt[X]);
		dual_plane = 0;
		if(cross0 >= cross1) continue;
	} else {
		dual_plane = 1;
	}
	
	//////////////////////////////////////////////////////////////
	// We have at least four vertices, and at least one polygon (the plane's data itself).
	//////////////////////////////////////////////////////////////
		sub_vert_cnt += 4;
		sub_poly_cnt += 1;
	
	//min_z = JO_MIN(JO_MIN(subdivided_points[subdivided_polygons[0][0]][Z], subdivided_points[subdivided_polygons[0][1]][Z]),
	//		JO_MIN(subdivided_points[subdivided_polygons[0][2]][Z], subdivided_points[subdivided_polygons[0][3]][Z]));
	///////////////////////////////////////////
	// Just a side note:
	// Doing subdivision in screen-space **does not work**.
	// Well, technically it works, but affine warping is experienced. It looks pretty awful.
	///////////////////////////////////////////
	
	if(flags & GV_FLAG_NDIV)
	{ 
		used_textures[0] = 0;
	} else {
	//////////////////////////////////////////////////////////////
	// Check: Find the polygon's scale and thus subdivision scale.
	// We find the true perimeter of the polygon.
	//////////////////////////////////////////////////////////////
		subdivision_rules[0] = mesh->attbl[i].plane_information & 0x3;
		subdivision_rules[1] = (mesh->attbl[i].plane_information>>2) & 0x3;
		subdivision_rules[2] = (mesh->attbl[i].plane_information>>4) & 0x3;
		subdivision_rules[3] = 0;
		
		
		if(!subdivision_rules[0] || subdivision_rules[3])
		{
			//In this case the polygon is too small or too large.
			//Large polygons will be excepted by making it look obviously wrong.
			used_textures[0] = 0;
		} else {
			///////////////////////////////////////////
			// The subdivision rules were pre-calculated by the converter tool.
			///////////////////////////////////////////
			// texture_rules[0] = 16;
			// texture_rules[1] = 16;
			// texture_rules[2] = 16;
			// texture_rules[3] = 16;
			short rule_0 = rule_to_texture[subdivision_rules[0]];
			short rule_1 = rule_to_texture[subdivision_rules[1]];
			short rule_2 = rule_to_texture[subdivision_rules[2]];
			texture_rules[0] = 16 - (rule_0 + rule_1 + rule_2);
			texture_rules[1] = texture_rules[0] + rule_0;
			texture_rules[2] = texture_rules[1] + rule_1;

			subdivide_plane(sub_vert_cnt, 0, 3, 0);
			///////////////////////////////////////////
			//
			// Screenspace Transform of SUBDIVIDED Vertices
			// v = subdivided point index
			// testing_planes[i] = plane data index
			///////////////////////////////////////////
			// Pre-loop: Set near-plane clip for first vertex, then set the division unit to work
			///////////////////////////////////////////
			ssh2VertArea[0].pnt[Z] = (subdivided_points[0][Z] > SUBDIVISION_NEAR_PLANE) ? subdivided_points[0][Z] : SUBDIVISION_NEAR_PLANE;
			SetFixDiv(scrn_dist, ssh2VertArea[0].pnt[Z]);
			for(int v = 0; v < sub_vert_cnt; v++)
			{
				//Push to near-plane for NEXT vertex
				ssh2VertArea[v+1].pnt[Z] = (subdivided_points[v+1][Z] > SUBDIVISION_NEAR_PLANE) ? subdivided_points[v+1][Z] : SUBDIVISION_NEAR_PLANE;
				//Get 1/z for CURRENT vertex
				inverseZ = *DVDNTL;
				//Set division for NEXT vertex
				SetFixDiv(scrn_dist, ssh2VertArea[v+1].pnt[Z]);
				//Transform to screen-space
				ssh2VertArea[v].pnt[X] = fxm(subdivided_points[v][X], inverseZ)>>SCR_SCALE_X;
				ssh2VertArea[v].pnt[Y] = fxm(subdivided_points[v][Y], inverseZ)>>SCR_SCALE_Y;
				//Screen Clip Flags for on-off screen decimation
				ssh2VertArea[v].clipFlag = ((ssh2VertArea[v].pnt[X]) > TV_HALF_WIDTH) ? SCRN_CLIP_X : 0; 
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[X]) < -TV_HALF_WIDTH) ? SCRN_CLIP_NX : ssh2VertArea[v].clipFlag; 
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) > TV_HALF_HEIGHT) ? SCRN_CLIP_Y : ssh2VertArea[v].clipFlag;
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Y]) < -TV_HALF_HEIGHT) ? SCRN_CLIP_NY : ssh2VertArea[v].clipFlag;
				ssh2VertArea[v].clipFlag |= ((ssh2VertArea[v].pnt[Z]) <= SUBDIVISION_NEAR_PLANE) ? CLIP_Z : ssh2VertArea[v].clipFlag;
				// clipping(&ssh2VertArea[v], USER_CLIP_INSIDE);
			}
		//Subdivision activation end stub
		}
	//Subdivision enabled end stub
	}
	///////////////////////////////////////////
	//
	// Z-sort Insertion & Command Arrangement of Polygons
	// j = subdivided polygon index
	//
	///////////////////////////////////////////
	if(ssh2SentPolys[0] + sub_poly_cnt > MAX_SSH2_SENT_POLYS) return;

	unsigned short usedCMDCTRL = (flags & GV_FLAG_POLYLINE) ? VDP1_POLYLINE_CMDCTRL : VDP1_BASE_CMDCTRL;
	flags = (((flags & GV_FLAG_MESH)>>1) | ((flags & GV_FLAG_DARK)<<4))<<8;

	vertex_t * ptv[5] = {0, 0, 0, 0, 0};
	for(int j = 0; j < sub_poly_cnt; j++)
	{
		
		ptv[0] = &ssh2VertArea[subdivided_polygons[j][0]];
		ptv[1] = &ssh2VertArea[subdivided_polygons[j][1]];
		ptv[2] = &ssh2VertArea[subdivided_polygons[j][2]];
		ptv[3] = &ssh2VertArea[subdivided_polygons[j][3]];
		
		 int offScrn = (ptv[0]->clipFlag & ptv[1]->clipFlag & ptv[2]->clipFlag & ptv[3]->clipFlag);
		///////////////////////////////////////////
		// Z-Sorting Stuff	
		// Uses weighted max
		// It's best to adjust how other things are sorted, rather than this,
		// because weighted max is the best between transparent floors/ceilings and walls.
		///////////////////////////////////////////
		zDepthTgt = (JO_MAX(
		JO_MAX(ptv[0]->pnt[Z], ptv[2]->pnt[Z]),
		JO_MAX(ptv[1]->pnt[Z], ptv[3]->pnt[Z])) + 
		((ptv[0]->pnt[Z] + ptv[1]->pnt[Z] + ptv[2]->pnt[Z] + ptv[3]->pnt[Z])>>2))>>1;
		// zDepthTgt = JO_MAX(JO_MAX(ptv[0]->pnt[Z], ptv[2]->pnt[Z]),
					// JO_MAX(ptv[1]->pnt[Z], ptv[3]->pnt[Z]));

			if(offScrn || zDepthTgt < NEAR_PLANE_DISTANCE || zDepthTgt > FAR_PLANE_DISTANCE) continue;
		///////////////////////////////////////////
		// Use a combined texture, if the subdivision system stated one should be used.
		// Otherwise, use the base texture.
		///////////////////////////////////////////
			if(used_textures[j] != 0)
			{
				//Could this be tabled? Would speed things up a bit.
				//Generally, when I UV-coordinate the textures, I'll have to use a table.
				//There will be a table for each subdivision level.
				specific_texture = ((mesh->attbl[i].texno - ent->base_texture) * TEXTS_GENERATED_PER_TEXTURE_LOADED)
				+ (ent->numTexture + ent->base_texture) + used_textures[j];
			} else {
				specific_texture = mesh->attbl[i].texno;
			}
		///////////////////////////////////////////
		// Flipping polygon such that vertice 0 is on-screen, or disable pre-clipping
		///////////////////////////////////////////
		flip = GET_FLIP_DATA(flags);
		preclipping(ptv, &flip, &pclp);
		///////////////////////////////////////////
		// Lighting Math
		// Using some approximation of an inverse squared law
		// The position of the polygon is treated as the average of points 0 and 2.
		///////////////////////////////////////////
		luma = 0;
/* 		for(int l = 0; l < MAX_DYNAMIC_LIGHTS; l++)
		{
			if(active_lights[l].pop == 1)
			{
				//This should be tabled for speed.
				//A 3D relative pos table should be used. 
				//Each entry is 10-bit precise.
				//The output for each entry is the dot product of the three entries divided into one (inverse).
				
				relative_light_pos[X] = (tx_light_pos[l][X] - ((subdivided_points[subdivided_polygons[j][0]][X]
														+ subdivided_points[subdivided_polygons[j][2]][X])>>1))>>12;
				relative_light_pos[Y] = (tx_light_pos[l][Y] - ((subdivided_points[subdivided_polygons[j][0]][Y]
														+ subdivided_points[subdivided_polygons[j][2]][Y])>>1))>>12;
				relative_light_pos[Z] = (tx_light_pos[l][Z] - ((subdivided_points[subdivided_polygons[j][0]][Z]
														+ subdivided_points[subdivided_polygons[j][2]][Z])>>1))>>12;
				inverted_proxima = ((relative_light_pos[X] * relative_light_pos[X]) +
									(relative_light_pos[Y] * relative_light_pos[Y]) +
									(relative_light_pos[Z] * relative_light_pos[Z]))>>8;
				inverted_proxima = (inverted_proxima < 65536) ? division_table[inverted_proxima] : 0;
						
				luma += inverted_proxima * (int)active_lights[l].bright;
			}
		//	if(luma > 0) break; // Early exit
		}

		luma = (luma < 0) ? 0 : luma; 
 */		luma += fxdot(mesh->nmtbl[i], active_lights[0].ambient_light);
		//If the plane is dual-plane, add the absolute luma, instead of the signed luma.
		luma = (dual_plane) ? JO_ABS(luma) : luma;
		luma += active_lights[0].min_bright; 
		determine_colorbank(&colorBank, &luma);
		//Shift the color bank code to the appropriate bits
		colorBank<<=6;
		//Added later: In case of a polyline (or really, any untextured command),
		// the color for the draw command is defined by the draw command's "texno" or texture number data.
		// this texture number data however is inserted in the wrong parts of the draw command to be the color.
		// So here, we insert it into the correct place in the command table to be the drawn color.
		colorBank += (usedCMDCTRL == VDP1_BASE_CMDCTRL) ? 0 : mesh->attbl[i].texno;

      ssh2SetCommand(ptv[0]->pnt, ptv[1]->pnt, ptv[2]->pnt, ptv[3]->pnt,
		usedCMDCTRL | flip, (VDP1_BASE_PMODE | flags) | pclp, //Reads flip value, mesh enable, and msb bit
		pcoTexDefs[specific_texture].SRCA, colorBank, pcoTexDefs[specific_texture].SIZE, 0, zDepthTgt);
	}
    transVerts[0] += sub_vert_cnt;
	transPolys[0] += sub_poly_cnt;
}
	//////////////////////////////////////////////////////////////
	// Planar polygon subdivision rendering end stub
	//////////////////////////////////////////////////////////////

}


/*

//Saved for posterity, to demonstrate how the subdvision rules are determined.
void	TEMP_process_mesh_for_subdivision_rules(GVPLY * mesh)
{
	
	for(unsigned int i = 0; i < mesh->nbPolygon; i++)
	{
		int * pl_pt0 = mesh->pntbl[mesh->pltbl[i].vertices[0]];
		int * pl_pt1 = mesh->pntbl[mesh->pltbl[i].vertices[1]];
		int * pl_pt2 = mesh->pntbl[mesh->pltbl[i].vertices[2]];
		int * pl_pt3 = mesh->pntbl[mesh->pltbl[i].vertices[3]];
								
		int len01 = unfix_length(pl_pt0, pl_pt1);
		int len12 = unfix_length(pl_pt1, pl_pt2);
		int len23 = unfix_length(pl_pt2, pl_pt3);
		int len30 = unfix_length(pl_pt3, pl_pt0);
		int perimeter = len01 + len12 + len23 + len30;

		int len_w = JO_MAX(len01, len23);//(len01 + len23)>>1; 
		int len_h = JO_MAX(len12, len30);//(len12 + len30)>>1;
	
		subdivision_rules[0] = 0;
		subdivision_rules[1] = 0;
		subdivision_rules[2] = 0;
		subdivision_rules[3] = (perimeter > 1200) ? 1 : 0;
	
			if(len_w >= SUBDIVISION_SCALE)
			{
				subdivision_rules[0] = SUBDIVIDE_W;
			}
			if(len_w >= SUBDIVISION_SCALE<<1)
			{
				subdivision_rules[1] = SUBDIVIDE_W;
			}
			if(len_w >= SUBDIVISION_SCALE<<2)
			{
				subdivision_rules[2] = SUBDIVIDE_W;
			}
			
			if(len_h >= SUBDIVISION_SCALE)
			{
				subdivision_rules[0] = (subdivision_rules[0] == SUBDIVIDE_W) ? SUBDIVIDE_HV : SUBDIVIDE_H;
			}
			if(len_h >= SUBDIVISION_SCALE<<1)
			{
				subdivision_rules[1] = (subdivision_rules[1] == SUBDIVIDE_W) ? SUBDIVIDE_HV : SUBDIVIDE_H;
			}
			if(len_h >= SUBDIVISION_SCALE<<2)
			{
				subdivision_rules[2] = (subdivision_rules[2] == SUBDIVIDE_W) ? SUBDIVIDE_HV : SUBDIVIDE_H;
			}
			unsigned char subrules = subdivision_rules[0];
			subrules |= subdivision_rules[1]<<2;
			subrules |= subdivision_rules[2]<<4;
			subrules |= subdivision_rules[3]<<6;
			mesh->attbl[i].plane_information = subrules;
	}	
	
}
		
*/




