//GCC 9.2 DEPENDENCIES

#include <newlib.h>

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <math.h>
#include <wchar.h>
#include <sys/lock.h>
#include <stdarg.h>

#ifndef _LONG_DOUBLE
#define _LONG_DOUBLE long double
#endif
/* For %La, an exponent of 15 bits occupies the exponent character,
   a sign, and up to 5 digits.  */
# define MAXEXPLEN		7
# define DEFPREC		6
#define	ALT		0x001		/* Alternate form.  */
#define	LADJUST		0x002		/* Left adjustment.  */
#define	ZEROPAD		0x004		/* Zero (as opposed to blank) pad.  */
#define PLUSSGN		0x008		/* Plus sign flag.  */
#define SPACESGN	0x010		/* Space flag.  */
#define	HEXPREFIX	0x020		/* Add 0x or 0X prefix.  */
#define	SHORTINT	0x040		/* Short integer.  */
#define	LONGINT		0x080		/* Long integer.  */
#define	LONGDBL		0x100		/* Long double.  */
/* ifdef _NO_LONGLONG, make QUADINT equivalent to LONGINT, so
   that %lld behaves the same as %ld, not as %d, as expected if:
   sizeof (long long) = sizeof long > sizeof int.  */
#define QUADINT		LONGINT
#define FPT		0x400		/* Floating point number.  */
/* Define as 0, to make SARG and UARG occupy fewer instructions.  */
# define CHARINT	0

int
_printf_float (struct _reent *data,
	       struct _prt_data_t *pdata,
	       FILE * fp,
	       int (*pfunc) (struct _reent *, FILE *, const char *,
			     size_t len), va_list * ap)
{
	return 0;
}	


