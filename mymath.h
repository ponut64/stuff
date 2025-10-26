#pragma once

//I'm not renaming these because Jo Engine *is* where I got them. ... but its pretty basic stuff.

# define JO_ABS(X)   	((X) < 0 ? -(X) : (X))
	
# define JO_MIN(A, B)	(((A) < (B)) ? (A) : (B))
	
# define JO_MAX(A, B)	(((A) > (B)) ? (A) : (B))
	
# define JO_SQUARE(A)	(A * A)
	
# define JO_IS_ODD(A)	((A) & 1)

extern volatile int * DVSR;
extern volatile int * DVDNTH;
extern volatile int * DVDNTL;
extern volatile int * SHADOW;

int			getRandom(void);
void		maintRand(void);

int			fxm(int d1, int d2);
FIXED		fxdot(FIXED * ptA, FIXED * ptB);
FIXED		fxdiv(FIXED dividend, FIXED divisor);
void		SetFixDiv(FIXED dividend, FIXED divisor);
void		SetDiv(int dividend, int divisor);

void		swap_ushort(unsigned short * a, unsigned short * b);

FIXED		approximate_distance(FIXED * p0, FIXED * p1);
FIXED		fxisqrt(FIXED input);

int		unfix_length(FIXED Max[XYZ], FIXED Min[XYZ]);
void	segment_to_vector(FIXED * start, FIXED * end, FIXED * out);

void	fxrotX(int * v_in, int * v_out, int angle);
void	fxrotY(int * v_in, int * v_out, int angle);
void	fxrotZ(int * v_in, int * v_out, int angle);
void	fxRotLocalAxis(int * mtx, int * axis, int angle);
void	fxMatrixMul(int * matrix_1, int * matrix_2, int * output_matrix);
void	fxMatrixApplyTranslation(int * matrix);
void	zero_matrix(int * mtx);
void	copy_matrix(int * mtx_dst, int * mtx_src);

void	cpy3(FIXED * dst, FIXED * src);

void	quick_normalize(FIXED * vector_in, FIXED * vector_out);
void	accurate_normalize(FIXED * vector_in, FIXED * vector_out, int accuracy);
int		normalize_with_scale(FIXED * vector_in, FIXED * vector_out);
int		line_intersection_function(FIXED * ptA, FIXED * vA, FIXED * ptB, FIXED * vB, FIXED * intersection);
void	fxcross(FIXED * vector1, FIXED * vector2, FIXED * output);
Bool	isPointonSegment(FIXED * point, FIXED * start, FIXED * end, int tolerance);

Uint8	solve_domain_y(FIXED normal[XYZ]);
Uint8	solve_domain_x(FIXED normal[XYZ]);

int		fxAtan2(int x_axis, int y_axis);

FIXED	pt_col_plane(FIXED planept[XYZ], FIXED ptoffset[XYZ], FIXED normal[XYZ], FIXED unitNormal[XYZ], FIXED offset[XYZ]);
int		ptalt_plane(FIXED ptreal[XYZ], FIXED normal[XYZ], FIXED offset[XYZ]);
FIXED	realpt_to_plane(FIXED ptreal[XYZ], FIXED normal[XYZ], FIXED offset[XYZ]);
Bool	line_hit_plane_here(FIXED * p0, FIXED * p1, FIXED * point_on_plane, FIXED * unitNormal, FIXED * offset, int tolerance, FIXED * output);
void	ray_to_plane(int * ray_normal, int * ray_pos, int * normal_of_plane, int * point_of_plane, int * hit);
void	print_from_id(Uint8 normid, Uint8 spotX, Uint8 spotY);

void *	align_4(void * ptr);

