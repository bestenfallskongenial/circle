#
# Makefile
#

CIRCLEHOME = ../../..

OBJS = vc_h264_parser.o

libh264parser.a: $(OBJS)
	@echo "  AR    $@"
	@rm -f $@
	@$(AR) cr $@ $(OBJS)

include $(CIRCLEHOME)/Rules.mk

-include $(DEPS)