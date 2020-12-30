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

# This cmake file contains everything related to building
# the dependencies.
#
# To prevent the dependencies' build systems from choosing
# the host system's libraries, it is necessary to configure
# them to look into $RPG_DEPS_INSTALL_DIR instead of the
# system's search pathes; $RPG_DEPS_INSTALL_DIR is set in
# the toplevel CMake file. This can be achieved as follows:
#
# - For projects that are built with GNU Autotools, the
#   option `--with-sysroot' has to be passed to the configure
#   script and set to $RPG_DEPS_INSTALL_DIR.
# - For projects that are build with CMake, nothing needs
#   to be done. CMake searches $CMAKE_PREFIX_PATH, and this
#   variable is set below.

########################################
# Source and installation pathes for dependencies

set(RPG_DEPS_SRC_DIR      ${CMAKE_SOURCE_DIR}/deps)
set(RPG_DEPS_INSTALL_DIR  ${CMAKE_BINARY_DIR}/deps)

########################################
# Ensure the compiler knows about the new directories created
# by compiling and installing the dependencies

set(CMAKE_PREFIX_PATH ${RPG_DEPS_INSTALL_DIR})
include_directories(${RPG_DEPS_INSTALL_DIR}/include)

########################################
# Commands for building the dependencies

include(ExternalProject)

ExternalProject_Add(zlib
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/zlib-1.2.11
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CONFIGURE_COMMAND ${RPG_DEPS_SRC_DIR}/zlib-1.2.11/configure
                    --prefix=${RPG_DEPS_INSTALL_DIR}
                    --static
  BUILD_COMMAND make
  INSTALL_COMMAND make install)

ExternalProject_Add(libpng
  DEPENDS zlib
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/libpng-1.6.37
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CONFIGURE_COMMAND ${RPG_DEPS_SRC_DIR}/libpng-1.6.37/configure
                     --prefix=${RPG_DEPS_INSTALL_DIR}
                     --with-sysroot=${RPG_DEPS_INSTALL_DIR}
                     --enable-static
                     --disable-shared
  BUILD_COMMAND make
  INSTALL_COMMAND make install)

# There is a interdependency of Freetype with Harfbuzz, see
# docs/INSTALL.UNIX in Freetype's source tree. It is necessary
# to first build freetype without Harfbuzz support, then build
# Harfbuzz, and then build Freetype again with Harfbuzz support.
ExternalProject_Add(freetype-stage1
  DEPENDS zlib libpng
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/freetype-2.10.4
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CONFIGURE_COMMAND ${RPG_DEPS_SRC_DIR}/freetype-2.10.4/configure
                     --srcdir=${RPG_DEPS_SRC_DIR}/freetype-2.10.4
                     --prefix=${RPG_DEPS_INSTALL_DIR}
                     --with-sysroot=${RPG_DEPS_INSTALL_DIR}
                     --enable-static
                     --disable-shared
                     --with-zlib=yes
                     --with-bzip2=no
                     --with-png=yes
                     --with-harfbuzz=no
                     --with-brotli=no)

ExternalProject_Add(icu
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/icu-68.2
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CONFIGURE_COMMAND ${RPG_DEPS_SRC_DIR}/icu-68.2/source/configure
                    --prefix=${RPG_DEPS_INSTALL_DIR}
                    --with-sysroot=${RPG_DEPS_INSTALL_DIR}
                    --enable-static
                    --disable-shared
                    --disable-layoutex
                    --disable-tests
                    --disable-samples)

# Consider to add graphite2 as dependency when non-western language
# support is needed. It has no further dependencies as of 2020.
ExternalProject_Add(harfbuzz
  DEPENDS freetype-stage1 icu
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/harfbuzz-2.7.4
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CONFIGURE_COMMAND ${RPG_DEPS_SRC_DIR}/harfbuzz-2.7.4/configure
  --prefix=${RPG_DEPS_INSTALL_DIR}
  --with-sysroot=${RPG_DEPS_INSTALL_DIR}
  --enable-static
  --disable-shared
  --with-glib=no
  --with-gobject=no
  --with-cairo=no
  --with-fontconfig=no
  --with-icu=yes
  --with-graphite2=no
  --with-freetype=yes
  --with-uniscribe=no
  --with-gdi=no
  --with-directwrite=no
  --with-coretext=no)

ExternalProject_Add(freetype
  DEPENDS zlib libpng harfbuzz
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/freetype-2.10.4
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CONFIGURE_COMMAND ${RPG_DEPS_SRC_DIR}/freetype-2.10.4/configure
                     --srcdir=${RPG_DEPS_SRC_DIR}/freetype-2.10.4
                     --prefix=${RPG_DEPS_INSTALL_DIR}
                     --with-sysroot=${RPG_DEPS_INSTALL_DIR}
                     --enable-static
                     --disable-shared
                     --with-zlib=yes
                     --with-bzip2=no
                     --with-png=yes
                     --with-harfbuzz=yes
                     --with-brotli=no)

# Considerations:
# - RENDERSYSTEM_GL is the OpenGL2 renderer. This is not needed.
# - RENDERSYSTEM_GL3PLUS is the modern OpenGL renderer and used instead.
# - IMGUI is not used, so disable it.
# - Bites is Ogre's window and input setup library, hardwired
#   to use SDL for window and event management. This project does the
#   system interaction in another way, most notably to circumvent SDL's
#   large dependency tree.
# - Tools should be used from the host system. Enabling the tools would
#   add a dependency on Zziplib (zip support still needs to be expressly
#   suppressed by setting OGRE_CONFIG_ENABLE_ZIP to OFF).
# - The DOT_SCENE plugin adds a dependency on pugixml, thus disable it.
#   Reconsider if the plugin is needed. Note: The Bites component
#   would pull in the pugixml dependency.
# - NODELESS_POSITIONING is a legacy compatibility feature which is not
#   needed for new projects.
# - FreeImage is the better codec, but FreeImage is a nightmare to build.
#   Upstream insists on shipping and compiling their own versions of its
#   dependencies, which mismatch with the dependencies by other software.
#   It is necessary to patch that out, see for example ArchLinux' patch:
#   https://github.com/archlinux/svntogit-community/blob/09da72387f2d9d1596ae26dd59a2f349a9f3132b/trunk/freeimage-unbundle.patch
#   STBI performs poorly, but builds easily and has no further dependencies.
#   Ogre ships a copy of STBI's code itself (it's a one-file implementation in C,
#   plus a header file). This is build during Ogre's compilation. And, maybe,
#   after years of further development, maybe STB(I) is not as bad anymore
#   as one used to say.
# - Assimp is probably needed to load external 3D models. It has hairy
#   dependies (boost, because it wants to use boost::thread, boost::shared_ptr,
#   and similar things which today are in C++11), which can be worked around.
#   See if the project can do without for now.
ExternalProject_Add(ogre3d
  DEPENDS zlib freetype
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/ogre-1.12.10
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
             -DCMAKE_INSTALL_PREFIX=${RPG_DEPS_INSTALL_DIR}
             -DCMAKE_PREFIX_PATH=${RPG_DEPS_INSTALL_DIR}
             -DBUILD_SHARED_LIBS=OFF
             -DBUILD_STATIC_LIBS=ON
             -DOGRE_STATIC=ON
             -DOGRE_BUILD_DEPENDENCIES=OFF
             -DOGRE_DEPENDENCIES_DIR=${RPG_DEPS_INSTALL_DIR}
             -DOGRE_BUILD_SAMPLES=OFF
             -DOGRE_BUILD_TOOLS=OFF
             -DOGRE_BUILD_RENDERSYSTEM_GL=OFF
             -DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=ON
             -DOGRE_BUILD_COMPONENT_OVERLAY_IMGUI=OFF
             -DOGRE_BUILD_COMPONENT_BITES=OFF
             -DOGRE_BUILD_PLUGIN_DOT_SCENE=OFF
             -DOGRE_BUILD_PLUGIN_STBI=ON
             -DOGRE_BUILD_PLUGIN_FREEIMAGE=OFF
             -DOGRE_BUILD_PLUGIN_ASSIMP=OFF
             -DOGRE_CONFIG_ENABLE_ZIP=OFF
             -DOGRE_INSTALL_DOCS=OFF
             -DOGRE_INSTALL_SAMPLES=OFF
             -DOGRE_NODELESS_POSITIONING=OFF
             -DOGRE_ENABLE_PRECOMPILED_HEADERS=OFF)
