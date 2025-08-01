/*
 * cuOpt JSON Solver - A C program that reads cuOpt JSON files and solves them using the C API
 * 
 * This program demonstrates how to:
 * 1. Parse a cuOpt JSON file containing LP/MIP problem data
 * 2. Convert the JSON data to the C API format
 * 3. Solve the problem using the cuOpt C API
 * 4. Display the results
 */

#include <cuopt/linear_programming/cuopt_c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cJSON.h>

// Helper function to convert termination status to string
const char* termination_status_to_string(cuopt_int_t termination_status)
{
    switch (termination_status) {
        case CUOPT_TERIMINATION_STATUS_OPTIMAL:
            return "Optimal";
        case CUOPT_TERIMINATION_STATUS_INFEASIBLE:
            return "Infeasible";
        case CUOPT_TERIMINATION_STATUS_UNBOUNDED:
            return "Unbounded";
        case CUOPT_TERIMINATION_STATUS_ITERATION_LIMIT:
            return "Iteration limit";
        case CUOPT_TERIMINATION_STATUS_TIME_LIMIT:
            return "Time limit";
        case CUOPT_TERIMINATION_STATUS_NUMERICAL_ERROR:
            return "Numerical error";
        case CUOPT_TERIMINATION_STATUS_PRIMAL_FEASIBLE:
            return "Primal feasible";
        case CUOPT_TERIMINATION_STATUS_FEASIBLE_FOUND:
            return "Feasible found";
        default:
            return "Unknown";
    }
}

// Helper function to parse infinity values from JSON
cuopt_float_t parse_numeric_value(cJSON* item) {
    if (cJSON_IsNumber(item)) {
        return item->valuedouble;
    } else if (cJSON_IsString(item)) {
        char* str = item->valuestring;
        if (strcmp(str, "inf") == 0 || strcmp(str, "infinity") == 0) {
            return CUOPT_INFINITY;
        } else if (strcmp(str, "-inf") == 0 || strcmp(str, "-infinity") == 0 || strcmp(str, "ninf") == 0) {
            return -CUOPT_INFINITY;
        } else {
            return strtod(str, NULL);
        }
    }
    return 0.0;
}

// Structure to hold parsed JSON data
typedef struct {
    // CSR matrix data
    cuopt_int_t* row_offsets;
    cuopt_int_t* column_indices;
    cuopt_float_t* matrix_values;
    cuopt_int_t num_constraints;
    cuopt_int_t num_variables;
    cuopt_int_t nnz;
    
    // Objective data
    cuopt_float_t* objective_coefficients;
    cuopt_float_t objective_offset;
    cuopt_int_t objective_sense;  // CUOPT_MINIMIZE or CUOPT_MAXIMIZE
    
    // Constraint bounds
    cuopt_float_t* constraint_lower_bounds;
    cuopt_float_t* constraint_upper_bounds;
    
    // Variable bounds
    cuopt_float_t* variable_lower_bounds;
    cuopt_float_t* variable_upper_bounds;
    
    // Variable types
    char* variable_types;
    
} ProblemData;

// Function to free allocated memory
void free_problem_data(ProblemData* data) {
    if (data) {
        free(data->row_offsets);
        free(data->column_indices);
        free(data->matrix_values);
        free(data->objective_coefficients);
        free(data->constraint_lower_bounds);
        free(data->constraint_upper_bounds);
        free(data->variable_lower_bounds);
        free(data->variable_upper_bounds);
        free(data->variable_types);
        memset(data, 0, sizeof(ProblemData));
    }
}

// Function to parse cuOpt JSON file
int parse_cuopt_json(const char* filename, ProblemData* data) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return -1;
    }
    
    // Read file content
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* file_content = malloc(file_size + 1);
    if (!file_content) {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return -1;
    }
    
    size_t bytes_read = fread(file_content, 1, file_size, file);
    file_content[bytes_read] = '\0';
    fclose(file);
    
    if (bytes_read != (size_t)file_size) {
        printf("Warning: Only read %zu bytes out of %ld expected\n", bytes_read, file_size);
    }
    
    // Parse JSON
    cJSON* json = cJSON_Parse(file_content);
    free(file_content);
    
    if (!json) {
        printf("Error: Failed to parse JSON\n");
        return -1;
    }
    
    // Parse CSR constraint matrix
    cJSON* csr_matrix = cJSON_GetObjectItem(json, "csr_constraint_matrix");
    if (!csr_matrix) {
        printf("Error: Missing csr_constraint_matrix in JSON\n");
        cJSON_Delete(json);
        return -1;
    }
    
    cJSON* offsets = cJSON_GetObjectItem(csr_matrix, "offsets");
    cJSON* indices = cJSON_GetObjectItem(csr_matrix, "indices");
    cJSON* values = cJSON_GetObjectItem(csr_matrix, "values");
    
    if (!offsets || !indices || !values) {
        printf("Error: Invalid CSR matrix format\n");
        cJSON_Delete(json);
        return -1;
    }
    
    data->num_constraints = cJSON_GetArraySize(offsets) - 1;
    data->nnz = cJSON_GetArraySize(indices);
    
    // Allocate memory for CSR data
    data->row_offsets = malloc((data->num_constraints + 1) * sizeof(cuopt_int_t));
    data->column_indices = malloc(data->nnz * sizeof(cuopt_int_t));
    data->matrix_values = malloc(data->nnz * sizeof(cuopt_float_t));
    
    // Parse CSR data
    for (int i = 0; i <= data->num_constraints; i++) {
        data->row_offsets[i] = cJSON_GetArrayItem(offsets, i)->valueint;
    }
    
    for (int i = 0; i < data->nnz; i++) {
        data->column_indices[i] = cJSON_GetArrayItem(indices, i)->valueint;
        data->matrix_values[i] = cJSON_GetArrayItem(values, i)->valuedouble;
    }
    
    // Parse objective data
    cJSON* objective_data = cJSON_GetObjectItem(json, "objective_data");
    if (!objective_data) {
        printf("Error: Missing objective_data in JSON\n");
        cJSON_Delete(json);
        return -1;
    }
    
    cJSON* obj_coeffs = cJSON_GetObjectItem(objective_data, "coefficients");
    data->num_variables = cJSON_GetArraySize(obj_coeffs);
    
    data->objective_coefficients = malloc(data->num_variables * sizeof(cuopt_float_t));
    for (int i = 0; i < data->num_variables; i++) {
        data->objective_coefficients[i] = cJSON_GetArrayItem(obj_coeffs, i)->valuedouble;
    }
    
    cJSON* offset = cJSON_GetObjectItem(objective_data, "offset");
    data->objective_offset = offset ? offset->valuedouble : 0.0;
    
    // Parse maximize flag
    cJSON* maximize = cJSON_GetObjectItem(json, "maximize");
    data->objective_sense = (maximize && cJSON_IsTrue(maximize)) ? CUOPT_MAXIMIZE : CUOPT_MINIMIZE;
    
    // Parse constraint bounds
    cJSON* constraint_bounds = cJSON_GetObjectItem(json, "constraint_bounds");
    if (constraint_bounds) {
        data->constraint_lower_bounds = malloc(data->num_constraints * sizeof(cuopt_float_t));
        data->constraint_upper_bounds = malloc(data->num_constraints * sizeof(cuopt_float_t));
        
        cJSON* lower_bounds = cJSON_GetObjectItem(constraint_bounds, "lower_bounds");
        cJSON* upper_bounds = cJSON_GetObjectItem(constraint_bounds, "upper_bounds");
        
        if (lower_bounds && upper_bounds) {
            for (int i = 0; i < data->num_constraints; i++) {
                data->constraint_lower_bounds[i] = parse_numeric_value(cJSON_GetArrayItem(lower_bounds, i));
                data->constraint_upper_bounds[i] = parse_numeric_value(cJSON_GetArrayItem(upper_bounds, i));
            }
        } else {
            // Fallback to bounds and types format
            cJSON* bounds = cJSON_GetObjectItem(constraint_bounds, "bounds");
            cJSON* types = cJSON_GetObjectItem(constraint_bounds, "types");
            
            if (bounds && types) {
                for (int i = 0; i < data->num_constraints; i++) {
                    cuopt_float_t bound_value = parse_numeric_value(cJSON_GetArrayItem(bounds, i));
                    char* type = cJSON_GetArrayItem(types, i)->valuestring;
                    
                    if (strcmp(type, "L") == 0) {  // Less than or equal
                        data->constraint_lower_bounds[i] = -CUOPT_INFINITY;
                        data->constraint_upper_bounds[i] = bound_value;
                    } else if (strcmp(type, "G") == 0) {  // Greater than or equal
                        data->constraint_lower_bounds[i] = bound_value;
                        data->constraint_upper_bounds[i] = CUOPT_INFINITY;
                    } else if (strcmp(type, "E") == 0) {  // Equal
                        data->constraint_lower_bounds[i] = bound_value;
                        data->constraint_upper_bounds[i] = bound_value;
                    }
                }
            }
        }
    }
    
    // Parse variable bounds
    cJSON* variable_bounds = cJSON_GetObjectItem(json, "variable_bounds");
    if (variable_bounds) {
        data->variable_lower_bounds = malloc(data->num_variables * sizeof(cuopt_float_t));
        data->variable_upper_bounds = malloc(data->num_variables * sizeof(cuopt_float_t));
        
        cJSON* var_lower = cJSON_GetObjectItem(variable_bounds, "lower_bounds");
        cJSON* var_upper = cJSON_GetObjectItem(variable_bounds, "upper_bounds");
        
        for (int i = 0; i < data->num_variables; i++) {
            data->variable_lower_bounds[i] = parse_numeric_value(cJSON_GetArrayItem(var_lower, i));
            data->variable_upper_bounds[i] = parse_numeric_value(cJSON_GetArrayItem(var_upper, i));
        }
    }
    
    // Parse variable types
    cJSON* variable_types = cJSON_GetObjectItem(json, "variable_types");
    if (variable_types) {
        data->variable_types = malloc(data->num_variables * sizeof(char));
        for (int i = 0; i < data->num_variables; i++) {
            char* type_str = cJSON_GetArrayItem(variable_types, i)->valuestring;
            if (strcmp(type_str, "I") == 0) {
                data->variable_types[i] = CUOPT_INTEGER;
            } else {
                data->variable_types[i] = CUOPT_CONTINUOUS;
            }
        }
    } else {
        // Default to continuous variables
        data->variable_types = malloc(data->num_variables * sizeof(char));
        for (int i = 0; i < data->num_variables; i++) {
            data->variable_types[i] = CUOPT_CONTINUOUS;
        }
    }
    
    cJSON_Delete(json);
    return 0;
}

// Function to solve the problem using cuOpt C API
int solve_problem(const ProblemData* data) {
    cuOptOptimizationProblem problem = NULL;
    cuOptSolverSettings settings = NULL;
    cuOptSolution solution = NULL;
    cuopt_int_t status;
    
    printf("Creating and solving problem...\n");
    printf("Problem size: %d constraints, %d variables, %d nonzeros\n", 
           data->num_constraints, data->num_variables, data->nnz);
    
    // Create the problem using ranged formulation
    status = cuOptCreateRangedProblem(data->num_constraints,
                                     data->num_variables,
                                     data->objective_sense,
                                     data->objective_offset,
                                     data->objective_coefficients,
                                     data->row_offsets,
                                     data->column_indices,
                                     data->matrix_values,
                                     data->constraint_lower_bounds,
                                     data->constraint_upper_bounds,
                                     data->variable_lower_bounds,
                                     data->variable_upper_bounds,
                                     data->variable_types,
                                     &problem);
    
    if (status != CUOPT_SUCCESS) {
        printf("Error creating problem: %d\n", status);
        goto CLEANUP;
    }
    
    // Create solver settings
    status = cuOptCreateSolverSettings(&settings);
    if (status != CUOPT_SUCCESS) {
        printf("Error creating solver settings: %d\n", status);
        goto CLEANUP;
    }
    
    // Set solver parameters (you can adjust these as needed)
    status = cuOptSetFloatParameter(settings, CUOPT_ABSOLUTE_PRIMAL_TOLERANCE, 1e-6);
    if (status != CUOPT_SUCCESS) {
        printf("Warning: Could not set primal tolerance: %d\n", status);
    }
    
    status = cuOptSetFloatParameter(settings, CUOPT_TIME_LIMIT, 300.0);  // 5 minute limit
    if (status != CUOPT_SUCCESS) {
        printf("Warning: Could not set time limit: %d\n", status);
    }
    
    // Solve the problem
    status = cuOptSolve(problem, settings, &solution);
    if (status != CUOPT_SUCCESS) {
        printf("Error solving problem: %d\n", status);
        goto CLEANUP;
    }
    
    // Get and display results
    cuopt_float_t solve_time;
    cuopt_int_t termination_status;
    cuopt_float_t objective_value;
    
    status = cuOptGetSolveTime(solution, &solve_time);
    if (status != CUOPT_SUCCESS) {
        printf("Error getting solve time: %d\n", status);
        goto CLEANUP;
    }
    
    status = cuOptGetTerminationStatus(solution, &termination_status);
    if (status != CUOPT_SUCCESS) {
        printf("Error getting termination status: %d\n", status);
        goto CLEANUP;
    }
    
    status = cuOptGetObjectiveValue(solution, &objective_value);
    if (status != CUOPT_SUCCESS) {
        printf("Error getting objective value: %d\n", status);
        goto CLEANUP;
    }
    
    // Print results
    printf("\nResults:\n");
    printf("--------\n");
    printf("Termination status: %s (%d)\n", termination_status_to_string(termination_status), termination_status);
    printf("Solve time: %f seconds\n", solve_time);
    printf("Objective value: %f\n", objective_value);
    
    // Get and print solution variables (first 20 or fewer)
    cuopt_float_t* solution_values = malloc(data->num_variables * sizeof(cuopt_float_t));
    status = cuOptGetPrimalSolution(solution, solution_values);
    if (status == CUOPT_SUCCESS) {
        printf("\nPrimal Solution (showing first %d variables):\n", 
               data->num_variables < 20 ? data->num_variables : 20);
        for (int i = 0; i < (data->num_variables < 20 ? data->num_variables : 20); i++) {
            printf("x%d = %f\n", i, solution_values[i]);
        }
        if (data->num_variables > 20) {
            printf("... (showing only first 20 of %d variables)\n", data->num_variables);
        }
    } else {
        printf("Error getting solution values: %d\n", status);
    }
    free(solution_values);
    
    // Check if this is a MIP and get MIP-specific information
    cuopt_int_t is_mip;
    status = cuOptIsMIP(problem, &is_mip);
    if (status == CUOPT_SUCCESS && is_mip) {
        cuopt_float_t mip_gap;
        status = cuOptGetMIPGap(solution, &mip_gap);
        if (status == CUOPT_SUCCESS) {
            printf("MIP Gap: %f\n", mip_gap);
        }
        
        cuopt_float_t solution_bound;
        status = cuOptGetSolutionBound(solution, &solution_bound);
        if (status == CUOPT_SUCCESS) {
            printf("Solution Bound: %f\n", solution_bound);
        }
    }
    
CLEANUP:
    cuOptDestroyProblem(&problem);
    cuOptDestroySolverSettings(&settings);
    cuOptDestroySolution(&solution);
    
    return status;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <cuopt_json_file>\n", argv[0]);
        printf("\nThis program reads a cuOpt JSON file and solves it using the cuOpt C API.\n");
        printf("The JSON file should contain LP or MIP problem data in cuOpt format.\n");
        return 1;
    }
    
    ProblemData data;
    memset(&data, 0, sizeof(ProblemData));
    
    printf("cuOpt JSON Solver\n");
    printf("=================\n");
    printf("Reading JSON file: %s\n", argv[1]);
    
    // Parse the JSON file
    if (parse_cuopt_json(argv[1], &data) != 0) {
        printf("Failed to parse JSON file\n");
        free_problem_data(&data);
        return 1;
    }
    
    printf("Successfully parsed JSON file\n");
    
    // Solve the problem
    cuopt_int_t solve_status = solve_problem(&data);
    
    // Clean up
    free_problem_data(&data);
    
    if (solve_status == CUOPT_SUCCESS) {
        printf("\nSolver completed successfully!\n");
        return 0;
    } else {
        printf("\nSolver failed with status: %d\n", solve_status);
        return 1;
    }
} 