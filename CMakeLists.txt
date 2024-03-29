# This file is part Ilmendur.
#
# Copyright (C) 2022 Marvin Gülker and the project team
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

cmake_minimum_required(VERSION 3.20)
project(ILMENDUR
  VERSION 0.0.1
  DESCRIPTION "Ilmendur - Heirs to the Elven sword. A Role-Play Game."
  HOMEPAGE_URL "https://redmine.guelker.eu/projects/ilmendur"
  LANGUAGES C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

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
find_package(PkgConfig REQUIRED)
include_directories(${ILMENDUR_DEPS_INSTALL_DIR}/include)

include(cmake/dependencies.cmake)

########################################
# Internationalization

set(ILMENDUR_GETTEXT_DOMAIN "Ilmendur")
if (ENABLE_NLS)
  file(GLOB po_files "data/translations/*.po")
  set(mofiles "")

  foreach(pofile ${po_files})
    cmake_path(GET pofile STEM polocale)
    cmake_path(SET mofile "${CMAKE_BINARY_DIR}")
    cmake_path(APPEND mofile "translations")
    cmake_path(APPEND mofile "${polocale}")
    cmake_path(APPEND mofile "LC_MESSAGES")
    cmake_path(SET catdir "${mofile}")
    cmake_path(APPEND mofile "${ILMENDUR_GETTEXT_DOMAIN}.mo")
    add_custom_command(OUTPUT "${mofile}"
      COMMAND ${CMAKE_COMMAND} -E make_directory "${catdir}"
      COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o "${mofile}" "${pofile}"
      DEPENDS "${pofile}")
    list(APPEND mofiles "${mofile}")
  endforeach()
  add_custom_target(translations SOURCES "${mofiles}")
  add_dependencies(ilmendur translations)
endif()

########################################
# Installation configuration

include(GNUInstallDirs)

# Binary executable
install(TARGETS ilmendur DESTINATION ${CMAKE_INSTALL_BINDIR})

# Game data.
install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/" DESTINATION ${CMAKE_INSTALL_DATADIR}/ilmendur)
install(DIRECTORY "${CMAKE_BINARY_DIR}/translations/" DESTINATION ${CMAKE_INSTALL_LOCALEDIR})

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
