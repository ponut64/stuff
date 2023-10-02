#JO_COMPILE_USING_SGL = 1
#JO_COMPILE_WITH_BACKUP_MODULE = 1
#JO_PSEUDO_SATURN_KAI_SUPPORT = 1
#JO_480i = 1
#JO_NTSC = 1
#JO_DEBUG = 1
#JO_GLOBAL_MEMORY_SIZE_FOR_MALLOC = 65536


SRCS=main.c gamespeed.c render.c renderAnim.c render2d.c renderSub.c dspm.c object_col.c \
input.c ldata.c pcmsys.c mymath.c collision.c pcmstm.c bounder.c player_phy.c minimap.c \
tga.c hmap_col.c control.c vdp2.c physobjet.c mloader.c menu.c particle.c hmap.c draw.c sound.c
JO_ENGINE_SRC_DIR=../../jo_engine
COMPILER_DIR=../../Compiler
include $(COMPILER_DIR)/COMMON/blank_sgl_makefile

