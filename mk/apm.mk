# find the mk/ directory, which is where this makefile fragment
# lives. (patsubst strips the trailing slash.)
SYSTYPE			:=	$(shell uname)

ifneq ($(SYSTYPE),Linux)
$(error "$(SYSTYPE),now we can only compile this project in Linux")
endif 

#/home/mdk/work/Copter-3.3/ArduCopter
SRC_PATH := $(shell pwd)

#MKDIR := ../mk
MK_DIR := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))

include $(MK_DIR)/environ.mk

# short-circuit build for the help target
include $(MK_DIR)/help.mk

# common makefile components
include $(MK_DIR)/targets.mk
include $(MK_DIR)/sketch_sources.mk

ifneq ($(MAKECMDGOALS),clean)
# board specific includes

ifeq ($(HAL_BOARD),HAL_BOARD_PX4)
include $(MK_DIR)/board_px4.mk
endif

endif
