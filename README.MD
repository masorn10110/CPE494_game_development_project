# LearnOpenGL (CMake Edition)

This repository is a cross-platform version of **LearnOpenGL** project.
It contains GLAD, stb_image, and CMake configuration to build all tutorials
without relying on Visual Studio project files.

## Demo clip video

https://github.com/user-attachments/assets/48993d46-a070-433c-9679-1475dc478a92

## Picture
![alt text](https://github.com/masorn10110/CPE494_game_development_project/blob/main/image/image.png)
![alt text](https://github.com/masorn10110/CPE494_game_development_project/blob/main/image/Screenshot%202025-12-17%20143146.png)

## 🧩 Requirements

- CMake >= 3.0
- C++17 compiler (GCC, Clang, or MSVC)
- OpenGL 3.3+ supported GPU
- Installed libraries:
  - GLFW3
  - GLM
  - Assimp
  - Freetype

## 🛠️ Build Instructions
### **Linux/Macos**

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
### **Windows**
```bash
# clone this repository
git clone --recurse-submodules https://github.com/masorn10110/CPE494_game_development_project.git
cd CPE494_game_development_project

# create build folder
mkdir build && cd build

# run cmake
cmake ..

# build
cmake --build . --config Release -j

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
├── resource/
├── src/
│   ├── glad.c
│   ├── stb_image.cpp
│   ├── main.cpp
│   └── ...
├── build/ (generated)
└── bin/ (generated)

```
## itch.io link
https://devil-hunters.itch.io/devil-hunters
