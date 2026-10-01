cmake_minimum_required(VERSION 3.16)

project(LMECA2170-Project LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # compile_commands.json for your autocompletion

## Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# =================== Find the required libraries ========================
find_package(OpenGL REQUIRED)  # libGL on Linux, opengl32 on Windows, OpenGL.framework on macOS

# Use the installed GLFW (apt, brew, vcpkg, ...) or download and build it
find_package(glfw3 3.3 QUIET)
if(NOT glfw3_FOUND)
    message(STATUS "GLFW not found, downloading it")
    include(FetchContent)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(glfw URL https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz)
    FetchContent_MakeAvailable(glfw)
endif()

# =================== Define the exec/libs ==============================

# ADD HERE ALL THE REQUIRED FILES YOU CREATE (1 per line)
set(STUDENT_PROJECT_FILES
    src/student_triangulation.c
    src/student_spatial_search.c
)

# ADD HERE ALL THE OTHER DIRECTORIES CONTAINING .h FILES (1 per line)
set(STUDENT_INCLUDE_DIRS
    include/
)

# These are the files required by the project (solver/display)
set(DEFAULT_PROJECT_FILES
    src/HeObject.c
    src/HeOperations.c
    src/HeGetters.c
    src/project_display.c
    src/project_solver.c
)

add_library(geonum SHARED)
target_sources(geonum PRIVATE ${STUDENT_PROJECT_FILES} ${DEFAULT_PROJECT_FILES})
target_include_directories(geonum PUBLIC ${STUDENT_INCLUDE_DIRS})  # Give access to the 'include/*.h' files

# Define private compilation option (given to the compiler just for this library)
if(MSVC)
    target_compile_options(geonum PRIVATE /W4)
    target_compile_definitions(geonum PRIVATE _CRT_SECURE_NO_WARNINGS)  # no "fopen is unsafe" warnings
else()
    target_compile_options(geonum PUBLIC -Wall -Wextra -pedantic -Wshadow)
endif()

# Link libraries to the mesh executable
target_link_libraries(geonum PUBLIC glfw OpenGL::GL)
if(UNIX)  # Linux and macOS, Windows has no separate math library
    target_link_libraries(geonum PUBLIC m)
endif()
if(APPLE)
    target_compile_definitions(geonum PRIVATE GL_SILENCE_DEPRECATION)  # OpenGL is deprecated on macOS
endif()

# Look for a CMakeLists.txt in ./exec/
add_subdirectory(exec/)

