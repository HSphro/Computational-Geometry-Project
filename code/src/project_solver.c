#include "project_solver.h"
#include <stdlib.h>



solver_st* solver_create(void) {
    solver_st * solver = calloc(1, sizeof(solver_st));
    // Initialize solver info 
    
    return solver;
}

void solver_destroy(solver_st* solver) {
    // Deallocate memory of solver 

    free(solver);
}


solver_status_st solve_navier_stokes(
    solver_st* solver,
    double   const * nodes, uint32_t nnodes, 
    uint32_t const * elems, uint32_t nelems, 
    node_type_et const * node_type, 
    uint32_t const * bnd_edges, uint32_t n_bnd_edges,
    physical_props_st const * props, double* uvp) {

    // Call migflow to solve the system

    return solver->status;
}
