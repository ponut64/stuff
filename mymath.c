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


FIXED		fxm(FIXED d1, FIXED d2) //Fixed Point Multiplication
{
	register FIXED rtval;
	asm(
	"dmuls.l %[d1],%[d2];"
	"sts MACH,r1;"		// Store system register [sts] , high of 64-bit register MAC to r1
	"sts MACL,%[out];"	// Low of 64-bit register MAC to the register of output param "out"
	"xtrct r1,%[out];" 	//This whole procress gets the middle 32-bits of 32 * 32 -> (2x32 bit registers)
    :    [out] "=r" (rtval)       		 //OUT
    :    [d1] "r" (d1), [d2] "r" (d2)    //IN
	:		"r1"						//CLOBBERS
	);
	return rtval;
}


FIXED	fxdot(VECTOR ptA, VECTOR ptB) //This can cause illegal instruction execution... I wonder why... fxm does not
{
	register FIXED rtval;
	asm(
		"clrmac;"
		"mac.l @%[ptr1]+,@%[ptr2]+;"
		"mac.l @%[ptr1]+,@%[ptr2]+;"
		"mac.l @%[ptr1]+,@%[ptr2]+;"
		"sts MACH,r1;"
		"sts MACL,%[ox];"
		"xtrct r1,%[ox];"
		: 	[ox] "=r" (rtval)											//OUT
		:	[ptr1] "r" (ptA) , [ptr2] "r" (ptB)							//IN
		:	"r1"														//CLOBBERS
	);
	return rtval;
}


FIXED		ANGtoDEG(FIXED angle){
	return (fxm(angle, 360.0))>>16;
}

bool	chk_matching_sign(FIXED io1[XYZ], FIXED io2[XYZ])
{
	if(io1[X] >= 0 && io2[X] < 0) return false;
	if(io1[X] < 0 && io2[X] >= 0) return false;
	
	if(io1[Y] >= 0 && io2[Y] < 0) return false;
	if(io1[Y] < 0 && io2[Y] >= 0) return false;
	
	if(io1[Z] >= 0 && io2[Z] < 0) return false;
	if(io1[Z] < 0 && io2[Z] >= 0) return false;
	
	return true;
}

int		unfix_dot(FIXED v1[XYZ], FIXED v2[XYZ])
{
	return (int)( ((v1[X]>>16) * (v2[X]>>16)) + ((v1[Y]>>16) * (v2[Y]>>16)) + ((v1[Z]>>16) * (v2[Z]>>16)) );
}

int unfix_length(FIXED Max[XYZ], FIXED Min[XYZ])
{
	return slSquart(JO_SQUARE( (Max[X]>>16) - (Min[X]>>16) ) + JO_SQUARE( (Max[Y]>>16) - (Min[Y]>>16) ) + JO_SQUARE( (Max[Z]>>16) - (Min[Z]>>16) ));
}

int unfix_mag(FIXED vectorX, FIXED vectorY, FIXED vectorZ)
{
	return slSquart(JO_SQUARE(vectorX>>16) + JO_SQUARE(vectorY>>16) + JO_SQUARE(vectorZ>>16));
}

//void	subtract_and_compare_points(POINT p1, POINT p2, POINT subPoint)

void	unfix(FIXED in[XYZ], int out[XYZ])
{
	out[X] = in[X]>>16;
	out[Y] = in[Y]>>16;
	out[Z] = in[Z]>>16;
}

void	segment_to_vector(FIXED start[XYZ], FIXED end[XYZ], FIXED out[XYZ])
{
	out[X] = (start[X] - end[X]);
	out[Y] = (start[Y] - end[Y]);
	out[Z] = (start[Z] - end[Z]);
}


FIXED		fxisqrt(FIXED input){
	
	static FIXED xSR = 0;
	static FIXED pushrsamp = 0;
	static FIXED msb = 0;
	static FIXED shoffset = 0;
	static FIXED yIsqr = 0;
	
	if(input <= 65536){
		return 65536;
	}
	
	xSR = input>>1;
	pushrsamp = input;
	msb = 0;
	shoffset = 0;
	yIsqr = 0;
	
	while(pushrsamp >= 65536){
		pushrsamp >>=1;
		msb++;
	}

	shoffset = (16 - ((msb)>>1));
	yIsqr = 1<<shoffset;
	//y = (y * (98304 - ( ( (x>>1) * ((y * y)>>16 ) )>>16 ) ) )>>16;   x2
	return (fxm(yIsqr, (98304 - fxm(xSR, fxm(yIsqr, yIsqr)))));
}

FIXED		double_fxisqrt(FIXED input){
	
	static FIXED xSR = 0;
	static FIXED pushrsamp = 0;
	static FIXED msb = 0;
	static FIXED shoffset = 0;
	static FIXED yIsqr = 0;
	
	if(input <= 65536){
		return 65536;
	}
	
	xSR = input>>1;
	pushrsamp = input;
	msb = 0;
	shoffset = 0;
	yIsqr = 0;
	
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


void	normalize(FIXED vector_in[XYZ], FIXED vector_out[XYZ])
{
	//Shift inputs rsamp by 8, to prevent overflow.
	static FIXED vmag = 0;
	vmag = fxisqrt(fxm(vector_in[X],vector_in[X]) + fxm(vector_in[Y],vector_in[Y]) + fxm(vector_in[Z],vector_in[Z]));
	vector_out[X] = fxm(vmag, vector_in[X]);
	vector_out[Y] = fxm(vmag, vector_in[Y]);
	vector_out[Z] = fxm(vmag, vector_in[Z]);
}

void	double_normalize(FIXED vector_in[XYZ], FIXED vector_out[XYZ])
{
	//Shift inputs rsamp by 8, to prevent overflow.
	static FIXED vmag = 0;
	vmag = double_fxisqrt(fxm(vector_in[X],vector_in[X]) + fxm(vector_in[Y],vector_in[Y]) + fxm(vector_in[Z],vector_in[Z]));
	vector_out[X] = fxm(vmag, vector_in[X]);
	vector_out[Y] = fxm(vmag, vector_in[Y]);
	vector_out[Z] = fxm(vmag, vector_in[Z]);
}


Bool	isPointonSegment(FIXED point[XYZ], FIXED start[XYZ], FIXED end[XYZ])
{
	FIXED max[XYZ];
	FIXED min[XYZ];
	
	max[X] = JO_MAX(start[X], end[X]);
	max[Y] = JO_MAX(start[Y], end[Y]);
	max[Z] = JO_MAX(start[Z], end[Z]);

	min[X] = JO_MIN(start[X], end[X]);
	min[Y] = JO_MIN(start[Y], end[Y]);
	min[Z] = JO_MIN(start[Z], end[Z]);
	
	if(point[X] >= (min[X] - MATH_TOLERANCE) && point[X] <= (max[X] + MATH_TOLERANCE) &&
		point[Y] >= (min[Y] - MATH_TOLERANCE) && point[Y] <= (max[Y] + MATH_TOLERANCE) &&
		point[Z] >= (min[Z] - MATH_TOLERANCE) && point[Z] <= (max[Z] + MATH_TOLERANCE)){
				return true;
	} else {
		return false;
	}
}

void	project_to_segment(POINT tgt, POINT p1, POINT p2, POINT outPt, VECTOR outV)
{
	
	VECTOR vectorized_pt;
	//Following makes a vector from the left/right centers.
	outV[X] = (p1[X] - p2[X]);
	outV[Y] = (p1[Y] - p2[Y]);
	outV[Z] = (p1[Z] - p2[Z]);
	
	if(JO_ABS(outV[X]) >= (SQUARE_MAX) || JO_ABS(outV[Y]) >= (SQUARE_MAX) || JO_ABS(outV[Z]) >= (SQUARE_MAX))
	{
	//Normalization that covers values >SQUARE_MAX
	int vmag = slSquart( ((outV[X]>>16) * (outV[X]>>16)) + ((outV[Y]>>16) * (outV[Y]>>16)) + ((outV[Z]>>16) * (outV[Z]>>16)) )<<16;
	vmag = slDivFX(vmag, 65536);
	//
	vectorized_pt[X] = fxm(outV[X], vmag);
	vectorized_pt[Y] = fxm(outV[Y], vmag);
	vectorized_pt[Z] = fxm(outV[Z], vmag);
	} else {
	normalize(outV, vectorized_pt);
	}
	//
	//Generatr a scalar for the source vector
	int scaler = slInnerProduct(vectorized_pt, tgt);
	//Scale the vector to project onto ; using scalar as a represenation of how far along the vector it is
	outPt[X] = fxm(scaler, vectorized_pt[X]);
	outPt[Y] = fxm(scaler, vectorized_pt[Y]);
	outPt[Z] = fxm(scaler, vectorized_pt[Z]);
	
	
}

void	cross_fixed(FIXED vector1[XYZ], FIXED vector2[XYZ], FIXED output[XYZ])
{
	output[X] = fxm(vector1[Y], vector2[Z]) - fxm(vector1[Z], vector2[Y]);
	output[Y] = fxm(vector1[Z], vector2[X]) - fxm(vector1[X], vector2[Z]);
	output[Z] = fxm(vector1[X], vector2[Y]) - fxm(vector1[Y], vector2[X]);
}

void	print_from_id(Uint8 normid, Uint8 spotX, Uint8 spotY)
{
	if(normid == 1){
		jo_printf(spotX, spotY, "((X)");
	}
	if(normid == 2){
		jo_printf(spotX, spotY, "(NX)");
	}
	if(normid == 3){
		jo_printf(spotX, spotY, "((Y)");
	}
	if(normid == 4){
		jo_printf(spotX, spotY, "(NY)");
	}
	if(normid == 5){
		jo_printf(spotX, spotY, "((Z)");
	}
	if(normid == 6){
		jo_printf(spotX, spotY, "(NZ)");
	}
}

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
	return slInnerProduct(pFNn, unitNormal);
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

//Why is there a duplicate? I had to make one to handle the data type conversion from FIXED to 16-bit integer without errors if I wanted to skip the first parameter.
FIXED	realpt_to_plane(FIXED ptreal[XYZ], FIXED normal[XYZ], FIXED offset[XYZ])
{
	realNormal[X] = normal[X] + (offset[X]);
	realNormal[Y] = normal[Y] + (offset[Y]);
	realNormal[Z] = normal[Z] + (offset[Z]);
	pFNn[X] = realNormal[X] - ptreal[X];
	pFNn[Y] = realNormal[Y] - ptreal[Y];
	pFNn[Z] = realNormal[Z] - ptreal[Z];
	return slInnerProduct(pFNn, normal);
}

Bool	line_hit_plane_here(FIXED p0[XYZ], FIXED p1[XYZ], FIXED centreFace[XYZ], FIXED unitNormal[XYZ], FIXED offset[XYZ], FIXED output[XYZ])
{
	//Line scalar: This factor determines where the calculated point lands.
	static FIXED line_scalar = 0;
	//Vector-segment: A vector that expresses the length and direction of the segment.
	static FIXED vseg[XYZ] = {0, 0, 0};
	vseg[X] = p0[X] - p1[X];
	vseg[Y] = p0[Y] - p1[Y];
	vseg[Z] = p0[Z] - p1[Z];
	//W is an expression of a vector that is from a point on the segment to a point on the plane. You would normally use addition, but it doesn't change anything.
	//centreFace + the offset finds a real point on the plane as we are using the centreFace which is also a point on the plane, when added to the plane's positional offset. This centreFace isn't actually used as a centreFace here.
	//This is backwards. I know. It doesn't matter.
	static FIXED w[XYZ] = {0, 0, 0};
	w[X] = (centreFace[X] + (offset[X])) - p0[X];
	w[Y] = (centreFace[Y] + (offset[Y])) - p0[Y];
	w[Z] = (centreFace[Z] + (offset[Z])) - p0[Z];
	
	//This finds the unit scalar as a dot product to find a distance scalar between the vector-segment and the centreFace.
	//We divide the scalar of an expression of distance between the plane and the segment and the centreFace.
	//By themselves, these factors are linear. When we divide them, we get a dynamic scalar that responds to proportionate changes in distance between the vector-segment, a point on the plane, and a point on the segment.
	//We use the unit vectors of the normals to prevent overflows of FIXED (16.16 bit) values.
	//Warning about slDivFX: Subscripted value is SECOND. Divisor is FIRST. BACK ASSWARDS!
	line_scalar = slDivFX(slInnerProduct(vseg, unitNormal), slInnerProduct(w, unitNormal));
	if(line_scalar > (1000<<16) || line_scalar < (-1000<<16)){
		return false;
	}
	//p0 is used as an offset (real position) for the final calculation, relative to the line (and not the plane).
	output[X] = (p0[X] + fxm(vseg[X], line_scalar));
	output[Y] = (p0[Y] + fxm(vseg[Y], line_scalar));
	output[Z] = (p0[Z] + fxm(vseg[Z], line_scalar));
//	slPrintFX(line_scalar, slLocate(0, 19));
	
	return isPointonSegment(output, p0, p1);
}



