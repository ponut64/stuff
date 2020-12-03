JO_COMPILE_USING_SGL = 1
JO_COMPILE_WITH_BACKUP_MODULE = 1
JO_PSEUDO_SATURN_KAI_SUPPORT = 1
JO_NTSC = 1
JO_DEBUG = 1
#JO_480p = 1

SRCS=main.c render.c input.c draw.c ldata.c tga.c pcmsys.c mymath.c \
msfs.c bounder.c collision.c player_phy.c hmap.c minimap.c hmap_col.c \
control.c vdp2.c physobjet.c dspm.c mloader.c
JO_ENGINE_SRC_DIR=../../jo_engine
COMPILER_DIR=../../Compiler
include $(COMPILER_DIR)/COMMON/jo_engine_makefile

