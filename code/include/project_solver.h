#ifndef __PROJECT_SOLVER_H__
#define __PROJECT_SOLVER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BULK = 0,          // The node is "completely inside" the mesh
    FREE_SURFACE = 1,  // The node is on the boundary of the mesh but not in contact with a solid wall
    WALL = 2           // The node is on the boundary of the mesh and in contact with a solid wall
} node_type_et; 

// Physical properties of the material
typedef struct {
    double density;      // rho
    double viscosity;    // nu
    double gravity[2];   // gravity in x, y
    double dt;           // time step
} physical_props_st;

// Info about the solver that can be queried afterward
typedef struct {
    int success;    
    //... Whatever feels useful
} solver_status_st;

typedef struct {
    // Contains the state of the solver
    solver_status_st status;
} solver_st;

/* Create a solver object. Contains the solver state/memory */
solver_st* solver_create(void);

/* Destroy a solver object 
 * 
 * @param solver [in]: the solver object to destroy
 */
void solver_destroy(solver_st* solver);

/* Calls the fluid solver. 
 *
 * Assemble and solve the linear system corresponding to a given (@nodes, @elems) mesh. 
 * The @node_type hint how we should treat nodes: 
 *     - BULK: degrees of freedom in u, v, p
 *     - FREE_SURFACE: degrees of freedom in u, v. p=0
 *     - WALL: fully constrained (no-slip) node, u=v=0.
 * 
 * @param nodes: the coordinates of the nodes, in row major.
 *               Size = 2*nnodes
 * @param nnodes: the number of nodes
 * @param elems: the triangle elements, in row major. 
 *               Size = 3*nelems
 * @param nelems: the number of triangle elements
 * @param node_type: the type of each node (bulk, free-surface, wall). 
 *                   Size = nnodes
 * @param bnd_edges: the edges on the boundary, in row major. 
 *                   Size = 2*bnd_edges
 * @param n_bnd_edges: the number of edges on the boundary. 
 * @param props: the physical properties of the simulation
 * @param uvp [IN/OUT]: the interleaved velocities/pressure (u1, v1, p1, u2, v2, p2, ...). 
 *                      Allocated by the caller, filled by the function
 *                      Size=3*nnodes.
 * @return status: the status of the solver
 */
solver_status_st solve_navier_stokes(
    solver_st* solver,
    double   const * nodes, uint32_t nnodes, 
    uint32_t const * elems, uint32_t nelems, 
    node_type_et const * node_type, 
    uint32_t const * bnd_edges, uint32_t n_bnd_edges,
    physical_props_st const * props, double* uvp); 

#ifdef __cplusplus
};
#endif

#endif // __PROJECT_SOLVER_H__
