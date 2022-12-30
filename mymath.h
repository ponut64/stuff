#ifndef __MYMATH_H__
# define __MYMATH_H__

FIXED		fxm(FIXED d1, FIXED d2);
FIXED		fxdot(FIXED * ptA, FIXED * ptB);
FIXED		fxdiv(FIXED dividend, FIXED divisor);

FIXED		approximate_distance(FIXED * p0, FIXED * p1);

int		unfix_length(FIXED Max[XYZ], FIXED Min[XYZ]);
void	segment_to_vector(FIXED * start, FIXED * end, FIXED * out);

void	cpy3(FIXED * src, FIXED * dst);

void	normalize(FIXED * vector_in, FIXED * vector_out);
void	double_normalize(FIXED * vector_in, FIXED * vector_out);
void	accurate_normalize(FIXED * vector_in, FIXED * vector_out);
int		normalize_with_scale(FIXED * vector_in, FIXED * vector_out);
int		line_intersection_function(FIXED * ptA, FIXED * vA, FIXED * ptB, FIXED * vB, FIXED * intersection);
bool	isPointonSegment(FIXED point[XYZ], FIXED start[XYZ], FIXED end[XYZ]);
void	cross_fixed(FIXED * vector1, FIXED * vector2, FIXED * output);
Uint8	solve_domain(FIXED normal[XYZ]);
FIXED	pt_col_plane(FIXED planept[XYZ], FIXED ptoffset[XYZ], FIXED normal[XYZ], FIXED unitNormal[XYZ], FIXED offset[XYZ]);
int		ptalt_plane(FIXED ptreal[XYZ], FIXED normal[XYZ], FIXED offset[XYZ]);
FIXED	realpt_to_plane(FIXED ptreal[XYZ], FIXED normal[XYZ], FIXED offset[XYZ]);
bool	line_hit_plane_here(FIXED p0[XYZ], FIXED p1[XYZ], FIXED centreFace[XYZ], FIXED unitNormal[XYZ], FIXED offset[XYZ], FIXED output[XYZ]);
void	print_from_id(Uint8 normid, Uint8 spotX, Uint8 spotY);

#endif

