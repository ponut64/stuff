#ifndef __COLLISION_H__
#define __COLLISION_H__

#define HIT_TOLERANCE (6553)

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
void	standing_surface_alignment(FIXED * unitNormal, int * output);
bool	player_collide_boxes(_boundBox * stator, _boundBox * mover);
void	player_collision_test_loop(void);

#endif

