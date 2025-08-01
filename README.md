# cuOpt JSON to C API

A C program that reads cuOpt JSON files containing Linear Programming (LP) and Mixed Integer Programming (MIP) problems and solves them using the NVIDIA cuOpt C API.

## Features

- **JSON Parsing**: Reads cuOpt JSON format for LP/MIP problems
- **Full API Integration**: Uses the complete cuOpt C API for solving optimization problems
- **Flexible Input**: Supports both LP and MIP problems with various constraint types
- **Comprehensive Output**: Displays detailed solution information including objective value, solve time, and solution variables
- **Error Handling**: Robust error handling and memory management
- **Auto-detection**: Automatically detects cuOpt installation paths

## JSON Format

The program expects JSON files in the cuOpt format with the following structure:

```json
{
  "csr_constraint_matrix": {
    "offsets": [0, 2, 4],
    "indices": [0, 1, 0, 1],
    "values": [3.0, 4.0, 2.7, 10.1]
  },
  "objective_data": {
    "coefficients": [-0.2, 0.1],
    "scalability_factor": 1.0,
    "offset": 0.0
  },
  "constraint_bounds": {
    "upper_bounds": [5.4, 4.9],
    "lower_bounds": ["-inf", "-inf"]
  },
  "variable_bounds": {
    "upper_bounds": ["inf", "inf"],
    "lower_bounds": [0.0, 0.0]
  },
  "maximize": false,
  "variable_types": ["C", "C"],
  "variable_names": ["x1", "x2"]
}
```

### JSON Field Descriptions

- **`csr_constraint_matrix`**: Constraint matrix in Compressed Sparse Row format
  - `offsets`: Row offsets array (size: num_constraints + 1)
  - `indices`: Column indices of non-zero elements
  - `values`: Values of non-zero elements

- **`objective_data`**: Objective function information
  - `coefficients`: Objective coefficients for each variable
  - `offset`: Constant offset added to objective (optional, default: 0.0)
  - `scalability_factor`: Scaling factor (optional, default: 1.0)

- **`constraint_bounds`**: Constraint bounds
  - `upper_bounds`: Upper bounds for constraints
  - `lower_bounds`: Lower bounds for constraints
  - Alternative format: `bounds` array with `types` array ("L", "G", "E")

- **`variable_bounds`**: Variable bounds
  - `upper_bounds`: Upper bounds for variables (use "inf" for infinity)
  - `lower_bounds`: Lower bounds for variables (use "-inf" for negative infinity)

- **`maximize`**: Boolean flag for maximization (optional, default: false)

- **`variable_types`**: Array of variable types (optional, default: all continuous)
  - `"C"`: Continuous variable
  - `"I"`: Integer variable

- **`variable_names`**: Array of variable names (optional)

## Prerequisites

### System Requirements
- Linux system with CUDA support
- NVIDIA cuOpt library installed
- C compiler (gcc or clang)
- cJSON library for JSON parsing

### Dependencies
- **libcjson-dev**: JSON parsing library
- **libcuopt**: NVIDIA cuOpt library

#### Installing Dependencies (Ubuntu/Debian)
```bash
# Install cJSON library
sudo apt-get update
sudo apt-get install libcjson-dev

# Or use the Makefile target
make install-deps
```

## Building

### Option 1: Using the Makefile (Recommended)

The Makefile provides several convenient targets and automatic path detection:

```bash
# View all available targets
make help

# Build with automatic path detection
make

# Build with custom paths (if cuOpt is built locally)
make CUOPT_INCLUDE_PATH=../cpp/include CUOPT_LIBRARY_PATH=../cpp/build

# Build with custom compiler
make CC=clang

# Build debug version
make debug

# Clean and rebuild
make rebuild
```

### Option 2: Using the Build Script

```bash
# Run the automated build script
./c_build.sh
```

### Option 3: Manual Compilation

If you prefer to compile manually or the Makefile doesn't work:

```bash
# Find cuOpt paths (adjust as needed)
CUOPT_INCLUDE_PATH="../cpp/include"
CUOPT_LIBRARY_PATH="../cpp/build"

# Compile
gcc -Wall -Wextra -O2 -std=c99 \
    -I$CUOPT_INCLUDE_PATH \
    -L$CUOPT_LIBRARY_PATH \
    -o cuopt_json_to_c_api cuopt_json_to_c_api.c \
    -lcuopt -lcjson -lm
```

### Build Troubleshooting

If you encounter build issues:

1. **Check paths**: Use `make print-vars` to see detected paths
2. **Manual paths**: Set `CUOPT_INCLUDE_PATH` and `CUOPT_LIBRARY_PATH`
3. **Dependencies**: Ensure libcjson-dev is installed
4. **cuOpt library**: Ensure libcuopt.so is built and accessible

## Usage

### Basic Usage
```bash
./cuopt_json_to_c_api problem.json
```

### Example Usage

**If using conda environment with cuOpt installed:**
```bash
# Activate the environment where cuOpt is installed
conda activate cuopt_dev  # or your cuOpt environment name

# Set library path (if needed)
export LD_LIBRARY_PATH=$CONDA_PREFIX/lib:$LD_LIBRARY_PATH

# Run the solver
./cuopt_json_to_c_api test_problem.json
```

**Other usage examples:**
1. **Use the provided test file**:
```bash
./cuopt_json_to_c_api test_problem.json
```

2. **Use the repository's example** (if running from cuOpt repo root):
```bash
./cuopt_json_to_c_api ../python/cuopt/cuopt/linear_programming/example_cuopt_problem.json
```

### Expected Output

The program will display:
- Problem parsing status
- Problem dimensions (constraints, variables, non-zeros)
- Solver progress and results
- Termination status
- Objective value
- Solve time
- Solution variables (first 20 shown)
- MIP gap and bounds (for MIP problems)

Example output:
```
cuOpt JSON to C API
===================
Reading JSON file: test_problem.json
Successfully parsed JSON file
Creating and solving problem...
Problem size: 2 constraints, 2 variables, 4 nonzeros

Results:
--------
Termination status: Optimal (1)
Solve time: 0.001234 seconds
Objective value: -0.360000

Primal Solution (showing first 2 variables):
x0 = 1.800000
x1 = 0.000000

Solver completed successfully!
```

## Problem Examples

### Example 1: Simple LP Problem
```json
{
  "csr_constraint_matrix": {
    "offsets": [0, 2, 4],
    "indices": [0, 1, 0, 1],
    "values": [3.0, 4.0, 2.7, 10.1]
  },
  "objective_data": {
    "coefficients": [-0.2, 0.1],
    "offset": 0.0
  },
  "constraint_bounds": {
    "upper_bounds": [5.4, 4.9],
    "lower_bounds": ["-inf", "-inf"]
  },
  "variable_bounds": {
    "upper_bounds": ["inf", "inf"],
    "lower_bounds": [0.0, 0.0]
  },
  "maximize": false,
  "variable_types": ["C", "C"]
}
```

### Example 2: Mixed Integer Problem
```json
{
  "csr_constraint_matrix": {
    "offsets": [0, 2, 4],
    "indices": [0, 1, 0, 1], 
    "values": [2.0, 4.0, 3.0, 2.0]
  },
  "constraint_bounds": {
    "bounds": [230.0, 190.0],
    "types": ["G", "L"]
  },
  "objective_data": {
    "coefficients": [5.0, 3.0],
    "offset": 0.0
  },
  "variable_bounds": {
    "upper_bounds": ["inf", 50.0],
    "lower_bounds": [0.0, 10.0]
  },
  "maximize": true,
  "variable_types": ["I", "I"],
  "variable_names": ["x", "y"]
}
```

## Advanced Configuration

### Solver Parameters

The program sets some default solver parameters, but you can modify these in the code:

- `CUOPT_ABSOLUTE_PRIMAL_TOLERANCE`: Primal feasibility tolerance (default: 1e-6)
- `CUOPT_TIME_LIMIT`: Maximum solve time in seconds (default: 300)

### Error Codes

The program returns different exit codes:
- `0`: Success
- `1`: General error (file not found, parsing error, solve error)

## Files

- **`cuopt_json_to_c_api.c`**: Main C program
- **`Makefile`**: Build system with automatic path detection
- **`c_build.sh`**: Automated build script
- **`test_problem.json`**: Simple test problem
- **`README.md`**: This documentation

## Limitations

- **Large problems**: Very large problems may require more memory
- **JSON parsing**: Relies on cJSON library for JSON parsing
- **File size**: No explicit limit, but large JSON files may be slow to parse

## Integration with cuOpt

This program demonstrates how to:
1. Parse problem data from external formats
2. Convert data to cuOpt C API format
3. Configure solver settings
4. Extract and display results
5. Handle both LP and MIP problems

It serves as a reference implementation for integrating cuOpt into larger applications that need to solve optimization problems from various data sources.

## License

This code is provided as an example and follows the same licensing terms as the NVIDIA cuOpt library. Please refer to the cuOpt documentation for licensing details.

## Contributing

Feel free to enhance this program by:
- Adding support for additional JSON formats
- Implementing more solver parameter options
- Adding solution export capabilities
- Improving error handling and validation 