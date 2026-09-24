# LMECA2170 -- project skeleton

Starter code for the numerical geometry project, including a half-edge mesh
data structure, mesh operations, and a 2D viewer.

## Layout

- `exec/main.c`: examples of mesh operations and viewer usage.
- `include/`: public headers, including the fluid solver API to implement.
- `src/`: implementations.
- `docs/`: documentation.

## Build and run

Requires a C compiler, Make, GLFW, and OpenGL. Install `glfw` on Arch Linux,
or `libglfw3-dev` and `libgl1-mesa-dev` on Debian/Ubuntu.

Run from `code/`:

```sh
make
./mesh 1   # default triangle, edge split, face split
./mesh 2   # initialise from node/triangle arrays, then swap an edge
./mesh 3   # view a mesh coloured by a nodal field
./mesh 4   # view the boundary edges of that mesh
./mesh 5   # watch that mesh being built, one operation per step
```

On macOS, set `LDLIBS` in the `Makefile` to `-lglfw -framework OpenGL -lm`.

## Viewer

See [the viewer guide](docs/viewer.md) for the data format, a minimal example,
controls, and an overview of how rendering works.
