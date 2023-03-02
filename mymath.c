#include <sl_def.h>
#include "def.h"

#include "mymath.h"

/*
	Deflection
	//d - 2 * DotProduct(d,n) * n
	d = direction vector
	n = normal vector
*/

#define MATH_TOLERANCE (16384)

	static int realNormal[XYZ] = {0, 0, 0};
	static int realpt[XYZ] = {0, 0, 0};
	static FIXED pFNn[XYZ] = {0, 0, 0};


inline FIXED		fxm(FIXED d1, FIXED d2) //Fixed Point Multiplication
{
	register volatile FIXED rtval;
	asm(
	"dmuls.l %[d1],%[d2];"
	"sts MACH,r1;"		// Store system register [sts] , high of 64-bit register MAC to r1
	"sts MACL,%[out];"	// Low of 64-bit register MAC to the register of output param "out"
	"xtrct r1,%[out];" 	//This whole procress gets the middle 32-bits of 32 * 32 -> (2x32 bit registers)
    :    [out] "=r" (rtval)				//OUT
    :    [d1] "r" (d1), [d2] "r" (d2)	//IN
	:		"r1", "mach", "macl"		//CLOBBERS
	);
	return rtval;
}


inline FIXED	fxdot(FIXED * ptA, FIXED * ptB) //Fixed-point dot product
{
	register volatile FIXED rtval;
	asm(
		"clrmac;"
		"mac.l @%[ptr1]+,@%[ptr2]+;"
		"mac.l @%[ptr1]+,@%[ptr2]+;"
		"mac.l @%[ptr1]+,@%[ptr2]+;"
		"sts MACH,r1;"
		"sts MACL,%[ox];"
		"xtrct r1,%[ox];"
		: 	[ox] "=r" (rtval), [ptr1] "+p" (ptA) , [ptr2] "+p" (ptB)	//OUT
		:																//IN
		:	"r1", "mach", "macl"										//CLOBBERS
	);
	return rtval;
}



inline FIXED	fxdiv(FIXED dividend, FIXED divisor) //Fixed-point division
{
	
	volatile int * DVSR = ( int*)0xFFFFFF00;
	volatile int * DVDNTH = ( int*)0xFFFFFF10;
	volatile int * DVDNTL = ( int*)0xFFFFFF14;

	*DVSR = divisor;
	*DVDNTH = (dividend>>16);
	*DVDNTL = (dividend<<16);
	return *DVDNTL;
}

//////////////////////////////////
// Shorthand to turn two points (to represent a segment) into a vector
//////////////////////////////////
inline void	segment_to_vector(FIXED * start, FIXED * end, FIXED * out)
{
	out[X] = (start[X] - end[X]);
	out[Y] = (start[Y] - end[Y]);
	out[Z] = (start[Z] - end[Z]);
}

//////////////////////////////////
// un-fixed point the vectors and get a length out of it
//////////////////////////////////
int unfix_length(FIXED Max[XYZ], FIXED Min[XYZ])
{
	static int vdif[XYZ];
	segment_to_vector(Max, Min, vdif);
	vdif[X]>>=16;
	vdif[Y]>>=16;
	vdif[Z]>>=16;
	return slSquart( (vdif[X] * vdif[X]) + (vdif[Y] * vdif[Y]) + (vdif[Z] * vdif[Z]) );
}

//////////////////////////////////
// Manhattan
//
// Cube root scalar.
// 1.25992104989ish / 3 = 0.2467ish * 65536 = 16168
//
//////////////////////////////////
FIXED		approximate_distance(FIXED * p0, FIXED * p1)
{
	// POINT difference;
	// segment_to_vector(p0, p1, difference);
	// int max = JO_MAX(JO_ABS(difference[X]), JO_MAX(JO_ABS(difference[Y]), JO_ABS(difference[Z])));
	// if(max == JO_ABS(difference[X]))
	// {
		// return JO_ABS(p0[X] - p1[X]) + fxm(JO_ABS(p0[Y] - p1[Y]), 16168) + fxm(JO_ABS(p0[Z] - p1[Z]), 16168);
	// } else if(max == JO_ABS(difference[Y]))
	// {
		// return JO_ABS(p0[Y] - p1[Y]) + fxm(JO_ABS(p0[X] - p1[X]), 16168) + fxm(JO_ABS(p0[Z] - p1[Z]), 16168);
	// } else {
		// return JO_ABS(p0[Z] - p1[Z]) + fxm(JO_ABS(p0[Y] - p1[Y]), 16168) + fxm(JO_ABS(p0[X] - p1[X]), 16168);
	// }
	return (JO_ABS(p0[X] - p1[X]) + JO_ABS(p0[Y] - p1[Y]) + JO_ABS(p0[Z] - p1[Z]));
}


//////////////////////////////////
// "fast inverse square root", but fixed-point
//////////////////////////////////
FIXED		fxisqrt(FIXED input){
	
	FIXED xSR = 0;
	FIXED pushrsamp = 0;
	FIXED msb = 0;
	FIXED shoffset = 0;
	FIXED yIsqr = 0;
	
	if(input <= 65536){
		return 65536;
	}
	
	xSR = input>>1;
	pushrsamp = input;
	
	while(pushrsamp >= 65536){
		pushrsamp >>=1;
		msb++;
	}

	shoffset = (16 - ((msb)>>1));
	yIsqr = 1<<shoffset;
	//y = (y * (98304 - ( ( (x>>1) * ((y * y)>>16 ) )>>16 ) ) )>>16;   x2
	return (fxm(yIsqr, (98304 - fxm(xSR, fxm(yIsqr, yIsqr)))));
}

//////////////////////////////////
// "fast inverse square root x2", but fixed-point
//////////////////////////////////
FIXED		double_fxisqrt(FIXED input){
	
	FIXED xSR = 0;
	FIXED pushrsamp = 0;
	FIXED msb = 0;
	FIXED shoffset = 0;
	FIXED yIsqr = 0;
	
	if(input <= 65536){
		return 65536;
	}
	
	xSR = input>>1;
	pushrsamp = input;
	
	while(pushrsamp >= 65536){
		pushrsamp >>=1;
		msb++;
	}

	shoffset = (16 - ((msb)>>1));
	yIsqr = 1<<shoffset;
	//y = (y * (98304 - ( ( (x>>1) * ((y * y)>>16 ) )>>16 ) ) )>>16;   x2
	yIsqr = (fxm(yIsqr, (98304 - fxm(xSR, fxm(yIsqr, yIsqr)))));
	return (fxm(yIsqr, (98304 - fxm(xSR, fxm(yIsqr, yIsqr)))));
}

void	fxrotX(int * v_in, int * v_out, int angle)
{
	int cosinus = slCos(angle);
	int sinus = slSin(angle);
	
	/*
	Mtx:
	xx xy xz
	yx yy yz
	zx zy zz
	
	Forward X:
	1		1		1
	1		cos*yy	-sin*yz
	1		sin*zy	cos*zz
	
	Transpose X?:
	1		1		1
	1		cos*yy	sin*zy
	1		-sin*yz	cos*zz
	*/
	
	//Vector X unchanged
	v_out[X] = v_in[X];
	v_out[Y] = fxm(cosinus, v_in[Y]) - fxm(sinus, v_in[Z]);
	v_out[Z] = fxm(sinus, v_in[Y]) + fxm(cosinus, v_in[Z]);
}

void	fxrotY(int * v_in, int * v_out, int angle)
{
	int cosinus = slCos(angle);
	int sinus = slSin(angle);
	
	/*
	Mtx:
	xx xy xz
	yx yy yz
	zx zy zz
	
	Forward Y:
	cos*xx	1		sin*xz
	1		1		1
	-sin*zx	1		cos*zz
	
	Transpose Y?:
	cos*xx	1		-sin*zx
	1		1		1
	sin*xz	1		cos*zz
	*/
	
	v_out[X] = fxm(cosinus, v_in[X]) + fxm(sinus, v_in[Z]);
	//Vector Y unchanged
	v_out[Y] = v_in[Y];
	v_out[Z] = fxm(sinus, -v_in[X]) + fxm(cosinus, v_in[Z]);
}

void	fxrotZ(int * v_in, int * v_out, int angle)
{
	int cosinus = slCos(angle);
	int sinus = slSin(angle);
	
	/*
	Mtx:
	xx xy xz
	yx yy yz
	zx zy zz
	
	Forward Z:
	cos*xx	-sin*xy	1
	sin*yx	cos*yy	1
	1		1		1
	
	Transpose Z?:
	cos*xx	sin*yx	1
	-sin*xy	cos*yy	1
	1		1		1
	*/
	
	v_out[X] = fxm(cosinus, v_in[X]) - fxm(sinus, v_in[Y]);
	v_out[Y] = fxm(sinus, v_in[X]) + fxm(cosinus, v_in[Y]);
	v_out[Z] = v_in[Z];
	//Vector Z unchanged
}

/**
INPUTS:
Matrix: the matrix space the axis is relative to
Axis: the axis to pivot about (yaw)
Angle: The angle to rotate about the axis
Special note: AXIS is commuted through the matrix.
For example, (0, 1, 0) will be the matrix' Y axis, (1, 0 ,0) will be the matrix' X axis, (0, 0, 1) will be the matrix' Z axis.
**/
void	fxRotLocalAxis(int * mtx, int * axis, int angle)
{
	int ix[3] = {1<<16, 0, 0};
	int iy[3] = {0, 1<<16, 0};
	int iz[3] = {0, 0, 1<<16};
	
	int cosinus = slCos(angle);
	int sinus = slSin(angle);
		
	ix[X] = fxm(fxm(axis[X], axis[X]), ((1<<16) - cosinus)) + cosinus;
	ix[Y] = fxm(fxm(axis[X], axis[Y]), ((1<<16) - cosinus)) - fxm(axis[Z], sinus); // SGL - Google deviation (+)
	ix[Z] = fxm(fxm(axis[X], axis[Z]), ((1<<16) - cosinus)) + fxm(axis[Y], sinus); // SGL - Google deviation (-)
	
	iy[X] = fxm(fxm(axis[Y], axis[X]), ((1<<16) - cosinus)) + fxm(axis[Z], sinus);
	iy[Y] = fxm(fxm(axis[Y], axis[Y]), ((1<<16) - cosinus)) + cosinus;
	iy[Z] = fxm(fxm(axis[Y], axis[Z]), ((1<<16) - cosinus)) - fxm(axis[X], sinus);
	
	iz[X] = fxm(fxm(axis[Z], axis[X]), ((1<<16) - cosinus)) - fxm(axis[Y], sinus);
	iz[Y] = fxm(fxm(axis[Z], axis[Y]), ((1<<16) - cosinus)) + fxm(axis[X], sinus);
	iz[Z] = fxm(fxm(axis[Z], axis[Z]), ((1<<16) - cosinus)) + cosinus;
	
	int trapX[3] = {mtx[0], mtx[3], mtx[6]};
	int trapY[3] = {mtx[1], mtx[4], mtx[7]};
	int trapZ[3] = {mtx[2], mtx[5], mtx[8]};
	
	mtx[0] = fxdot(ix, trapX);
	mtx[1] = fxdot(ix, trapY);
	mtx[2] = fxdot(ix, trapZ);
	mtx[3] = fxdot(iy, trapX);
	mtx[4] = fxdot(iy, trapY);
	mtx[5] = fxdot(iy, trapZ);
	mtx[6] = fxdot(iz, trapX);
	mtx[7] = fxdot(iz, trapY);
	mtx[8] = fxdot(iz, trapZ);
	
}

void	fxMatrixMul(int * matrix_1, int * matrix_2, int * output_matrix)
{
	int * ix1 = matrix_1;
	int * iy1 = &matrix_1[3];
	int * iz1 = &matrix_1[6];
	
	int * ix2 = matrix_2;
	int * iy2 = &matrix_2[3];
	int * iz2 = &matrix_2[6];
	
	int trapX[3] = {ix2[X], ix2[X], ix2[X]};
	int trapY[3] = {iy2[Y], iy2[Y], iy2[Y]};
	int trapZ[3] = {iz2[Z], iz2[Z], iz2[Z]};
	
	output_matrix[0] = fxdot(ix1, trapX);
	output_matrix[1] = fxdot(ix1, trapY);
	output_matrix[2] = fxdot(ix1, trapZ);
	
	output_matrix[3] = fxdot(iy1, trapX);
	output_matrix[4] = fxdot(iy1, trapY);
	output_matrix[5] = fxdot(iy1, trapZ);
	
	output_matrix[6] = fxdot(iz1, trapX);
	output_matrix[7] = fxdot(iz1, trapY);
	output_matrix[9] = fxdot(iz1, trapZ);
}

void	zero_matrix(int * mtx)
{
	for(int i = 0; i < 9; i++)
	{
		mtx[i] = 0;
	}
}

void	cpy3(FIXED * dst, FIXED * src)
{
	dst[X] = src[X];
	dst[Y] = src[Y];
	dst[Z] = src[Z];
}

void	normalize(FIXED * vector_in, FIXED * vector_out)
{
	//Shift inputs rsamp by 8, to prevent overflow.
	static FIXED vmag = 0;
	vmag = fxisqrt(fxm(vector_in[X],vector_in[X]) + fxm(vector_in[Y],vector_in[Y]) + fxm(vector_in[Z],vector_in[Z]));
	vector_out[X] = fxm(vmag, vector_in[X]);
	vector_out[Y] = fxm(vmag, vector_in[Y]);
	vector_out[Z] = fxm(vmag, vector_in[Z]);
}

void	double_normalize(FIXED * vector_in, FIXED * vector_out)
{
	//Shift inputs rsamp by 8, to prevent overflow.
	static FIXED vmag = 0;
	vmag = double_fxisqrt(fxm(vector_in[X],vector_in[X]) + fxm(vector_in[Y],vector_in[Y]) + fxm(vector_in[Z],vector_in[Z]));
	vector_out[X] = fxm(vmag, vector_in[X]);
	vector_out[Y] = fxm(vmag, vector_in[Y]);
	vector_out[Z] = fxm(vmag, vector_in[Z]);
}

void	accurate_normalize(FIXED * vector_in, FIXED * vector_out)
{
	//Shift inputs rsamp by 8, to prevent overflow.
	static FIXED vmag = 0;
	vmag = slSquartFX(fxm(vector_in[X],vector_in[X]) + fxm(vector_in[Y],vector_in[Y]) + fxm(vector_in[Z],vector_in[Z]));
	vmag = fxdiv(1<<16, vmag);
	vector_out[X] = fxm(vmag, vector_in[X]);
	vector_out[Y] = fxm(vmag, vector_in[Y]);
	vector_out[Z] = fxm(vmag, vector_in[Z]);
}

int		normalize_with_scale(FIXED * vector_in, FIXED * vector_out)
{
	//Shift inputs rsamp by 8, to prevent overflow.
	static FIXED vmag = 0;
	vmag = slSquartFX(fxm(vector_in[X],vector_in[X]) + fxm(vector_in[Y],vector_in[Y]) + fxm(vector_in[Z],vector_in[Z]));
	int scale = vmag;
	vmag = fxdiv(1<<16, vmag);
	vector_out[X] = fxm(vmag, vector_in[X]);
	vector_out[Y] = fxm(vmag, vector_in[Y]);
	vector_out[Z] = fxm(vmag, vector_in[Z]);
	return scale;
}

void	fxcross(FIXED * vector1, FIXED * vector2, FIXED * output)
{
	output[X] = fxm(vector1[Y], vector2[Z]) - fxm(vector1[Z], vector2[Y]);
	output[Y] = fxm(vector1[Z], vector2[X]) - fxm(vector1[X], vector2[Z]);
	output[Z] = fxm(vector1[X], vector2[Y]) - fxm(vector1[Y], vector2[X]);
}


//////////////////////////////////
// Checks if "point" is between "start" and "end".
//////////////////////////////////
Bool	isPointonSegment(FIXED point[XYZ], FIXED start[XYZ], FIXED end[XYZ], int tolerance)
{
	FIXED max[XYZ];
	FIXED min[XYZ];
	
	max[X] = JO_MAX(start[X], end[X]);
	max[Y] = JO_MAX(start[Y], end[Y]);
	max[Z] = JO_MAX(start[Z], end[Z]);

	min[X] = JO_MIN(start[X], end[X]);
	min[Y] = JO_MIN(start[Y], end[Y]);
	min[Z] = JO_MIN(start[Z], end[Z]);
	
	if(point[X] >= (min[X] - tolerance) && point[X] <= (max[X] + tolerance) &&
		point[Y] >= (min[Y] - tolerance) && point[Y] <= (max[Y] + tolerance) &&
		point[Z] >= (min[Z] - tolerance) && point[Z] <= (max[Z] + tolerance)){
				return true;
	} else {
		return false;
	}
}

	//output[X] = fxm(vector1[Y], vector2[Z]) - fxm(vector1[Z], vector2[Y]);
	//output[Y] = fxm(vector1[Z], vector2[X]) - fxm(vector1[X], vector2[Z]);
	//output[Z] = fxm(vector1[X], vector2[Y]) - fxm(vector1[Y], vector2[X]);

//////////////////////////////////
//
//	Provides the intersection or nearest-coincident point of two lines.
//	These two lines have parameters of a point (pt_a / pt_b) and a vector (v_a / v_b).
//	Inputs vA and vB are *not* unit vectors. At least one of them can be, but both **CANNOT** be unit vectors.
//	The parameter "intersection" is the output.
//	Another good hint is, this function returns the scale from point A to the intersection point.
//	If you made vA out of a segment and ptA is one of the points on the segment you used,
//	if the scale is less than one, the intersection point is on the segment.
//
//////////////////////////////////
int	line_intersection_function(FIXED * ptA, FIXED * vA, FIXED * ptB, FIXED * vB, FIXED * intersection)
{
	/*
	scale_to_point = dot(cross(dc,db),cross(da,db)) / dot(cross(da,db),cross(da,db));
	da = vector for line A
	db = vector for line B
	dc = difference of A and B (new vector)
	We subtract, because I don't know that's just what put it in the right spot,
	fuck you, do not pass go, do not collect 200 rubles
	*/
	
	VECTOR vC = {ptA[X] - ptB[X], ptA[Y] - ptB[Y], ptA[Z] - ptB[Z]};
	VECTOR crossCB;
	VECTOR crossAB;
	fxcross(vC, vB, crossCB);
	fxcross(vA, vB, crossAB);
	int sclA = fxdiv(slInnerProduct(crossCB, crossAB), slInnerProduct(crossAB, crossAB));
	
	intersection[X] = ptA[X] - fxm(sclA, vA[X]);
	intersection[Y] = ptA[Y] - fxm(sclA, vA[Y]);
	intersection[Z] = ptA[Z] - fxm(sclA, vA[Z]);
	return sclA;
}

void	print_from_id(Uint8 normid, Uint8 spotX, Uint8 spotY)
{
	if(normid == N_Xp){
		slPrint("((X)", slLocate(spotX, spotY));
	}
	if(normid == N_Xn){
		slPrint("(NX)", slLocate(spotX, spotY));
	}
	if(normid == N_Yp){
		slPrint("((Y)", slLocate(spotX, spotY));
	}
	if(normid == N_Yn){
		slPrint("(NY)", slLocate(spotX, spotY));
	}
	if(normid == N_Zp){
		slPrint("((Z)", slLocate(spotX, spotY));
	}
	if(normid == N_Zn){
		slPrint("(NZ)", slLocate(spotX, spotY));
	}
}

//////////////////////////////////
//A helper function which checks the X and Z signs of a vector to find its domain.
//////////////////////////////////
Uint8	solve_domain(FIXED normal[XYZ]){
	if(normal[X] >= 0 && normal[Z] >= 0){
		//PP
		return 1;
	} else if(normal[X] >= 0 && normal[Z] < 0){
		//PN
		return 2;
	} else if(normal[X] < 0 && normal[Z] >= 0){
		//NP
		return 3;
	} else if(normal[X] < 0 && normal[Z] < 0){
		//NN
		return 4;
	};
	/*
	3	-	1
	4	-	2
	*/
	return 0;
}

FIXED	pt_col_plane(FIXED planept[XYZ], FIXED ptoffset[XYZ], FIXED normal[XYZ], FIXED unitNormal[XYZ], FIXED offset[XYZ])
{
	//Using a NORMAL OF A PLANE which is also a POINT ON THE PLANE and checking IF A POINT IS ON THAT PLANE
	//the REAL POSITION of the normal, which is also a POINT ON THE PLANE, needs an actual position. WE FIND IT HERE.
	realNormal[X] = normal[X] + (offset[X]);
	realNormal[Y] = normal[Y] + (offset[Y]);
	realNormal[Z] = normal[Z] + (offset[Z]);
	realpt[X] = planept[X] + (ptoffset[X]);
	realpt[Y] = planept[Y] + (ptoffset[Y]);
	realpt[Z] = planept[Z] + (ptoffset[Z]);
	//the DIFFERENCE between a POSSIBLE POINT ON THE PLANE, and a KNOWN POINT ON THE PLANE, must use the REAL POSITION of the NORMAL POINT.
	pFNn[X] = realNormal[X] - realpt[X];
	pFNn[Y] = realNormal[Y] - realpt[Y];
	pFNn[Z] = realNormal[Z] - realpt[Z];
	//The NORMAL of the plane has NO REAL POSITION. it is FROM ORIGIN. We use the normal here.
	//If the dot product here is zero, the point lies on the plane.
	return fxdot(pFNn, unitNormal);
}

int	ptalt_plane(FIXED ptreal[XYZ], FIXED normal[XYZ], FIXED offset[XYZ]) //Shifts down the pFNn to suppress overflows
{
	realNormal[X] = normal[X] + (offset[X]);
	realNormal[Y] = normal[Y] + (offset[Y]);
	realNormal[Z] = normal[Z] + (offset[Z]);
	pFNn[X] = (realNormal[X] - ptreal[X])>>8;
	pFNn[Y] = (realNormal[Y] - ptreal[Y])>>8;
	pFNn[Z] = (realNormal[Z] - ptreal[Z])>>8;
	return fxdot(pFNn, normal);
}

// Input 'ptreal' - a real-space / world-space point, tested against a plane
// Input 'normal' - Treated as both a point on the plane, and a vector normal to the plane
// Input 'offset' - The location of the plane (center, one point, that point)
// output - a dot product, where values approaching zero indicate closer and closer contact with the plane.
// Zero is perfect contact. You can do experiments to find out if you want less than or greater than zero to be "passed through".
// Typically, storing the value of this test for the previous frame is done, and if the values for the last frame and this frame differ,
// the plane is considered to have been passed (collided).
FIXED	realpt_to_plane(FIXED ptreal[XYZ], FIXED normal[XYZ], FIXED offset[XYZ])
{
	realNormal[X] = normal[X] + (offset[X]);
	realNormal[Y] = normal[Y] + (offset[Y]);
	realNormal[Z] = normal[Z] + (offset[Z]);
	pFNn[X] = realNormal[X] - ptreal[X];
	pFNn[Y] = realNormal[Y] - ptreal[Y];
	pFNn[Z] = realNormal[Z] - ptreal[Z];
	return fxdot(pFNn, normal);
}

//////////////////////////////////
// Line-to-plane projection function
// Line: p0->p1
// point_on_plane : a point on the plane
// unitNormal : the unit vector normal of the plane
// offset : world-space position of the point_on_plane. If point_on_plane is already, substitute with "zPt". (do not leave blank)
// output : the point at which the line intersects the plane
// return value : whether or not the output point is between p0 and p1
//////////////////////////////////
Bool	line_hit_plane_here(FIXED * p0, FIXED * p1, FIXED * point_on_plane, FIXED * unitNormal, FIXED * offset, int tolerance, FIXED * output)
{

	FIXED line_scalar = 0;
	FIXED vector_of_line[XYZ] = {0, 0, 0};
	FIXED vector_to_plane[XYZ] = {0, 0, 0};
	
	vector_of_line[X] = p0[X] - p1[X];
	vector_of_line[Y] = p0[Y] - p1[Y];
	vector_of_line[Z] = p0[Z] - p1[Z];

	vector_to_plane[X] = (point_on_plane[X] + (offset[X])) - p0[X];
	vector_to_plane[Y] = (point_on_plane[Y] + (offset[Y])) - p0[Y];
	vector_to_plane[Z] = (point_on_plane[Z] + (offset[Z])) - p0[Z];

	line_scalar = fxdiv(fxdot(vector_to_plane, unitNormal), fxdot(vector_of_line, unitNormal));
	if(line_scalar > (1000<<16) || line_scalar < -(1000<<16)){
		return false;
	}
	
	output[X] = (p0[X] + fxm(vector_of_line[X], line_scalar));
	output[Y] = (p0[Y] + fxm(vector_of_line[Y], line_scalar));
	output[Z] = (p0[Z] + fxm(vector_of_line[Z], line_scalar));

	return isPointonSegment(output, p0, p1, tolerance);
}

