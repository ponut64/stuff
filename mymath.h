#ifndef __MYMATH_H__
# define __MYMATH_H__

#include "jo/jo.h"
#include "def.h"

FIXED		fxm(FIXED d1, FIXED d2);
FIXED		fxdot(VECTOR ptA, VECTOR ptB);
FIXED		fxdiv(FIXED dividend, FIXED divisor);

FIXED	ANGtoDEG(FIXED angle);
bool	chk_matching_sign(FIXED io1[XYZ], FIXED io2[XYZ]);
int		unfix_dot(FIXED v1[XYZ], FIXED v2[XYZ]);
int		unfix_length(FIXED Max[XYZ], FIXED Min[XYZ]);
int		unfix_mag(FIXED vectorX, FIXED vectorY, FIXED vectorZ);
void	avg_pts(FIXED p1[XYZ], FIXED p2[XYZ], FIXED output[XYZ]);
void	unfix(FIXED in[XYZ], int out[XYZ]);
void	segment_to_vector(FIXED start[XYZ], FIXED end[XYZ], FIXED out[XYZ]);
void	normalize(FIXED vector_in[XYZ], FIXED vector_out[XYZ]);
void	double_normalize(FIXED vector_in[XYZ], FIXED vector_out[XYZ]);
void	project_to_segment(POINT tgt, POINT p1, POINT p2, POINT outPt, VECTOR outV);
Bool	isPointonSegment(FIXED point[XYZ], FIXED start[XYZ], FIXED end[XYZ]);
void	cross_fixed(FIXED vector1[XYZ], FIXED vector2[XYZ], FIXED output[XYZ]);
Uint8	solve_domain(FIXED normal[XYZ]);
FIXED	pt_col_plane(FIXED planept[XYZ], FIXED ptoffset[XYZ], FIXED normal[XYZ], FIXED unitNormal[XYZ], FIXED offset[XYZ]);
int	ptalt_plane(FIXED ptreal[XYZ], FIXED normal[XYZ], FIXED offset[XYZ]);
FIXED	realpt_to_plane(FIXED ptreal[XYZ], FIXED normal[XYZ], FIXED offset[XYZ]);
Bool	line_hit_plane_here(FIXED p0[XYZ], FIXED p1[XYZ], FIXED centreFace[XYZ], FIXED unitNormal[XYZ], FIXED offset[XYZ], FIXED output[XYZ]);
void	print_from_id(Uint8 normid, Uint8 spotX, Uint8 spotY);

#endif

