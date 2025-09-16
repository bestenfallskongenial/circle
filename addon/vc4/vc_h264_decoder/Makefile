#
# Makefile
#

CIRCLEHOME = ../../..

OBJS = vc_h264_decoder.o

libh264_decoder.a: $(OBJS)
	@echo "  AR    $@"
	@rm -f $@
	@$(AR) cr $@ $(OBJS)

include $(CIRCLEHOME)/Rules.mk

-include $(DEPS)