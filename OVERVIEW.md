# cuOpt JSON to C API - Overview

This directory contains a complete C implementation for reading cuOpt JSON files and solving them using the NVIDIA cuOpt C API.

## What this solves

The problem of integrating cuOpt into C applications that need to:
- Read optimization problems from JSON files
- Convert JSON data to cuOpt C API format
- Solve LP and MIP problems using cuOpt
- Extract and display comprehensive solution information

## Files

- **`cuopt_json_to_c_api.c`** - Main C program with JSON parsing and cuOpt API integration
- **`Makefile`** - Build system with automatic path detection and dependency management
- **`c_build.sh`** - Automated build script with dependency installation
- **`README.md`** - Comprehensive documentation and usage guide
- **`test_problem.json`** - Simple test problem in cuOpt JSON format
- **`OVERVIEW.md`** - This overview file

## Quick Start

From this directory:

```bash
# Build using the script (installs dependencies if needed)
./c_build.sh

# Or build using make
make

# Test with the provided example
./cuopt_json_to_c_api test_problem.json
```

## Key Features

- **Complete JSON Support**: Handles all cuOpt JSON format fields including CSR matrices, bounds, variable types
- **Robust Parsing**: Handles infinity values, optional fields, and different constraint formats  
- **Full C API Usage**: Demonstrates proper use of cuOpt C API for problem creation, solving, and result extraction
- **Memory Management**: Proper allocation/deallocation with error handling
- **Auto-detection**: Automatically finds cuOpt headers and libraries
- **Cross-platform**: Works with both local and system cuOpt installations

This serves as a reference implementation for integrating cuOpt into larger C applications. 