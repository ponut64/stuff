JO_COMPILE_USING_SGL = 1
JO_COMPILE_WITH_BACKUP_MODULE = 1
#JO_COMPILE_WITH_3D_MODULE = 1
JO_COMPILE_WITH_DUAL_CPU = 1
JO_PSEUDO_SATURN_KAI_SUPPORT = 1
JO_NTSC = 1
JO_MAX_SPRITE = 4
#JO_DEBUG = 1
#JO_480p = 1

JO_GLOBAL_MEMORY_SIZE_FOR_MALLOC = 460800
SRCS=main.c render.c input.c draw.c ldata.c timer.c tga.c pcmsys.c mymath.c \
msfs.c bounder.c collision.c player_phy.c hmap.c minimap.c hmap_col.c \
control.c vdp2.c physobjet.c dspm.c ZT/ZT_LOAD_MODEL.c ZT/ZT_TOOLS.c ZT/ZT_CD.c 
JO_ENGINE_SRC_DIR=./jo_engine
COMPILER_DIR=./Compiler
include $(COMPILER_DIR)/COMMON/jo_engine_makefile
