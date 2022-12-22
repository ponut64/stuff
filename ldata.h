#ifndef __LDATA_H__
# define __LDATA_H__

#include <jo/jo.h>
#include "def.h"
#include "tga.h"
#include "physobjet.h"

extern bool ldata_ready;

void	process_tga_as_ldata(void * source_data);
void	level_data_basic(void);

#endif

