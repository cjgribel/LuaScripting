cmake_minimum_required(VERSION 3.14)

#
# Project
#
project(LUAScripting
  VERSION 1.0
  LANGUAGES CXX
)

message(STATUS "Starting CMake configuration")

#
# CPP
#
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

#
# Compiler
#
if(APPLE)
    add_compile_definitions(GL_SILENCE_DEPRECATION=1)
    message(STATUS "Apple platform detected, setting GL_SILENCE_DEPRECATION=1")
else()
    add_compile_definitions(GL_SILENCE_DEPRECATION=0)
    message(STATUS "Non-Apple platform detected")
endif()

# 'target_include_directories' if target specific
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/vec)

#
# OpenGL
#
find_package(OpenGL REQUIRED)
message(STATUS "Found OpenGL")
message(STATUS "OpenGL include dir: ${OPENGL_INCLUDE_DIR}")
message(STATUS "OpenGL libraries: ${OPENGL_LIBRARIES}")

#
# Git
#
find_package (Git REQUIRED)
message(STATUS "Found Git: ${GIT_EXECUTABLE}")

include(FetchContent)

#
# Fetch SDL
#
find_package(SDL2 QUIET)
if (NOT SDL2_FOUND)
    message(STATUS "SDL2 not found, using FetchContent to fetch it.")
    FetchContent_Declare(
        SDL2
        URL https://www.libsdl.org/release/SDL2-2.0.12.zip
    )
    FetchContent_MakeAvailable(SDL2)
    set(SDL2_LIBRARIES SDL2::SDL2)
    set(SDL2_INCLUDE_DIRS ${sdl2_SOURCE_DIR}/include)
else()
    message(STATUS "Found SDL2.")
    set(SDL2_LIBRARIES SDL2::SDL2)
    set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIRS})
endif()
include_directories(${SDL2_INCLUDE_DIRS})
#message(STATUS "Fetching SDL2")
#FetchContent_Declare(
#    sdl2
#    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
#    GIT_TAG        release-2.30.2 # April 2nd 2024
#)
#FetchContent_GetProperties(sdl2)
#if(NOT sdl2_POPULATED)
#    FetchContent_Populate(sdl2)
#    add_subdirectory(${sdl2_SOURCE_DIR} ${sdl2_BINARY_DIR})
#    message(STATUS "SDL2 populated and added to the build")
#endif()
#include_directories(${sdl2_SOURCE_DIR}/include)

#
# Fetch glew
# https://github.com/Perlmint/glew-cmake/blob/master/glew-cmake/fetch-content.cmake
#
message(STATUS "Fetching GLEW")
include(FetchContent)
FetchContent_Declare(
  glew
  GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
  GIT_TAG        origin/master
)
FetchContent_GetProperties(glew)
if(NOT glew_POPULATED)
  FetchContent_Populate(glew)
  add_subdirectory(${glew_SOURCE_DIR} ${glew_BINARY_DIR} EXCLUDE_FROM_ALL)
  message(STATUS "GLEW populated and added to the build")
endif()

#
# Fetch enTT (header-only)
#
message(STATUS "Fetching enTT")
FetchContent_Declare(
  entt
  GIT_REPOSITORY https://github.com/skypjack/entt.git
  GIT_TAG        v3.13.2
)
FetchContent_MakeAvailable(entt)
include_directories(${entt_SOURCE_DIR}/src) 
message(STATUS "enTT fetched and included")

#
# Fetch Dear ImGui
#
message(STATUS "Fetching Dear ImGui")
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG v1.89.9)
FetchContent_MakeAvailable(imgui)
include_directories(${imgui_SOURCE_DIR})
include_directories(${imgui_SOURCE_DIR}/backends)
message(STATUS "Dear ImGui fetched and included")

#
# Lua
#
# find_package(Lua REQUIRED)
# message(STATUS "Lua found")
# message(STATUS "Found Lua: ${LUA_VERSION_STRING}")
# message(STATUS "Lua include dir: ${LUA_INCLUDE_DIR}")
# message(STATUS "Lua library: ${LUA_LIBRARY}")

# sol2

#
# Targets
#

# Module1
message(STATUS "Creating executable target for SpaceInvaders")
add_executable(SpaceInvaders 
    SpaceInvaders/main.cpp 
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GLDebugMessageCallback.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ShapeRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vec/vec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vec/mat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vec/quat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vec/ray.cpp
    )

set_target_properties(SpaceInvaders PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/SpaceInvaders"
)
target_link_libraries(SpaceInvaders PRIVATE ${SDL2_LIBRARIES} libglew_static ${OPENGL_LIBRARIES})
#target_include_directories(Module1 PRIVATE ${imgui_SOURCE_DIR})
#target_include_directories(Module1 PRIVATE ${imgui_SOURCE_DIR}/backends)

# Post-build commands
add_custom_command(TARGET SpaceInvaders POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_FILE_DIR:SpaceInvaders>
)
message(STATUS "Post-build commands to copy SDL2 DLLs to Module1 folder")

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(TARGET SpaceInvaders PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    message(STATUS "Set Visual Studio debugger working directory")
endif()

# ModulOther targets ...

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Module1)
    message(STATUS "Set Visual Studio startup project to Module1")
endif()

# For 'make install'
#install(TARGETS Module1 Module2)
# message(STATUS "Install targets