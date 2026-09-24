# LMECA2170 -- project skeleton

Starter code for the numerical geometry project, including a half-edge mesh
data structure, mesh operations, and a 2D viewer.

## Layout

- `exec/main.c`: examples of mesh operations and viewer usage.
- `include/`: public headers, including the fluid solver API to implement.
- `src/`: implementations.
- `docs/`: documentation.

## Build and run

Requires a C compiler, CMake 3.16 or newer, and OpenGL. If GLFW is not
installed, CMake downloads and builds it the first time you configure.

- **Linux**: install `cmake` and `glfw` on Arch Linux, or `cmake`,
  `libglfw3-dev` and `libgl1-mesa-dev` on Debian/Ubuntu.
- **macOS**: install the Xcode command line tools (`xcode-select --install`)
  and CMake (`brew install cmake glfw`).
- **Windows**: install Visual Studio with the "Desktop development with C++"
  workload, which includes CMake. Run the commands below from the "Developer
  PowerShell for VS".

Run from `code/`:

```sh
mkdir build && cd build
cmake .. 
```

This creates the executable in the current directory (which should be `build/`). Run it with the number of an example:

```sh
./mesh 1   # default triangle, edge split, face split
./mesh 2   # initialise from node/triangle arrays, then swap an edge
./mesh 3   # view a mesh coloured by a nodal field
./mesh 4   # view the boundary edges of that mesh
./mesh 5   # watch that mesh being built, one operation per step
```

## Viewer

See [the viewer guide](docs/viewer.md) for the data format, a minimal example,
controls, and an overview of how rendering works.
