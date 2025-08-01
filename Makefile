# Makefile for cuOpt JSON to C API
# This builds a C program that reads cuOpt JSON files and solves them using the C API

# Default compiler
CC = gcc

# Default flags
CFLAGS = -Wall -Wextra -O2 -std=c99

# Program name
PROGRAM = cuopt_json_to_c_api

# Source files
SOURCES = cuopt_json_to_c_api.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Default target
all: $(PROGRAM)

# Build the program
$(PROGRAM): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(PROGRAM) $(LIBS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(PROGRAM)

# Clean and rebuild
rebuild: clean all

# Install dependencies (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y libcjson-dev

# Help target
help:
	@echo "cuOpt JSON to C API Makefile"
	@echo "============================"
	@echo ""
	@echo "Targets:"
	@echo "  all         - Build the program (default)"
	@echo "  clean       - Remove build artifacts"
	@echo "  rebuild     - Clean and rebuild"
	@echo "  install-deps- Install dependencies (Ubuntu/Debian)"
	@echo "  help        - Show this help"
	@echo ""
	@echo "Variables you may need to set:"
	@echo "  CUOPT_INCLUDE_PATH  - Path to cuOpt include directory"
	@echo "  CUOPT_LIBRARY_PATH  - Path to cuOpt library directory"
	@echo ""
	@echo "Example usage:"
	@echo "  # If cuOpt is installed in system paths:"
	@echo "  make"
	@echo ""
	@echo "  # If cuOpt is built locally (from c_json directory):"
	@echo "  make CUOPT_INCLUDE_PATH=../cpp/include CUOPT_LIBRARY_PATH=../cpp/build"
	@echo ""
	@echo "  # Custom compiler or flags:"
	@echo "  make CC=clang CFLAGS=\"-O3 -g\""

# Set up include and library paths
ifdef CUOPT_INCLUDE_PATH
    INCLUDES += -I$(CUOPT_INCLUDE_PATH)
endif

ifdef CUOPT_LIBRARY_PATH
    LDFLAGS += -L$(CUOPT_LIBRARY_PATH)
endif

# Add cJSON flags if available
CJSON_CFLAGS := $(shell pkg-config --cflags libcjson 2>/dev/null)
CJSON_LIBS := $(shell pkg-config --libs libcjson 2>/dev/null)

ifneq ($(CJSON_CFLAGS),)
    CFLAGS += $(CJSON_CFLAGS)
endif

# Default library paths (try common system locations)
ifneq ($(CJSON_LIBS),)
    LIBS = -lcuopt $(CJSON_LIBS) -lm
else
    LIBS = -lcuopt -lcjson -lm
endif

# Auto-detect cuOpt paths if not specified
ifndef CUOPT_INCLUDE_PATH
    # Search order: current conda env, other conda envs, local build, system paths
    SEARCH_PATHS := $(CONDA_PREFIX) /home/tmckay/miniforge3/envs/cuopt_dev /usr/include /usr/local/include ..
    CUOPT_HEADER_SEARCH := $(shell for path in $(SEARCH_PATHS); do find "$$path" -name "cuopt_c.h" -path "*/linear_programming/*" 2>/dev/null | head -1; done | head -1)
    ifneq ($(CUOPT_HEADER_SEARCH),)
        CUOPT_INCLUDE_DIR := $(shell dirname $(shell dirname $(shell dirname $(CUOPT_HEADER_SEARCH))))
        INCLUDES += -I$(CUOPT_INCLUDE_DIR)
        $(info Found cuOpt headers at: $(CUOPT_INCLUDE_DIR))
    endif
endif

ifndef CUOPT_LIBRARY_PATH
    # Search order: current conda env, other conda envs, local build, system paths
    SEARCH_PATHS := $(CONDA_PREFIX)/lib /home/tmckay/miniforge3/envs/cuopt_dev/lib /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64 ../cpp/build
    CUOPT_LIBRARY_SEARCH := $(shell for path in $(SEARCH_PATHS); do find "$$path" -name "libcuopt.so" 2>/dev/null | head -1; done | head -1)
    ifneq ($(CUOPT_LIBRARY_SEARCH),)
        CUOPT_LIB_DIR := $(shell dirname $(CUOPT_LIBRARY_SEARCH))
        LDFLAGS += -L$(CUOPT_LIB_DIR)
        $(info Found cuOpt library at: $(CUOPT_LIB_DIR))
    endif
endif

# Debug target
debug: CFLAGS += -g -DDEBUG
debug: $(PROGRAM)

# Print variables for debugging
print-vars:
	@echo "CC = $(CC)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "INCLUDES = $(INCLUDES)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "LIBS = $(LIBS)"
	@echo "SOURCES = $(SOURCES)"
	@echo "OBJECTS = $(OBJECTS)"

.PHONY: all clean rebuild install-deps help debug print-vars 