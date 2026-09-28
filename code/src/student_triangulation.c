#include "student_triangulation.h"
#include <stdlib.h>

// Documentation can be found in student_triangulation.h

void mesh_triangulate(mesh_st * mesh) {
    // Reuse existing memory if it exists, otherwise allocate and compute
}

boundary_st mesh_get_boundary(mesh_st const * mesh) {
    // Do the thing...
    return (boundary_st){
        .nodes=mesh->nodes,
        .edges=NULL,
        .nnodes=mesh->nnodes,
        .nedges=0};
}

uint32_t mesh_delete_elements(
        uint32_t * elems, 
        uint8_t const * to_delete,
        uint32_t nelems) {
    // Do the thing...
    return nelems;
}

uint32_t mesh_adapt(
        double ** nodes, 
        double ** node_size,
        uint32_t ** elems, 
        uint32_t * nnodes,
        uint32_t * nelems) {

    // Helper to access the real data.
    double * pnodes = *nodes;
    double * pnode_size = *node_size;
    uint32_t* pelems = *elems;

    // Do the thing...

    // Do not forget to write the pointer to the double pointer afterward
    // so that the caller can see you've reallocated data
    *nodes = pnodes;
    *node_size = pnode_size;
    *elems = pelems;
    *nnodes = 0;
    *nelems = 0;
    return 0;
}
