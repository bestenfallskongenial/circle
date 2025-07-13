#
# Makefile
#

CIRCLEHOME = ../../..

OBJS = vc_sm_class.o

libvc_sm.a: $(OBJS)
	@echo "  AR    $@"
	@rm -f $@
	@$(AR) cr $@ $(OBJS)

include $(CIRCLEHOME)/Rules.mk

-include $(DEPS)