#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "student_triangulation.h"
#include "project_display.h"

/* This is a program to help you visualize the first part of the project. 
 *
 * It is called the "vortex in a box" and is a classical test-case for research in 
 * surface recovery and surface tracking. 
 *
 * It is simply a set of nodes (forming a disk) that is advected through a divergence-free 
 * vortex. 
 *
 * You can use it as a baseline to test: 
 * - Your triangulation (and its speed)
 * - Your alpha-shape
 * - Your mesh adaptation
 *
 */

#ifdef _WIN32
    #include <windows.h>
#else
    #include <time.h>
    #include <errno.h>
#endif

void sleep_ms(int64_t milliseconds) {
#ifdef _WIN32
    Sleep((DWORD)milliseconds);
#else
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000L;

    // nanosleep can be interrupted by a signal; retry if so
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {
        continue;
    }
#endif
}

/* Advect the nodes using Range-Kutta 4
 *
 * @param nodes [in/out]: the nodes to advect. Size = 2*nnodes
 * @param nnodes [in]: the number of nodes
 * @param dt [in]: the time step
 *
 */
static void advect_node_rk4(double * const nodes, uint32_t nnodes, double dt, double t, double T) {

    for (uint32_t i = 0; i < nnodes; ++i) {
        double const x0 = nodes[2*i];
        double const y0 = nodes[2*i+1];
        double xk1 =  sinf(2.*M_PI*y0)*sinf(M_PI*x0)*sinf(M_PI*x0)*sinf(2.*M_PI*t/T);
        double yk1 = -sinf(2.*M_PI*x0)*sinf(M_PI*y0)*sinf(M_PI*y0)*sinf(2.*M_PI*t/T);

        double xk2 =  sinf(2.*M_PI*(y0 + 0.5*dt*yk1))*sinf(M_PI*(x0 + 0.5*dt*xk1))*sinf(M_PI*(x0 + 0.5*dt*xk1))*sinf(2.*M_PI*(t+0.5*dt)/T);
        double yk2 = -sinf(2.*M_PI*(x0 + 0.5*dt*xk1))*sinf(M_PI*(y0 + 0.5*dt*yk1))*sinf(M_PI*(y0 + 0.5*dt*yk1))*sinf(2.*M_PI*(t+0.5*dt)/T);

        double xk3 =  sinf(2.*M_PI*(y0 + 0.5*dt*yk2))*sinf(M_PI*(x0 + 0.5*dt*xk2))*sinf(M_PI*(x0 + 0.5*dt*xk2))*sinf(2.*M_PI*(t+0.5*dt)/T);
        double yk3 = -sinf(2.f*M_PI*(x0 + 0.5*dt*xk2))*sinf(M_PI*(y0 + 0.5*dt*yk2))*sinf(M_PI*(y0 + 0.5*dt*yk2))*sinf(2.*M_PI*(t+0.5*dt)/T);

        double xk4 =  sinf(2.*M_PI*(y0 + dt*yk3))*sinf(M_PI*(x0 + dt*xk3))*sinf(M_PI*(x0 + dt*xk3))*sinf(2.*M_PI*(t+dt)/T);
        double yk4 = -sinf(2.f*M_PI*(x0 + dt*xk3))*sinf(M_PI*(y0 + dt*yk3))*sinf(M_PI*(y0 + dt*yk3))*sinf(2.f*M_PI*(t+dt)/T);

        double const vx = (1.0/6.0)*(xk1 + 2.0*xk2 + 2.0*xk3 + xk4);
        double const vy = (1.0/6.0)*(yk1 + 2.0*yk2 + 2.0*yk3 + yk4);
        nodes[2*i] += dt*vx;
        nodes[2*i+1] += dt*vy;
    }
}


int main(int argc, char** argv) {

    uint32_t n = 1000;  // The number of nodes
    double T = 5.0;  // The time for a cycle
    double dt = 0.05;  // A single time step
    double fps = 60.0; // Frame per second
    
    srand(42); // seed the random number generator to always get the same nodes

    // Generation of the nodes: 
    //      disk of radius r centered at (0.5, 0.75)
    double * nodes = malloc(2*n*sizeof(*nodes));
    for (uint32_t i = 0; i < n; ++i) {
        double const r = 0.15 * (float) rand() / RAND_MAX;
        double const theta = 2.0*M_PI * (float) rand() / RAND_MAX;
        nodes[2*i] = 0.5 + r * cos(theta);
        nodes[2*i+1] = 0.75 + r * sin(theta);
    }

    mesh_st mesh = {.nodes = nodes, 
                    .elems = NULL, 
                    .nnodes = n, 
                    .nelems = 0};

    struct timespec t0, t1;
    for (double t = 0.0; t < T; t += dt) {
        timespec_get(&t0, TIME_UTC);

        mesh_triangulate(&mesh);
        display_mesh(mesh.nodes, mesh.nnodes, mesh.elems, mesh.nelems, NULL, 0, 0);
        advect_node_rk4(nodes, n, dt, t, T);

        timespec_get(&t1, TIME_UTC);
        double const ft = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec)*1e-9;
        // Frame was generated fast: wait a bit to ensure target fps)
        if (ft < 1.0/fps) {
            sleep_ms(1000.0 * (1.0/fps - ft));         
        }
    }
    display_wait();

    free(nodes);
    return EXIT_SUCCESS;
}
