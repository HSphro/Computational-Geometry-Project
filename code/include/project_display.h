#ifndef PROJECT_DISPLAY_H
#define PROJECT_DISPLAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The viewer shows a mesh, lets your code change it, then shows it again:
 *
 *   while (display_mesh(...)) {   // waits for space, 0 once the window closes
 *       if (!myAlgorithmStep(mesh))
 *           break;
 *   }
 *   display_wait();               // keep the final mesh on screen
 *
 * To just look at a mesh: display_mesh(...); display_wait();
 *
 * The window opens at the first display_*() call and stays open until
 * display_wait(). Each display_*() call returns when the user presses space
 * (holding space runs through the steps).
 * The arrays are copied: once the call returns you may change or free them.
 */

/* Displays a triangle mesh, as the next step
 *
 * A NULL fields pointer, zero nfields or an invalid render_field displays a
 * plain grey mesh.
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
 * @return 1 to go on, 0 if the window was closed (every later call then
 *         returns 0 at once, until display_wait())
 */
int display_mesh(
    double   const * nodes,  uint32_t nnodes,
    uint32_t const * elems,  uint32_t nelems,
    double   const * fields, uint32_t nfields,
    int render_field);

/* Displays a list of edges, as the next step
 *
 * @param nodes: the coordinates of the nodes, in row major.
 *               Size = 2*nnodes
 * @param nnodes: the number of nodes
 * @param edges: the edges to draw, in row major.
 *               Size = 2*nedges
 * @param nedges: the number of edges
 * @return 1 to go on, 0 if the window was closed, as for display_mesh()
 */
int display_edges(
    double   const * nodes, uint32_t nnodes,
    uint32_t const * edges, uint32_t nedges);


/* Displays a list of nodes, as the next step
 *
 * @param nodes: the coordinates of the nodes, in row major.
 *               Size = 2*nnodes
 * @param nnodes: the number of nodes
 * @return 1 to go on, 0 if the window was closed, as for display_mesh()
 */
int display_nodes(double const * nodes, uint32_t nnodes);

/* Keeps the last mesh or edges shown on screen until the user closes the
 * window, then frees the viewer. The next display_*() call opens a new
 * window. */
void display_wait(void);

#ifdef __cplusplus
};
#endif

#endif // PROJECT_DISPLAY_H
