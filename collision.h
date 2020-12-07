#ifndef __COLLISION_H__
# define __COLLISION_H__

#include "def.h"
#include "draw.h"
#include "bounder.h"
#include "mymath.h"
#include "physobjet.h"

#define HIT_TOLERANCE (6553)

#define N_Xp (0)
#define N_Xn (1)
#define N_Yp (2)
#define N_Yn (3)
#define N_Zp (4)
#define N_Zn (5)

typedef struct {
	FIXED xp0[XYZ];
	FIXED xp1[XYZ];
	FIXED yp0[XYZ];
	FIXED yp1[XYZ];
	FIXED zp0[XYZ];
	FIXED zp1[XYZ];
} _lineTable;

typedef struct {
	FIXED pXpYpZ[XYZ];
	FIXED pXnYpZ[XYZ];
	FIXED pXnYnZ[XYZ];
	FIXED pXpYnZ[XYZ];
	FIXED nXpYpZ[XYZ];
	FIXED nXnYpZ[XYZ];
	FIXED nXnYnZ[XYZ];
	FIXED nXpYnZ[XYZ];
} _boxPrimitive;

typedef struct {
	int pXpYpZ[XYZ];
	int pXnYpZ[XYZ];
	int pXnYnZ[XYZ];
	int pXpYnZ[XYZ];
	int nXpYpZ[XYZ];
	int nXnYpZ[XYZ];
	int nXnYnZ[XYZ];
	int nXpYnZ[XYZ];
} _16bePrimitive;

Bool 	sort_collide(FIXED pos[XYZ], _boundBox * targetBox, int* nearNormalID, int tolerance);
Bool	simple_collide(FIXED pos[XYZ], _boundBox * targetBox);
void	sort_angle_to_domain(FIXED unitNormal[XYZ], FIXED unitOrient[XYZ], int output[XYZ]);
void	separateAngles(FIXED unitA[XYZ], FIXED plUN[XYZ], int degreeOut[XYZ]);
bool	player_collide_boxes(_boundBox * stator, _boundBox * mover);
void	player_collision_test_loop(void);

#endif

