/*
 * Small demo of the array-based half-edge data structure.
 *
 *   ./mesh 1   default triangle -> split one edge -> split one face
 *   ./mesh 2   build the structure from a hand written node/triangle list
 */

#include "HeObject.h"
#include "HeOperations.h"
#include "HeGetters.h"

#include <stdio.h>
#include <stdlib.h>


/* ========================================================================== */
/*  Printing, through the getters only                                        */
/* ========================================================================== */

static void printMesh(const Mesh* mesh, const char* title)
{
    int nNodes, nTriangles, nHe;
    const double* xy   = getNodes(mesh, &nNodes);
    const int* tri     = getTriangles(mesh, &nTriangles);
    const int* start   = getStartHE(mesh, NULL);
    const int* origin  = getHeOrigin(mesh, &nHe);
    const int* next    = getHeNext(mesh, NULL);
    const int* twin    = getHeTwin(mesh, NULL);

    printf("\n===== %s =====\n", title);
    printf("  %d nodes (room for %d), %d triangles (room for %d)\n",
           nNodes, mesh->allocNodes, nTriangles, mesh->allocTriangles);

    printf("\n  nodes                      triangles\n");
    printf("  idx      x      y  startHE    idx   nodes\n");
    for (int i = 0; i < nNodes || i < nTriangles; i++) {
        if (i < nNodes)
            printf("  %3d %6.3f %6.3f  %5d ", i, xy[2*i], xy[2*i + 1], start[i]);
        else
            printf("                            ");

        if (i < nTriangles)
            printf("    %3d   %2d %2d %2d",
                   i, tri[3*i], tri[3*i + 1], tri[3*i + 2]);
        printf("\n");
    }

    printf("\n  half-edges\n");
    printf("   he  tri  origin -> dest   next   twin\n");
    for (int h = 0; h < nHe; h++) {
        printf("  %3d  %3d  %6d -> %4d  %5d  %5d", h, h/3,
               origin[h], origin[next[h]], next[h], twin[h]);
        if (twin[h] == HE_BOUNDARY) printf("   (boundary)");
        printf("\n");
    }

    int errors = meshCheck(mesh);
    printf("\n  meshCheck: %s\n", errors == 0 ? "ok" : "BROKEN");
}

// EXAMPLE : computes the mid point of an edge.
static void edgeMidpoint(const Mesh* mesh, int he, double* x, double* y)
{
    int node1 = mesh->origin[he];
    int node2 = mesh->origin[mesh->next[he]];
    *x = 0.5 * (mesh->coords[2*node1]+mesh->coords[2*node2]);
    *y = 0.5 * (mesh->coords[2*node1+1]+mesh->coords[2*node2+1]);
}

// EXAMPLE : computes the centroid of a face. This time, we get the arrays through getters. 
static void triangleCentroid(const Mesh* mesh, int t, double* x, double* y)
{
    const double* xy = getNodes(mesh, NULL);
    const int* tri = getTriangles(mesh, NULL);
    *x = *y = 0.0;
    for (int i = 0; i < 3; i++) {
        *x += xy[2*tri[3*t + i] + 0] / 3.0;
        *y += xy[2*tri[3*t + i] + 1] / 3.0;
    }
}


/* ========================================================================== */
/*  Case 1 : default triangle, then an edge split, then a face split          */
/* ========================================================================== */

static void demoSplits(void)
{
    double x, y;

    /* [1] the default triangle (0,0) (1,0) (0,1) */
    Mesh* mesh = meshInitDefault();

    /* [2] add a point that cuts an edge: the midpoint of half-edge 0 */
    int he = 0;
    edgeMidpoint(mesh, he, &x, &y);
    edgeSplit(mesh, he, x, y);

    /* [3] add a point that cuts a face: the centroid of triangle 1 */
    int tri = 1;
    triangleCentroid(mesh, tri, &x, &y);
    faceSplit(mesh, tri, x, y);
   
    printMesh(mesh, "Final mesh for demo1 :-).");
    /* Free the mesh when done with it! :-) */
    freeMesh(mesh);
}


/* ========================================================================== */
/*  Case 2 : build the structure from a hand written list                     */
/* ========================================================================== */

static void demoFromArrays(void)
{
    /*        3-----------2        a unit square, 5 nodes, 4 triangles.
     *        | \       / |        Node 4 is slightly off-centre so that the
     *        |   \   /   |        swap in step [3] is not degenerate.
     *        |     4     |
     *        |   /   \   |
     *        | /       \ |
     *        0-----------1
     */
    static const double coords[] = {0.00, 0.00,     // 0
                                    1.00, 0.00,     // 1
                                    1.00, 1.00,     // 2
                                    0.00, 1.00,     // 3
                                    0.45, 0.55};    // 4
    static const int triangles[] = {0, 1, 4,        // all counter-clockwise
                                    1, 2, 4,
                                    2, 3, 4,
                                    3, 0, 4};

    int nNodes = (int)(sizeof(coords) / (2 * sizeof(double)));
    int nTriangles = (int)(sizeof(triangles) / (3 * sizeof(int)));

    Mesh* mesh = meshInitFromArrays(coords, nNodes, triangles, nTriangles);
    printMesh(mesh, "[1] meshInitFromArrays()");

    /* [2] bonus: the third operation, on the interior edge of half-edge 1 */
    int he = 1;
    int status = edgeSwap(mesh, he);
    printf("\n>>> edgeSwap(he=%d) returned %d (%s)\n", he, status,
           status == 0 ? "swapped" :
           status == -1 ? "boundary edge" : "refused, non convex quad");
    printMesh(mesh, "[2] after edgeSwap");

    freeMesh(mesh);
}


/* ========================================================================== */

int main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s [1|2]\n"
                        "  1  default triangle, edge split, face split\n"
                        "  2  init from a node/triangle list\n", argv[0]);
        return EXIT_FAILURE;
    }

    switch (atoi(argv[1])) {
        case 1: demoSplits();     break;
        case 2: demoFromArrays(); break;
        default:
            fprintf(stderr, "unknown case '%s', expected 1 or 2\n", argv[1]);
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
