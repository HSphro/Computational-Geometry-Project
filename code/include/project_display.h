#ifndef __PROJECT_DISPLAY_H__
#define __PROJECT_DISPLAY_H__

#include <stdint.h>

/* NOTE:
 * 
 * We'll probably need to add some state to the renderer, like: 
 *
 *     - struct renderer_t;
 *     - renderer_t* renderer_new;
 *     - void renderer_destroy;
 *     - void renderer_set_bg_col;
 *
 * And then passing around the renderer in the display calls
 *
 */


#ifdef __cplusplus
extern "C" {
#endif

/* Displays a triangle mesh
 *
 * @param nodes: the coordinates of the nodes, in row major. 
 *               Size = 2*nnodes
 * @param nnodes: the number of nodes (NOT the number of coordinates!)
 * @param elems: the triangles in the mesh, in row major. 
 *               Size = 3*nelems
 * @param nelems: the number of elements
 * @param fields: the different fields to visualize, stored in row major (e.g. x1, y1, z1, x2, y2, z2, ...). 
 *                Size = nnodes*nfields
 * @param nfields: the number of different fields that are stored.
 * @param render_field: the index of the field to render
 */ 
void display_mesh(
    double   const * nodes,  uint32_t nnodes,
    uint32_t const * elems,  uint32_t nelems, 
    double   const * fields, uint32_t nfields, 
    int render_field);

/* Displays a list of edges
 *
 * @param nodes: the coordinates of the nodes, in row major.  
 *               Size = 2*nnodes
 * @param nnodes: the number of nodes
 * @param edges: the edges to draw, in row major.  
 *               Size = 2*nedges
 * @param nedges: the number of edges
 */
void display_edges(
    double   const * nodes, uint32_t nnodes, 
    uint32_t const * edges, uint32_t nedges);

#ifdef __cplusplus
};
#endif

#endif // __PROJECT_DISPLAY_H__
