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
project(ILMENDUR
  VERSION 0.0.1
  DESCRIPTION "Ilmendur - Heirs to the Elven sword. A Role-Play Game."
  HOMEPAGE_URL "https://redmine.guelker.eu/projects/ilmendur"
  LANGUAGES C CXX)

########################################
# Flags and options

option(ENABLE_NLS "Enable native language support (most notably translations)" ON)

########################################
# Configuring the compiler

# Use standards-compliant C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wno-unused-parameter) # Ogre has lots of these
add_compile_options(-DGLFW_INCLUDE_NONE=1) # Ogre ships its own set of OpenGL headers

# Configuration options for nuklear
add_compile_options(-DNK_INCLUDE_FIXED_TYPES=1)
add_compile_options(-DNK_INCLUDE_DEFAULT_ALLOCATOR=1)
add_compile_options(-DNK_INCLUDE_VERTEX_BUFFER_OUTPUT=1)

if(UNIX)
  # Request POSIX.1-2008
  add_compile_options(-D_POSIX_C_SOURCE=200809L)
endif()

########################################
# Game source code

file(GLOB_RECURSE ilmendur_sources "src/*.cpp" "src/*.c" "src/*.hpp" "src/*.h")
add_executable(ilmendur ${ilmendur_sources})

########################################
# Dependencies

# This project builds most of its dependencies itself and
# links them in statically. The sources are stored
# in SRC_DIR and the compiled results are stored
# under INSTALL_DIR as configured below.
# CMAKE_PREFIX_PATH is set for completeness. find_package()
# operates on it, but all dependencies built automatically
# are included in ILMENDUR_DEPENDENCIES with an absolute path anyway.
set(ILMENDUR_DEPS_SRC_DIR      ${CMAKE_BINARY_DIR}/deps-source)
set(ILMENDUR_DEPS_INSTALL_DIR  ${CMAKE_BINARY_DIR}/deps)
set(CMAKE_PREFIX_PATH          ${ILMENDUR_DEPS_INSTALL_DIR})

# Instruct CMake to build the subdirectory deps/ and
# tell it what to do with the results. add_subdirectory(deps)
# results in $ILMENDUR_DEPENDENCIES being set in this scope.
include_directories(${ILMENDUR_DEPS_INSTALL_DIR}/include
                    ${ILMENDUR_DEPS_INSTALL_DIR}/include/bullet
                    ${ILMENDUR_DEPS_INSTALL_DIR}/include/freetype2
                    ${ILMENDUR_DEPS_INSTALL_DIR}/include/OGRE
                    ${ILMENDUR_DEPS_INSTALL_DIR}/include/OGRE/RenderSystems/GL3Plus
                    ${ILMENDUR_DEPS_INSTALL_DIR}/include/OGRE/RTShaderSystem
                    ${ILMENDUR_DEPS_INSTALL_DIR}/include/OGRE/Plugins/ParticleFX
                    ${ILMENDUR_DEPS_INSTALL_DIR}/include/OGRE/Plugins/STBICodec
                    ${ILMENDUR_DEPS_INSTALL_DIR}/include/OGRE/Plugins/DotScene
                    ${ILMENDUR_DEPS_INSTALL_DIR}/include/OGRE/Overlay)
add_subdirectory(deps)
add_dependencies(ilmendur ogre3d bullet glfw) # Doing it explicitely because ExternalProject targets don't work in target_link_libraries(), just like the result of gettext_process_po_files
target_link_libraries(ilmendur ${ILMENDUR_DEPENDENCIES} stdc++fs)

# System-specific libraries are linked dynamically from the host.
# Following is the configuration to find them and what to do
# with the find results.
if (WIN32)
  find_package(Threads REQUIRED)
  find_package(OpenGL REQUIRED COMPONENTS OpenGL GLX)

  target_link_libraries(ilmendur OpenGL::GLX OpenGL::GL Threads::Threads ${CMAKE_DL_LIBS})
  message(WARNING "Win32 support is experimental")
elseif (APPLE)
  set(THREADS_PREFER_PTHREAD_FLAG ON)

  find_package(Threads REQUIRED)
  find_package(OpenGL REQUIRED COMPONENTS OpenGL GLX)

  target_link_libraries(ilmendur OpenGL::GLX OpenGL::GL Threads::Threads ${CMAKE_DL_LIBS})
  message(WARNING "Apple support is experimental")
else() # That is, Linux or another good Unix
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  set(OpenGL_GL_PREFERENCE GLVND)

  find_package(X11 REQUIRED)
  find_package(Threads REQUIRED)
  find_package(OpenGL REQUIRED COMPONENTS OpenGL GLX)

  if (NOT X11_Xrandr_FOUND)
    message(SEND_ERROR "Xrandr not found")
  endif()

  include_directories(${X11_INCLUDE_DIR})
  target_link_libraries(ilmendur OpenGL::GLX OpenGL::GL ${X11_Xrandr_LIB} ${X11_LIBRARIES} Threads::Threads ${CMAKE_DL_LIBS})
endif()

########################################
# Installation configuration

include(GNUInstallDirs)

# Binary executable
install(TARGETS ilmendur DESTINATION ${CMAKE_INSTALL_BINDIR})

# Game data. Ensure this matches the expectations of
# OS::game_resource_dir() and OS::ogre_resource_dir().
install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/" DESTINATION ${CMAKE_INSTALL_DATADIR}/ilmendur)
install(DIRECTORY "${ILMENDUR_DEPS_SRC_DIR}/ogre/Media/" DESTINATION ${CMAKE_INSTALL_DATADIR}/ilmendur/ogre)

########################################
# Native language support

if (ENABLE_NLS)
  find_package(Gettext REQUIRED)
  find_package(Intl REQUIRED)
  target_link_libraries(ilmendur ${INTL_LIBRARIES})

  set(ILMENDUR_GETTEXT_DOMAIN "Ilmendur")
  file(GLOB po_files "data/translations/*.po")
  gettext_process_po_files(languages PO_FILES ${po_files})

  # gettext_process_po_files() places the .gmo files directly in ${CMAKE_BINARY_DIR}.
  # This is not how bindtextdomain() requires it. The `podev' custom target caters
  # for this, and as a bonus the `gettext-catalogues' directory created during the
  # build process allows the catalogues to be used when Ilmendur is run from the
  # build directory.
  add_custom_target(podev ${CMAKE_COMMAND} -DILMENDUR_GETTEXT_DOMAIN=${ILMENDUR_GETTEXT_DOMAIN} -DPO_FILES=${po_files} -P ${CMAKE_SOURCE_DIR}/cmake/copy_pofiles.cmake
    DEPENDS pofiles)
  add_dependencies(ilmendur podev)

  install(DIRECTORY "${CMAKE_BINARY_DIR}/gettext-catalogues/"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/ilmendur/translations)
endif()

########################################
# Build configuration header
# Forwards some build configuration to the source code

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  set(ILMENDUR_DEBUG_BUILD ON)
endif()

configure_file(${CMAKE_SOURCE_DIR}/src/buildconfig.hpp.in ${CMAKE_BINARY_DIR}/buildconfig.hpp)
include_directories(${CMAKE_BINARY_DIR})

########################################
# Print summary

message(STATUS "--------- Build info summary: ---------")
message(STATUS "Build type:            ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix:        ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Binary directory:      ${CMAKE_INSTALL_BINDIR}")
message(STATUS "Shared data directory: ${CMAKE_INSTALL_DATADIR}")
message(STATUS "(relative pathes are relative to the install prefix)")
message(STATUS "---------------------------------------")
