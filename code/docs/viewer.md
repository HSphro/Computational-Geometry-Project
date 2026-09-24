# How the viewer works

The viewer displays 2D triangle meshes and edge lists, and is made for
watching an algorithm run: show the mesh, change it, show it again. GLFW
handles the window and user input; OpenGL 1.1 draws the geometry using simple
`glBegin`/`glEnd` calls. The public API is in
[project_display.h](../include/project_display.h), and the implementation is
in [project_display.c](../src/project_display.c).

## Try it

With GLFW and OpenGL installed, run these commands from `code/`:

```sh
make
./mesh 3   # triangle mesh coloured by a nodal field
./mesh 4   # boundary edges of the same mesh
./mesh 5   # the same mesh being built, one operation per step
```

## The three calls

| Call              | What it does                                                    |
| ----------------- | --------------------------------------------------------------- |
| `display_mesh()`  | Shows a triangle mesh as the next step, returns on space        |
| `display_edges()` | Shows a list of edges as the next step, returns on space        |
| `display_wait()`  | Keeps the last picture up until the window is closed, closes it |

The window opens at the first `display_mesh()` or `display_edges()` call and
stays open until `display_wait()`. Both display calls return `1` when you
press space, and `0` once the window has been closed.

To just look at a mesh:

```c
display_mesh(nodes, nnodes, elems, nelems, NULL, 0, 0);
display_wait();
```

To watch your algorithm, put the display call in your own loop:

```c
while (display_mesh(nodes, nnodes, elems, nelems, NULL, 0, 0)) {
    if (!myAlgorithmStep(mesh))   /* change the mesh as you like here */
        break;
    /* refresh nodes, nnodes, elems, nelems from the mesh */
}
display_wait();                   /* keep the final mesh on screen */
```

- **The arrays are copied.** Once a display call returns, you may change,
  reallocate or free them. This is what lets your loop modify the mesh.
- **Closing the window ends the loop.** After that every display call returns
  `0` at once, so the rest of your program runs without being shown.
  `display_wait()` then frees the viewer; a later display call opens a new
  window.
- **The view stays put between steps.** Pan, zoom and framing are kept, so
  the picture does not jump as the mesh changes. Press `r` to frame the
  current mesh again.
- **Hold space to run through the steps.** It goes at the keyboard repeat
  rate, about 30 steps per second. For a long run, only show every Nth step:
  `if (step % 100 == 0 && !display_mesh(...)) break;`.
- The window title shows the step number.

`buildDemoMesh()` in [exec/main.c](../exec/main.c) (`./mesh 5`) calls the
viewer after every split; with those calls removed it is the plain algorithm.

## Passing data to the viewer

`display_mesh(nodes, nnodes, elems, nelems, fields, nfields, render_field)`
takes flat arrays:

- `nodes`: coordinates as `x0, y0, x1, y1, ...` (`double`).
- `elems`: three node indices per triangle, in counter-clockwise order
  (`uint32_t`). Indices start at zero.
- `fields`: values grouped by node. Field `f` at node `v` is stored at
  `fields[nfields * v + f]`.
- `render_field`: the zero-based index of the field used for colouring.
  Pass `fields = NULL` for a plain grey mesh.

For example, this displays one triangle with one scalar value per node:

```c
#include "project_display.h"

int main(void) {
    double nodes[] = {0.0, 0.0, 1.0, 0.0, 0.0, 1.0};
    uint32_t elems[] = {0, 1, 2};
    double fields[] = {0.0, 0.5, 1.0};

    display_mesh(nodes, 3, elems, 1, fields, 1, 0);
    display_wait();
    return 0;
}
```

`display_edges(nodes, nnodes, edges, nedges)` uses the same coordinates, with
two `uint32_t` node indices per edge. It draws black lines.

The mesh data structure stores connectivity as `int`;
`showMesh()` in [exec/main.c](../exec/main.c) shows how to copy it into a
`uint32_t` array. If triangle connectivity is stored as `uint32_t` and exposed
by the getters with that type, it can be passed directly to `display_mesh()`
without a conversion copy.

## Controls

| Input            | Action                                               |
| ---------------- | ---------------------------------------------------- |
| Space            | Next step (hold it to run through the steps)         |
| Left-button drag | Pan                                                  |
| Scroll           | Zoom                                                 |
| `w`              | Toggle triangle outlines (`z` in azerty layout)      |
| `f`              | Toggle filled triangles                              |
| `h`              | Cycle through no arrows, arrows, arrows with numbers |
| `n`              | Toggle node indices                                  |
| `r`              | Reset pan and zoom, and frame the current mesh       |
| `q` or Escape    | Close the window (`a` in azerty layout)              |

The `w` and `f` keys apply to triangle meshes; edge lists always show their
lines. During `display_wait()`, space does nothing.

## Inside the viewer

`project_display.c` keeps its state in a few globals: `window` (the GLFW
window, `NULL` until the first call), `scene` (the copy of the last arrays
shown), `step` (the step counter) and `view` (pan, zoom, framing and the
toggles set by the keys). A display call goes through these functions:

1. **`show()`** opens the window if needed, and returns `0` at once if it has
   already been closed. `display_mesh()` and `display_edges()` both call it;
   they only differ in the number of nodes per element (3 or 2).
2. **`sceneSet()`** replaces `scene` with a copy of the arrays, keeping only
   the selected field (`fields[nfields * v + render_field]`), and computes
   the coordinate bounds and field range. On the first step, `viewReset()`
   frames the view on those bounds.
3. **`handleEvents()`** draws the scene, then sleeps until the next event
   (key, mouse, resize, ...). GLFW calls `onKey()`, `onScroll()` or
   `onButton()` for it, which update `view`, and the loop draws again. It
   returns when space is pressed or the window is closed; in
   `display_wait()` only closing ends it.
4. **`drawScene()`** draws one frame. `beginFrame()` sets a flat,
   aspect-ratio-correct view, centred on the frame and adjusted by pan and
   zoom. Then come the layers, according to the enabled controls: filled
   triangles, black outlines, blue half-edge arrows, and node labels. Field
   values are scaled between their minimum and maximum and mapped to a
   jet-like colour scale; OpenGL interpolates the node colours inside each
   triangle.

Half-edge arrows follow the triangle's node order: local side `k` of triangle
`t` runs from node `k` to node `(k + 1) % 3` and is numbered `3*t + k`, matching
[HeObject.h](../include/HeObject.h). Arrows sit inside their triangle so that
the two directions along a shared edge remain distinct. For an edge list,
arrows follow each stored node pair and labels use the edge index.

Numbers are drawn with a small built-in seven-segment font. They are most
useful on small meshes, where the labels stay readable; on big meshes they can
crowd the view. Feel free to improve the viewer and make your own adjustments.
