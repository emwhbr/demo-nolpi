# Include this file the very beginning of the top level makefile

# Reset flag variables
CFLAGS   :=
CPPFLAGS :=
CXXFLAGS :=
LDFLAGS  :=

# List maintained for 'make show'.
PHONY_TARGETS :=

OUTDIR_BIN  := $(OUTDIR)/bin
OUTDIR_LIB  := $(OUTDIR)/lib
OUTDIR_FPIC := $(OUTDIR)/_fpic

###############################################################################
# Set shell to be used to avoid possibly different behavior in different
# environments because of the shell used.
SHELL = /bin/sh

# Disable all built-in rules, specify rules explicitly to avoid surprises.
.SUFFIXES:

# Enable secondary expansion to be able to create object file output directories
# based on sentinel target file (.keep).
# See https://www.gnu.org/software/make/manual/html_node/Secondary-Expansion.html
.SECONDEXPANSION:

# Assume there will be a phony all target and make it the default.
.PHONY: all
.DEFAULT_GOAL := all
PHONY_TARGETS += all

###############################################################################
# Basic commands.
RM := rm -f
CP := cp
MV := mv

# For escaping comma using $(,) to pass it as argument to GNU Make functions.
, := ,

###############################################################################
# Command verbosity support.
# Commands can be prefixed with $(Q) to make them honor the verbosity level.

ifeq ($(V),1)
	Q         :=
	Q_compile :=
	Q_link    :=
	Q_ar      :=
else
	Q         := @
	Q_compile = @echo "Compiling $<\n       to $(@:$(OUTDIR)/%=%)" ;
	Q_link    = @echo "Linking $(@:$(OUTDIR)/%=%)" ;
	Q_ar      = @echo "Creating archive $(@:$(OUTDIR)/%=%)" ;
endif

###############################################################################
# Rule that will create output directories on the fly when a target depends on it.
# It's not possible to create a dependency on the directory itself as its
# timestamp will change when each file is written to it. The .keep file is a
# workaround to get something inside the output directoy that marks it as
# created.
.PRECIOUS: %/.keep
%/.keep:
	@mkdir -p $(@D)
	@touch $@

# Let target recipes depend on $$(KEEPFILE) to automatically create output directories.
$(OUTDIR)/%: KEEPFILE = $(@D)/.keep

###############################################################################
# Function that filter out makefiles and other unwanted files from prerequisites ($^).
# Makefiles are included as prerequisites of all build products in order to
# trigger rebuild when makefiles are changed. This function can be used instead
# of $^ to filter out unwanted files.
prereq = $(filter-out $(MAKEFILE_LIST) %/.keep,$^)

###############################################################################
# Function that filters out object files from prerequisites ($^).
#
prereq-obj = $(filter %.o,$^)

###############################################################################
# Function that compiles a C source file and generates dependency info.
#
# Arg1: Optional extra compilation parameters
#
cc-compile = $(Q_compile) $(strip $(CC) $(CFLAGS) $(CPPFLAGS) $1) \
	-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -c -o $@ $<

###############################################################################
# Function that compiles a C++ source file and generates dependency info.
#
# Arg1: Optional extra compilation parameters
#
cxx-compile = $(Q_compile) $(strip $(CXX) $(CXXFLAGS) $(CPPFLAGS) $1) \
	-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -c -o $@ $<

###############################################################################
# Function that links an executable of shared library using the C++ compiler.
#
# Arg1: Optional extra link parameters
#
cxx-link = $(Q_link) $(strip $(CXX) $(call prereq-obj) -o $@ $(LDFLAGS) $1)

###############################################################################
# Function that creates an archive (static lib).
#
ar-create = $(Q_ar) $(RM) $@ && $(AR) rsc $@ $(call prereq-obj)

###############################################################################
# Helper for define-srcs_
#
# Arg1: Module name
# Arg2: Subdirectory of sources relative project root
#
define define-srcs-rules_
$$(OUTDIR)/$2/%.c.$1.o: $2/%.c $$$$(KEEPFILE)
	$$(call cc-compile)
$$(OUTDIR)/$2/%.cpp.$1.o: $2/%.cpp $$$$(KEEPFILE)
	$$(call cxx-compile)
$$(OUTDIR_FPIC)/$2/%.c.$1.o: $2/%.c $$$$(KEEPFILE)
	$$(call cc-compile, -fpic)
$$(OUTDIR_FPIC)/$2/%.cpp.$1.o: $2/%.cpp $$$$(KEEPFILE)
	$$(call cxx-compile, -fpic)
endef

###############################################################################
# Helper for define-srcs
#
# Arg1: Module name
# Arg2: Subdirectory of sources relative project root
# Arg3: List with source files
#
define define-srcs_
OBJS_$(1) := $(foreach src,$3,$(OUTDIR)/$2/$(src).$1.o)
OBJS_FPIC_$(1) := $(foreach src,$3,$(OUTDIR_FPIC)/$2/$(src).$1.o)
$(call define-srcs-rules_,$1,$2)
endef

###############################################################################
# Function that create OBJS_<name> variables for a collection of source files
# and associated rules to compile them.
#
# Arg1: Module name
# Arg2: Subdirectory of sources relative project root
# Arg3: List with source files
#
define-srcs = $(eval $(call define-srcs_,$(strip $1),$(strip $2),$3))

###############################################################################
# Helper for add-srcs
#
# Arg1: Module name
# Arg2: Subdirectory of sources relative project root
# Arg3: List with source files
#
define add-srcs_
OBJS_$(1) += $(foreach src,$3,$(OUTDIR)/$2/$(src).$1.o)
OBJS_FPIC_$(1) += $(foreach src,$3,$(OUTDIR_FPIC)/$2/$(src).$1.o)
endef

###############################################################################
# Function that adds additional sources to an already defined module name.
#
# NOTE: This function does not define the compilation rules.
#       It relies on define-srcs to have done that.
#
# Arg1: Module name
# Arg2: Subdirectory of sources relative project root
# Arg3: List with source files
#
add-srcs = $(eval $(call add-srcs_,$(strip $1),$(strip $2),$3))

###############################################################################
# Helper for concat-objs
#
define concat-objs_
OBJS_$(1)      := $(foreach v,$2,$(OBJS_$(v)))
OBJS_FPIC_$(1) := $(foreach v,$2,$(OBJS_FPIC_$(v)))
endef

###############################################################################
# Function that create OBJS_ variables by concatenating one or more other OBJS_ variables.
# This can be used if parts of a software component needs to be compiled with
# different compiler flags but in the end one want to create a single list with
# object files that one can refer to.
#
# Arg1: Module name
# Arg2: List with module names whose object files should become part of Arg1
#
concat-objs = $(eval $(call concat-objs_,$(strip $1),$2))

###############################################################################
# Function that expand all OBJS_ variables for the specified software modules.
#
# Arg1: List with module names
#
expand-objs = $(foreach v,$1,$(OBJS_$(v)) $(OBJS_FPIC_$(v)))

###############################################################################
# Function that expand all OBJS_ prefixed variables.
#
expand-objs-all = $(sort $(foreach v,$(filter OBJS_%,$(.VARIABLES)),$($(v))))

###############################################################################
# Function that applies CPPFLAGS to the specified software modules.
# This can be done directly using expand-objs but since it's such a common
# operation this function does it in a slightly more compact way.
#
# Arg1: List with module names
# Arg2: List with CPPFLAGS
#
apply-cppflags = $(eval $(call expand-objs,$1): CPPFLAGS := $$(CPPFLAGS) $2)

###############################################################################
# Helper for create-bin-target
#
define create-bin-target_
TARGET_BIN_$(1) := $(OUTDIR_BIN)/$(1)
$(OUTDIR_BIN)/$(1): extra_ldflags := $(4)
$(OUTDIR_BIN)/$(1): $$$$(KEEPFILE) $(foreach v,$(2),$$$$(OBJS_$(v))) $(3)
	$$(call cxx-link,$$(extra_ldflags))

.PHONY: bin/$(1)
PHONY_TARGETS += bin/$(1)
bin/$(1): $(OUTDIR_BIN)/$(1)
endef

###############################################################################
# Create a binary target and the make rule to compile it.
#
# Arg1: Target name
# Arg2: List of object variables to expand
# Arg3: Optional prerequisites, e.g. dependencies to non-object file targets or
#       fully qualified object files.
# Arg4: Optional extra LDFLAGS
#
create-bin-target = $(eval $(call create-bin-target_,$(strip $1),$2,$3,$4))

###############################################################################
# Helper for create-lib.a-target
#
define create-lib.a-target_
TARGET_LIB_$(1) := $(OUTDIR_LIB)/lib$(1).a
$(OUTDIR_LIB)/lib$(1).a: $$$$(KEEPFILE) $(foreach v,$(2),$$$$(OBJS_$(v))) $(3)
	$$(call ar-create)

.PHONY: lib.a/$(1)
PHONY_TARGETS += lib.a/$(1)
lib.a/$(1): $(OUTDIR_LIB)/lib$(1).a
endef

###############################################################################
# Create a static library target and the make rule to compile it.
#
# Arg1: Target name
# Arg2: List of object variables to expand
# Arg3: Optional prerequisites, e.g. dependencies to non-object file targets or
#       fully qualified object files.
#
create-lib.a-target = $(eval $(call create-lib.a-target_,$(strip $1),$2,$3))

###############################################################################
# Helper for create-lib.so-target
#
# Arg1: Target name
# Arg2: List of object variables to expand
# Arg3: Optional prerequisites, e.g. dependencies to non-object file targets or
#       fully qualified object files.
# Arg4: Optional extra LDFLAGS
#
define create-lib.so-target_
TARGET_LIB_$(1) := $(OUTDIR_LIB)/lib$(1).so.1
$$(TARGET_LIB_$(1)): extra_ldflags := $4
$$(TARGET_LIB_$(1)): $$$$(KEEPFILE) $(foreach v,$2,$$$$(OBJS_FPIC_$(v))) $3
	$$(call cxx-link,-shared -Wl$$(,)-export-dynamic -Wl$$(,)-soname$$(,)lib$(1).so.1 $$(extra_ldflags))
	@ln -sf $$(notdir $$@) $(OUTDIR_LIB)/lib$(1).so

.PHONY: lib.so/$1
PHONY_TARGETS += lib.so/$1
lib.so/$1: $$(TARGET_LIB_$(1))
endef

###############################################################################
# Create a shared library target and the make rule to compile it.
#
# Arg1: Target name
# Arg2: List of object variables to expand
# Arg3: Optional prerequisites, e.g. dependencies to non-object file targets or
#       fully qualified object files.
# Arg4: Optional extra LDFLAGS
#
create-lib.so-target = $(eval $(call create-lib.so-target_,$(strip $1),$2,$3,$4))

###############################################################################
# Function that expands all variables prefixed with TARGET_BIN_
#
expand-targets-bin = $(foreach v,$(filter TARGET_BIN_%,$(.VARIABLES)),$($(v)))

###############################################################################
# Function that expands all variables prefixed with TARGET_LIB_
#
expand-targets-lib = $(foreach v,$(filter TARGET_LIB_%,$(.VARIABLES)),$($(v)))

###############################################################################
# Function that expands all target prefixed variables.
# Note: Could filter out everything with TARGET_ prefix but don't risk it as
# it's less specific.
#
expand-targets-all = $(foreach v,$(filter TARGET_BIN_%,$(.VARIABLES)),$($(v))) \
                     $(foreach v,$(filter TARGET_LIB_%,$(.VARIABLES)),$($(v)))

###############################################################################
# Helper functions to build linker flags and library dependencies
#
bin-ldflags = -Wl,--start-group $(foreach v,$1,-l$(v)) -Wl,--end-group $(APP_LDFLAGS) $2
bin-deps    = $(foreach v,$1,$(TARGET_LIB_$v))
