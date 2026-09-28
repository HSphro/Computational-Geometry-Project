#ifndef __STUDENT_TRIANGULATION_H__
#define __STUDENT_TRIANGULATION_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =========================== Structures ============================

// Minimal boundary structure 
typedef struct {
    // You can modify anything you need. e.g. tags for wall contact...
    double * nodes;  // Coordinates of the boundary nodes in row-major. 
                     // NOTE: this can contains nodes that are not in the boundary, 
                     //       as long as the @edges contains only boundary nodes
    uint32_t * edges; // The boundary edges
    uint32_t nnodes; // number of nodes
    uint32_t nedges; // number of edges
} boundary_st;

// Minimal mesh structure
typedef struct {
    // You can modify/add anything you need. e.g. half-edges, boundary, node type, size-field...
    double * nodes;    // Coordinates of the mesh nodes in row-major
    uint32_t * elems;  // Triangles of the mesh
    uint32_t nnodes;   // number of nodes
    uint32_t nelems;   // number of elems
} mesh_st;

// ============================= Functions ============================

/* Triangulate the points in space
 *
 * @param nodes [in]: the nodes to be triangulated
 * @param nnodes [in]: the number of nodes
 *
 * @return mesh: The mesh constructed from @nodes
 */
mesh_st mesh_triangulate(double * nodes, uint32_t nnodes);

/* Retrieve the boundary of a mesh.
 *
 * @param mesh [in]: the mesh
 *
 * @return bnd: The boundary the input mesh
 */
boundary_st mesh_get_boundary(mesh_st const * mesh);

/* Delete selected elements in the mesh.
 *
 * call example: mesh.nelems = mesh_delete_elements(mesh.elems, to_delete, mesh.nelems);
 *
 * @param elems [in/out]: The elements in the mesh (modified by the function)
 * @param to_delete [in]: The elements to delete in the mesh. 
 *                        If to_delete[i] = 0, the i-th element is kept.
 *                        if to_delete[i] = 1, the i-th element is deleted.
 *                        Size = nelems
 * @param nelems [in]: the number of elements in the mesh
 *
 * @return nelems_out: the remaining element in the mesh (i.e. the number of zeros in @to_delete)
 */
uint32_t mesh_delete_elements(
        uint32_t * elems, 
        uint8_t const * to_delete,
        uint32_t nelems);

/* Adapt a mesh according to a given size-field (e.g. Chew's algorithm)
 *
 * NOTE: these are pointer of pointers because you will need to resize (and thus reallocate)
 *       them. Take a look at "realloc(...)" : https://koor.fr/C/cstdlib/realloc.wp
 *       E.g: *nodes = realloc(*nodes, 2*new_size*sizeof(double));
 *
 *       If we gave only a pointer to the function, any reallocation would only 
 *       be seen from inside the function and not from the caller. Meaning that 
 *       the caller would get a freed @nodes afterward without explanation
 *
 * @param nodes [in/out]: On input, the nodes of the initial mesh
 *                        On output, the nodes of the adapted mesh (the added nodes are at the end of the array)
 * @param node_size [in/out]: On input, the prescribed size of the mesh at each node
 *                            On output, same but the added nodes get an interpolated value
 * @param elems [in/out]: On input, the elements of the initial mesh 
 *                        On output, the elements of the adapted mesh
 * @param nnodes [in/out]: On input, the number of nodes in the initial mesh
 *                         On output, the number of nodes in the adapted mesh
 * @param nelems [in/out]: On input, the number of elements in the initial mesh
 *                         On output, the number of elements in the initial mesh
 *
 * @return nadd: The number of added nodes in the mesh
 */
uint32_t mesh_adapt(
        double ** nodes, 
        double ** node_size,
        uint32_t ** elems, 
        uint32_t * nnodes,
        uint32_t * nelems);

#ifdef __cplusplus
};
#endif

#endif // __STUDENT_TRIANGULATION_H__
