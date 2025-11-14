cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0)

project(MyOpenGLProject)

# -----------------------------
# Compiler and general settings
# -----------------------------
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# -----------------------------
# Include paths
# -----------------------------
include_directories(${CMAKE_SOURCE_DIR}/includes)
include_directories(${CMAKE_BINARY_DIR}/configuration)
include_directories(${CMAKE_SOURCE_DIR}/src)

# -----------------------------
# Library linking
# -----------------------------
link_directories(${CMAKE_SOURCE_DIR}/lib)

# Core libraries
find_package(GLM REQUIRED)
message(STATUS "Using GLM at: ${GLM_INCLUDE_DIR}")

find_package(GLFW3 REQUIRED)
message(STATUS "Using GLFW at: ${GLFW3_INCLUDE_DIR}")

find_package(ASSIMP REQUIRED)
message(STATUS "Using Assimp at: ${ASSIMP_INCLUDE_DIR}")

if(APPLE)
  find_package(Freetype REQUIRED)
  message(STATUS "Using Freetype at: ${FREETYPE_INCLUDE_DIRS}")
endif()

# Platform-specific linking
if(WIN32)
  set(LIBS glfw3 opengl32 assimp freetype irrKlang)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT APPLE)
  find_package(OpenGL REQUIRED)
  find_package(X11 REQUIRED)
  add_definitions(${OPENGL_DEFINITIONS})
  set(LIBS ${GLFW3_LIBRARY} ${OPENGL_LIBRARIES} ${ASSIMP_LIBRARY} X11 Xrandr Xinerama Xi Xxf86vm Xcursor dl pthread freetype)
elseif(APPLE)
  find_library(COCOA_LIBRARY Cocoa)
  find_library(OpenGL_LIBRARY OpenGL)
  find_library(IOKit_LIBRARY IOKit)
  find_library(CoreVideo_LIBRARY CoreVideo)
  set(LIBS ${GLFW3_LIBRARY} ${ASSIMP_LIBRARY} ${FREETYPE_LIBRARIES}
           ${COCOA_LIBRARY} ${OpenGL_LIBRARY} ${IOKit_LIBRARY} ${CoreVideo_LIBRARY})
endif()

# -----------------------------
# Project source files (recursive)
# -----------------------------
file(GLOB_RECURSE PROJECT_SOURCES
    "src/*.cpp"
    "src/*.c"
    "src/*.h"
    "src/*.hpp"
)

# -----------------------------
# Static libraries for GLAD & STB_IMAGE
# -----------------------------
add_library(GLAD "src/glad.c")
add_library(STB_IMAGE "src/stb_image.cpp")
set(LIBS ${LIBS} GLAD STB_IMAGE)

configure_file(configuration/root_directory.h.in configuration/root_directory.h)
include_directories(${CMAKE_BINARY_DIR}/configuration)

# -----------------------------
# Create executable
# -----------------------------
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} ${LIBS})

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /std:c++17 /MP)
  target_link_options(${PROJECT_NAME} PUBLIC /ignore:4099)
endif()

# -----------------------------
# Copy shader files (*.vs, *.fs, etc.)
# -----------------------------
file(GLOB_RECURSE SHADERS
    "src/*.vs"
    "src/*.fs"
    "src/*.tcs"
    "src/*.tes"
    "src/*.gs"
    "src/*.cs"
)

foreach(SHADER ${SHADERS})
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SHADER}"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
    COMMENT "Copying shader: ${SHADER} to target directory"
  )
endforeach()

# -----------------------------
# Build summary
# -----------------------------
message(STATUS "")
message(STATUS "-----------------------------------------")
message(STATUS "Project:        ${PROJECT_NAME}")
message(STATUS "Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "Output Path:    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "-----------------------------------------")
