# Compiler and linker options

# Use C++ 11
CXXFLAGS += -std=c++11
CFLAGS += -std=c11

# Compiler warning level
warning_flags := \
	-Wall -Wextra -Werror

# Compiler optimzation
CXXFLAGS += -O0 -g
CFLAGS += -O0 -g

CFLAGS += $(warning_flags)
CXXFLAGS += $(warning_flags)

# Build with threading support
CFLAGS   += -pthread
CXXFLAGS += -pthread
LDFLAGS  += -pthread

# Add top level project directory to include search path
CPPFLAGS += -iquote $(CURDIR)

# Link flags used for all application binaries (but not shared libraries).
APP_LDFLAGS := -lrt -lm

# Always search for libraries in the output library directory first.
# This is just compiled libraries from this source code.
LDFLAGS := -L$(OUTDIR_LIB) $(LDFLAGS)
