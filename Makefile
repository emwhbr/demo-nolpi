# ------ Directories

ifeq ($(OUTDIR),)
	OUTDIR := build
endif
OUTDIR := $(abspath $(OUTDIR))

# Include at the *very beginning* of this top level makefile
include rules.mak

# Compiler and linker options
include options.mak

# Extra link flags used for LittlevGL application
LITTLEVGL_EXTRA_LDFLAGS :=
ifeq ($(PCENV),1)
	LITTLEVGL_EXTRA_LDFLAGS := $(shell sdl2-config --libs)
endif

# ------ Include project components make files

include Support/Support.mak
include LittlevGL/LittlevGL.mak
include App1/App1.mak
include App2/App2.mak
include App3/App3.mak
include App4/App4.mak
include NolPi/NolPi.mak

# ------ Project shared libraries

# lib.so/platform
$(call create-lib.so-target, platform, support-platform)

# lib.so/littlevgl
$(call create-lib.so-target, littlevgl, littlevgl)

# ------ Project executables

#######################################################################
#
# bin/app1.exe (C++)
#
libs    :=
modules := app1
deps    := $(call bin-deps,$(libs))
ldflags := $(call bin-ldflags,$(libs))
$(call create-bin-target, app1.exe, $(modules), $(deps), $(ldflags))

#######################################################################
#
# bin/app2.exe (C)
#
libs    :=
modules := app2
deps    := $(call bin-deps,$(libs))
ldflags := $(call bin-ldflags,$(libs))
$(call create-bin-target, app2.exe, $(modules), $(deps), $(ldflags))

#######################################################################
#
# bin/app3.exe (C++, using shared library and a module from this project)
#
libs    := platform
modules := app3 support-xbase
deps    := $(call bin-deps,$(libs))
ldflags := $(call bin-ldflags,$(libs))
$(call create-bin-target, app3.exe, $(modules), $(deps), $(ldflags))

#######################################################################
#
# bin/app4.exe (C++, using LittlevGL as shared library)
#
libs    := littlevgl
modules := app4
deps    := $(call bin-deps,$(libs))
ldflags := $(call bin-ldflags,$(libs)) $(LITTLEVGL_EXTRA_LDFLAGS)
$(call create-bin-target, app4.exe, $(modules), $(deps), $(ldflags))

#######################################################################
#
# bin/nolpi.exe (C++, using LittlevGL as shared library)
#
libs    := littlevgl
modules := nolpi
deps    := $(call bin-deps,$(libs))
ldflags := $(call bin-ldflags,$(libs)) $(LITTLEVGL_EXTRA_LDFLAGS)
$(call create-bin-target, nolpi.exe, $(modules), $(deps), $(ldflags))

# ------ Targets

.PHONY: clean
PHONY_TARGETS += clean
clean:	delete_outdir

.PHONY: delete_outdir
delete_outdir:
	@[ -d $(OUTDIR) ] && rm -rf $(OUTDIR) || true

.PHONY: all
all:	lib/all bin/all

.PHONY: bin/all
PHONY_TARGETS += bin/all
bin/all:	$(call expand-targets-bin)

.PHONY: lib/all
PHONY_TARGETS += lib/all
lib/all:  $(call expand-targets-lib)

$(call expand-objs-all):

# Show all PHONY targets
.PHONY: show
PHONY_TARGETS += show
show:
	@echo "Make targets:"
	@for target in $(sort $(PHONY_TARGETS)); do \
	    echo "  $$target" ;                     \
	done

# ------ Must be *very last* in this top level makefile.

# Trigger rebuild of build products that are older than any makefile.
$(call expand-targets-all) $(call expand-objs-all): $(MAKEFILE_LIST)

# Read dependency info for *existing* .o files
-include $(patsubst %.o,%.d, $(call expand-objs-all))
