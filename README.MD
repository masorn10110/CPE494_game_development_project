# LearnOpenGL (CMake Edition)

This repository is a cross-platform version of **LearnOpenGL** project.
It contains GLAD, stb_image, and CMake configuration to build all tutorials
without relying on Visual Studio project files.

## 🧩 Requirements

- CMake >= 3.0
- C++17 compiler (GCC, Clang, or MSVC)
- OpenGL 3.3+ supported GPU
- Installed libraries:
  - GLFW3
  - GLM
  - Assimp
  - Freetype (optional, for text rendering)

## 🛠️ Build Instructions

```bash
# clone this repository
git clone --recurse-submodules https://github.com/masorn10110/CPE494_game_development_project.git
cd CPE494_game_development_project

# create build folder
mkdir build && cd build

# run cmake
cmake ..

# build
make # u can add -j n for fast complie ex. make -j 8

```
Executables will be generated inside bin/ organized by chapter.

```
📁 Directory Structure
CPE494_game_development_project /
├── CMakeLists.txt
├── cmake/modules/
├── includes/
│   ├── glad/
│   ├── glm/
│   ├── learnopengl/
│   └── stb_image.h
├── src/
│   ├── glad.c
│   ├── stb_image.cpp
│   └── ...
└── bin/ (generated)

```