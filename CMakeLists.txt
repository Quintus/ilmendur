# This file is part of the project.
#
# Copyright (C) 2020 Marvin Gülker and the project team
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

########################################
# Configuring CMake

cmake_minimum_required(VERSION 3.13)
project(RPG
  VERSION 0.0.1
  DESCRIPTION "A Role-Play Game."
  HOMEPAGE_URL "https://redmine.guelker.eu/projects/rpg"
  LANGUAGES CXX)

set(RPG_DEPS_SRC_DIR      ${CMAKE_SOURCE_DIR}/deps)
set(RPG_DEPS_INSTALL_DIR  ${CMAKE_BINARY_DIR}/deps)

########################################
# Configuring the compiler

# Use standards-compliant C++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate warnings
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wno-unused-parameter) # Ogre has lots of these
add_compile_options(-DGLFW_INCLUDE_NONE=1)
include_directories(${RPG_DEPS_INSTALL_DIR}/include)
include_directories(${RPG_DEPS_INSTALL_DIR}/include/OGRE/RenderSystems/GL3Plus)
include_directories(${RPG_DEPS_INSTALL_DIR}/include/OGRE/RTShaderSystem)
include_directories(${RPG_DEPS_INSTALL_DIR}/include/OGRE/Plugins/ParticleFX)
include_directories(${RPG_DEPS_INSTALL_DIR}/include/OGRE/Plugins/STBICodec)
include_directories(${RPG_DEPS_INSTALL_DIR}/include/OGRE)

########################################
# Dependencies

# Most dependencies are built statically
add_subdirectory(deps)

# System-specific libraries are linked dynamically from the host

if (WIN32)
  message(SEND_ERROR "Windows builds are not supported yet")
else()
  set(OpenGL_GL_PREFERENCE GLVND)

  find_package(X11 REQUIRED)
  find_package(Threads REQUIRED)
  find_package(OpenGL REQUIRED COMPONENTS OpenGL GLX)

  if (X11_Xrandr_FOUND)
    message(STATUS "Xrandr found: ${X11_Xrandr_LIB}")
  else()
    message(SEND_ERROR "Xrandr not found")
  endif()

  include_directories(${X11_INCLUDE_DIR})
endif()

########################################
# The game itself

file(GLOB_RECURSE rpg_sources "src/*.cpp")

add_executable(rpg ${rpg_sources})
add_dependencies(rpg ogre3d glfw)
target_link_libraries(rpg ${RPG_DEPENDENCIES})

if (WIN32)
  message(SEND_ERROR "Windows builds are not supported yet")
else()
  target_link_libraries(rpg OpenGL::GLX OpenGL::OpenGL ${X11_Xrandr_LIB} ${X11_LIBRARIES} Threads::Threads ${CMAKE_DL_LIBS})
endif()

########################################
# Installation configuration

include(GNUInstallDirs)

########################################
# Print summary

message(STATUS "--------- Build info summary: ---------")
message(STATUS "Build type:            ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix:        ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Binary directory:      ${CMAKE_INSTALL_BINDIR}")
message(STATUS "Shared data directory: ${CMAKE_INSTALL_DATADIR}")
message(STATUS "(relative pathes are relative to the install prefix)")
message(STATUS "---------------------------------------")
