# find key paths and system type

# Save the system type for later use.
#
SYSTYPE			:=	$(shell uname)

GIT_VERSION := $(shell git rev-parse HEAD | cut -c1-8)
EXTRAFLAGS += -DGIT_VERSION="\"$(GIT_VERSION)\""

# force LANG to C so awk works sanely on MacOS
export LANG=C

#
# Locate the sketch sources based on the initial Makefile's path
#
#/home/mdk/work/Copter-3.3/ArduCopter
#SRCROOT			:=	$(realpath $(dir $(firstword $(MAKEFILE_LIST)))
SRCROOT			:= $(SRC_PATH)

#
# We need to know the location of the sketchbook.  If it hasn't been overridden,
# try the parent of the current directory.  If there is no libraries directory
# there, assume that we are in a library's examples directory and try backing up
# further.
#
#/home/mdk/work/Copter-3.3/
SKETCHBOOK			:= $(shell cd ../ && pwd)


#
# Work out the sketch name from the name of the source directory.
#
#ArduCopter
SKETCH			:=	$(lastword $(subst /, ,$(SRCROOT)))

#
# Work out where we are going to be building things
#
TMPDIR			?=	/tmp

ifneq ($(findstring px4, $(MAKECMDGOALS)),)
# when building px4 we need all sources to be inside the sketchbook directory
# as the NuttX build system relies on it
BUILDROOT		:=	$(SKETCHBOOK)/Build.$(SKETCH)
endif


# handle target based overrides for board type
ifneq ($(findstring px4, $(MAKECMDGOALS)),)
HAL_BOARD = HAL_BOARD_PX4
endif

