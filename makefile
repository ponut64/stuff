JO_COMPILE_USING_SGL = 1
JO_COMPILE_WITH_BACKUP_MODULE = 1
JO_PSEUDO_SATURN_KAI_SUPPORT = 1
#JO_480i = 1
JO_NTSC = 1
JO_DEBUG = 1
JO_GLOBAL_MEMORY_SIZE_FOR_MALLOC = 65536


SRCS=main.c render.c 2drender.c input.c draw.c ldata.c pcmsys.c mymath.c \
pcmstm.c bounder.c collision.c player_phy.c hmap.c minimap.c tga.c hmap_col.c \
control.c vdp2.c physobjet.c dspm.c mloader.c object_col.c
JO_ENGINE_SRC_DIR=../../jo_engine
COMPILER_DIR=../../Compiler
include $(COMPILER_DIR)/COMMON/jo_engine_makefile

