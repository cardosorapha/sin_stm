######################################################################
#  Project
######################################################################

BINARY			= dac_tim
SRCFILES		= dac_tim.c time_stm32.c
BUILDPATH		= ./build
OBJPATH			= ./objs
SRCPATH			= ./src
DEBUGPATH		= ./debug

all: elf images

include ../../Makefile.mk

# End
