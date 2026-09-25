# LMECA2170 -- project skeleton

Starter code for the numerical geometry project.


--- 

# Disclaimers

> [!IMPORTANT] 
> The goal of this project is to build a fully functioning "Particle Finite Element Method" (PFEM) 
> implementation. We chose this topic because lots of us in the teaching team 
> finds this subject genuinely fun, and it overlaps nicely with the course material.
> However, since this **is not a numerical simulation course**, we provide 
> the functions needed to perform the simulation itself, so you can focus on 
> the geometry part. This means that if anything in the simulation code 
> breaks, **it is not your job to fix it: it is ours**. Contact us and 
> we will update this repository.


> [!IMPORTANT] 
> This repo is only meant to give you a general idea of the things 
> you are supposed to implement. You are free to modify **anything and everything**
> (including the simulation part) or even start from scratch. You can do the 
> project in C, C++ or Python (sorry for the Julia fans but we're not proficient 
> enough in it to evaluate you fairly).
>
> The repo contains a skeleton of the project in C because Nathan
> thinks you should do it in C. But he knows C can be scary so he's included a
> tutorial on how to call C functions from Python. This way you can use Python 
> for the high-level logic and still implement the performance-critical parts in C !
>
> Our objective is that every group builds a PFEM simulator, but it'll be funnier 
> for you if it is reasonably efficient so that you can make cool videos :)

---

## Layout

- `exec/main.c`: examples of mesh operations and viewer usage.
- `include/`: public headers, including the fluid solver API to implement.
- `src/`: implementations.
- `docs/`: documentation.

## Build

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

## Viewer

See [the viewer guide](docs/viewer.md) for the data format, a minimal example,
controls, and an overview of how rendering works.

If you've completed the [Build](#build) section successfully, you should be able 
to run the viewer examples:

```sh
./mesh 1   # default triangle, edge split, face split
./mesh 2   # initialise from node/triangle arrays, then swap an edge
./mesh 3   # view a mesh coloured by a nodal field
./mesh 4   # view the boundary edges of that mesh
./mesh 5   # watch that mesh being built, one operation per step
```

