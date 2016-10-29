default: help

# convenient targets for our supported boards

# cope with OBC targets
%-obc: EXTRAFLAGS += "-DOBC_FAILSAFE=ENABLED "

# support debug build
%-debug: OPTFLAGS = -g -O0

# cope with -nologging
%-nologging: EXTRAFLAGS += "-DLOGGING_ENABLED=DISABLED "

# cope with copter and hil targets
FRAMES = quad tri hexa y6 octa octa-quad heli single coax obc nologging
BOARDS = apm1 apm2 apm2beta apm1-1280 px4 px4-v1 px4-v2 sitl flymaple linux vrbrain vrbrain-v40 vrbrain-v45 vrbrainv-50 vrbrain-v51 vrbrain-v52 vrubrain-v51 vrubrain-v52 vrhero-v10 erle pxf navio bbbmini

define frame_template
$(1)-$(2) : EXTRAFLAGS += "-DFRAME_CONFIG=$(shell echo $(2) | tr a-z A-Z | sed s/-/_/g)_FRAME "
$(1)-$(2) : $(1)
$(1)-$(2)-hil : $(1)-$(2)
$(1)-$(2)-debug : $(1)-$(2)
$(1)-$(2)-hilsensors : $(1)-$(2)
$(1)-$(2)-upload : $(1)-$(2)
$(1)-$(2)-upload : $(1)-upload
endef

define board_template
$(1)-hil : $(1)
$(1)-debug : $(1)
$(1)-hilsensors : $(1)
endef

USED_BOARDS := $(foreach board,$(BOARDS), $(findstring $(board), $(MAKECMDGOALS)))
USED_FRAMES := $(foreach frame,$(FRAMES), $(findstring $(frame), $(MAKECMDGOALS)))
#$(warning $(USED_BOARDS))
#$(warning $(USED_FRAMES))
# generate targets of the form BOARD-FRAME and BOARD-FRAME-HIL
$(foreach board,$(USED_BOARDS),$(eval $(call board_template,$(board))))
$(foreach board,$(USED_BOARDS),$(foreach frame,$(USED_FRAMES),$(eval $(call frame_template,$(board),$(frame)))))


.PHONY: etags
etags:
	cd .. && etags -f ArduCopter/TAGS --lang=c++ $$(git ls-files ArduCopter libraries)
	cd .. && etags -f ArduPlane/TAGS --lang=c++ $$(git ls-files ArduPlane libraries)
	cd .. && etags -f APMrover2/TAGS --lang=c++ $$(git ls-files APMrover2 libraries)

clean:
	@rm -fr $(BUILDROOT)

include $(MK_DIR)/modules.mk
