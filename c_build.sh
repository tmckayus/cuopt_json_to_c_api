#!/bin/bash

# Build script for cuOpt JSON to C API
# This script attempts to automatically find cuOpt and build the program

set -e  # Exit on any error

echo "cuOpt JSON to C API Build Script"
echo "================================="

# Check for cuOpt in different locations (priority order)
if [ -n "$CONDA_PREFIX" ] && [ -f "$CONDA_PREFIX/include/cuopt/linear_programming/cuopt_c.h" ]; then
    echo "Found cuOpt in current conda environment: $CONDA_PREFIX"
    CUOPT_INCLUDE_PATH="$CONDA_PREFIX/include"
    CUOPT_LIBRARY_PATH="$CONDA_PREFIX/lib"
elif [ -f "/home/tmckay/miniforge3/envs/cuopt_dev/include/cuopt/linear_programming/cuopt_c.h" ]; then
    echo "Found cuOpt in conda environment: cuopt_dev"
    CUOPT_INCLUDE_PATH="/home/tmckay/miniforge3/envs/cuopt_dev/include"
    CUOPT_LIBRARY_PATH="/home/tmckay/miniforge3/envs/cuopt_dev/lib"
elif [ -f "../cpp/include/cuopt/linear_programming/cuopt_c.h" ]; then
    echo "Found cuOpt source in parent directory"
    CUOPT_INCLUDE_PATH="../cpp/include"
    
    # Check if library is built
    if [ -f "../cpp/build/libcuopt.so" ]; then
        echo "Found built cuOpt library"
        CUOPT_LIBRARY_PATH="../cpp/build"
    else
        echo "Warning: cuOpt library not found at ../cpp/build/libcuopt.so"
        echo "You may need to build cuOpt first. Try running:"
        echo "  cd ../cpp && mkdir -p build && cd build"
        echo "  cmake .. && make -j"
        echo ""
        echo "Continuing with system paths..."
    fi
fi

# Check for cJSON library
echo "Checking for cJSON library..."
if ! pkg-config --exists libcjson; then
    echo "cJSON library not found. Installing..."
    if command -v apt-get >/dev/null 2>&1; then
        sudo apt-get update
        sudo apt-get install -y libcjson-dev
    elif command -v yum >/dev/null 2>&1; then
        sudo yum install -y libcjson-devel
    elif command -v dnf >/dev/null 2>&1; then
        sudo dnf install -y libcjson-devel
    else
        echo "Error: Could not install cJSON. Please install libcjson-dev manually."
        exit 1
    fi
fi

# Build using make
echo "Building cuOpt JSON to C API..."

if [ -n "$CUOPT_INCLUDE_PATH" ] && [ -n "$CUOPT_LIBRARY_PATH" ]; then
    echo "Using local cuOpt paths:"
    echo "  Include: $CUOPT_INCLUDE_PATH"
    echo "  Library: $CUOPT_LIBRARY_PATH"
    make CUOPT_INCLUDE_PATH="$CUOPT_INCLUDE_PATH" CUOPT_LIBRARY_PATH="$CUOPT_LIBRARY_PATH"
else
    echo "Using system paths (auto-detection)"
    make
fi

echo ""
echo "Build completed successfully!"
echo ""

# Check if we're using conda environment and provide setup instructions
if [ -n "$CUOPT_INCLUDE_PATH" ] && [[ "$CUOPT_INCLUDE_PATH" == *"/envs/"* ]]; then
    echo "Note: For optimal compatibility, build outside conda environment:"
    echo "  conda deactivate"
    echo "  make clean && make"
    echo ""
    echo "Then when running, set the library path:"
    echo "  export LD_LIBRARY_PATH=$CUOPT_LIBRARY_PATH:\$LD_LIBRARY_PATH"
    echo "  ./cuopt_json_to_c_api test_problem.json"
    echo ""
fi

echo "Usage:"
echo "  ./cuopt_json_to_c_api <json_file>"
echo ""
echo "Example:"
echo "  ./cuopt_json_to_c_api test_problem.json"
echo ""

# Test if the example JSON file exists
if [ -f "test_problem.json" ]; then
    echo "Found test_problem.json - you can test with:"
    echo "  ./cuopt_json_to_c_api test_problem.json"
elif [ -f "../python/cuopt/cuopt/linear_programming/example_cuopt_problem.json" ]; then
    echo "Found example JSON file - you can test with:"
    echo "  ./cuopt_json_to_c_api ../python/cuopt/cuopt/linear_programming/example_cuopt_problem.json"
fi
