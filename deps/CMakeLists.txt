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

# This cmake file contains everything related to building the static
# dependencies. The idea behind the dependency management is this:
# everything that is not a system-dependent library is build from
# scratch and linked in statically. This should elminate any problems
# with version mismatches, people being unable to build this or that
# dependency, etc, and also serves as a nice reminder to keep the
# dependency tree small. System-dependent libraries like Mesa or X11
# should not be linked in statically. They are taken care of in the
# toplevel CMakeLists.txt.
#
# To prevent the dependencies' build systems from choosing
# the host system's libraries, it is necessary to configure
# them to look into $RPG_DEPS_INSTALL_DIR instead of the
# system's search pathes; $RPG_DEPS_INSTALL_DIR is set in
# the toplevel CMake file. This can be achieved as follows:
#
# - For projects that are built with GNU Autotools,
#   it is necessary to provide three environment variables for
#   any command:
#     * CFLAGS set to -I=${RPG_DEPS_INSTALL_DIR}/include
#       - To have `configure' find the headers.
#     * LDFLAGS set to -L=${RPG_DEPS_INSTALL_DIR}/lib
#       - To have the linker find the library.
#     * PKG_CONFIG_LIBDIR set to ${RPG_DEPS_INSTALL_DIR}/lib/pkgconfig
#       - To have pkg-config(1) sub-calls look at the right place.
#   Passing environment variables via ExternalProject_Add
#   is not easily possible, see <http://cmake.3232098.n2.nabble.com/Environment-variables-and-ExternalProject-td6901280.html>.
#   It is possible to wrap by calling cmake(1) with -E env,
#   see <https://stackoverflow.com/a/62437353>.
#
# - For projects that are build with CMake, CMAKE_PREFIX_PATH needs to
#   be set by passing -DCMAKE_PREFIX_PATH=${RPG_DEPS_INSTALL_DIR}.
#   Starting from version 3.1, cmake configures pkg-config(1) subcalls
#   to use that prefix path automatically if CMAKE_PREFIX_PATH is set
#   (see <https://cmake.org/cmake/help/v3.17/module/FindPkgConfig.html>).
#   It probably uses PKG_CONFIG_PATH instead of PKG_CONFIG_LIBDIR
#   under the hood, but the difference is negligible (the former
#   overrides the default pkg-config search path, the latter
#   prepends to it). ExternalProject_Add does not automatically pick
#   up an existing set CMAKE_PREFIX_PATH.
#
# After each call to ExternalProject, the required libraries
# should be added to the RPG_DEPENDENCIES list. Be careful to
# add libraries on which other libraries depend *before* those
# depending libraries. At the end of this file the RPG_DEPENDENCIES
# list is reversed so that proper linker order is maintained.
#
# Note: see here for the $(MAKE) trick: <https://stackoverflow.com/a/33171336>
# It makes cmake use make's jobserver to parallelize builds
# with external projects. Sadly, that trick enforces use of
# make on Unix, i.e. it is non-portable.

include(ExternalProject)

ExternalProject_Add(zlib
  URL "http://www.zlib.net/zlib-1.2.11.tar.gz"
  URL_HASH SHA256=c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/zlib
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${RPG_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${RPG_DEPS_INSTALL_DIR}/lib CFLAGS=-I${RPG_DEPS_INSTALL_DIR}/include ${RPG_DEPS_SRC_DIR}/zlib/configure
                    --prefix=${RPG_DEPS_INSTALL_DIR}
                    --static
  BUILD_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${RPG_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${RPG_DEPS_INSTALL_DIR}/lib CFLAGS=-I${RPG_DEPS_INSTALL_DIR}/include $(MAKE)
  INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install)
list(APPEND RPG_DEPENDENCIES
  ${RPG_DEPS_INSTALL_DIR}/lib/libz.a)

ExternalProject_Add(libpng
  DEPENDS zlib
  URL "http://prdownloads.sourceforge.net/libpng/libpng-1.6.37.tar.gz?download"
  URL_HASH SHA256=daeb2620d829575513e35fecc83f0d3791a620b9b93d800b763542ece9390fb4
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/libpng
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${RPG_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${RPG_DEPS_INSTALL_DIR}/lib CFLAGS=-I${RPG_DEPS_INSTALL_DIR}/include ${RPG_DEPS_SRC_DIR}/libpng/configure
                     --prefix=${RPG_DEPS_INSTALL_DIR}
                     --enable-static
                     --disable-shared
  BUILD_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${RPG_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${RPG_DEPS_INSTALL_DIR}/lib CFLAGS=-I${RPG_DEPS_INSTALL_DIR}/include $(MAKE)
  INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install)
list(APPEND RPG_DEPENDENCIES
  ${RPG_DEPS_INSTALL_DIR}/lib/libpng16.a)

# There is a interdependency of Freetype with Harfbuzz, see
# docs/INSTALL.UNIX in Freetype's source tree. It is necessary
# to first build freetype without Harfbuzz support, then build
# Harfbuzz, and then build Freetype again with Harfbuzz support.
ExternalProject_Add(freetype-stage1
  DEPENDS zlib libpng
  URL "https://download.savannah.gnu.org/releases/freetype/freetype-2.10.4.tar.gz"
  URL_HASH SHA256=5eab795ebb23ac77001cfb68b7d4d50b5d6c7469247b0b01b2c953269f658dac
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/freetype
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${RPG_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${RPG_DEPS_INSTALL_DIR}/lib CFLAGS=-I${RPG_DEPS_INSTALL_DIR}/include ${RPG_DEPS_SRC_DIR}/freetype/configure
                     --srcdir=${RPG_DEPS_SRC_DIR}/freetype
                     --prefix=${RPG_DEPS_INSTALL_DIR}
                     --enable-static
                     --disable-shared
                     --with-zlib=yes
                     --with-bzip2=no
                     --with-png=yes
                     --with-harfbuzz=no
                     --with-brotli=no
  BUILD_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${RPG_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${RPG_DEPS_INSTALL_DIR}/lib CFLAGS=-I${RPG_DEPS_INSTALL_DIR}/include $(MAKE)
  INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install)
# No appending to RPG_DEPENDENCIES, because this library is not used as-is,
# see above

ExternalProject_Add(icu
  URL "https://github.com/unicode-org/icu/releases/download/release-68-2/icu4c-68_2-src.tgz"
  URL_HASH SHA256=c79193dee3907a2199b8296a93b52c5cb74332c26f3d167269487680d479d625
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/icu
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${RPG_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${RPG_DEPS_INSTALL_DIR}/lib CFLAGS=-I${RPG_DEPS_INSTALL_DIR}/include ${RPG_DEPS_SRC_DIR}/icu/source/configure
                    --prefix=${RPG_DEPS_INSTALL_DIR}
                    --enable-static
                    --disable-shared
                    --disable-layoutex
                    --disable-tests
                    --disable-samples
  BUILD_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${RPG_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${RPG_DEPS_INSTALL_DIR}/lib CFLAGS=-I${RPG_DEPS_INSTALL_DIR}/include $(MAKE)
  INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install)
list(APPEND RPG_DEPENDENCIES
  ${RPG_DEPS_INSTALL_DIR}/lib/libicudata.a
  ${RPG_DEPS_INSTALL_DIR}/lib/libicuuc.a
  ${RPG_DEPS_INSTALL_DIR}/lib/libicui18n.a)

# Consider to add graphite2 as dependency when non-western language
# support is needed. It has no further dependencies as of 2020.
ExternalProject_Add(harfbuzz
  DEPENDS freetype-stage1 icu
  URL "https://github.com/harfbuzz/harfbuzz/releases/download/2.8.0/harfbuzz-2.8.0.tar.xz"
  URL_HASH SHA256=a1079fd021adaf5683351f34587a255975cc3bcadba31246ae40ebebb95ecd42
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/harfbuzz
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${RPG_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${RPG_DEPS_INSTALL_DIR}/lib CFLAGS=-I${RPG_DEPS_INSTALL_DIR}/include ${RPG_DEPS_SRC_DIR}/harfbuzz/configure
  --prefix=${RPG_DEPS_INSTALL_DIR}
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
  --with-coretext=no
  BUILD_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${RPG_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${RPG_DEPS_INSTALL_DIR}/lib CFLAGS=-I${RPG_DEPS_INSTALL_DIR}/include $(MAKE)
  INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install)
list(APPEND RPG_DEPENDENCIES
  ${RPG_DEPS_INSTALL_DIR}/lib/libharfbuzz.a
  ${RPG_DEPS_INSTALL_DIR}/lib/libharfbuzz-icu.a)

# Freetype stage2 reuses the sources already downloaded by freetype stage 1.
# Hence, DOWNLOAD_COMMAND is an empty string. Leaving it out entirely is
# not possible, as cmake would complain.
ExternalProject_Add(freetype
  DEPENDS zlib libpng harfbuzz
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/freetype
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${RPG_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${RPG_DEPS_INSTALL_DIR}/lib CFLAGS=-I${RPG_DEPS_INSTALL_DIR}/include ${RPG_DEPS_SRC_DIR}/freetype/configure
                     --srcdir=${RPG_DEPS_SRC_DIR}/freetype
                     --prefix=${RPG_DEPS_INSTALL_DIR}
                     --enable-static
                     --disable-shared
                     --with-zlib=yes
                     --with-bzip2=no
                     --with-png=yes
                     --with-harfbuzz=yes
                     --with-brotli=no
  BUILD_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${RPG_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${RPG_DEPS_INSTALL_DIR}/lib CFLAGS=-I${RPG_DEPS_INSTALL_DIR}/include $(MAKE)
  INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install)
list(APPEND RPG_DEPENDENCIES
  ${RPG_DEPS_INSTALL_DIR}/lib/libfreetype.a
  ${RPG_DEPS_INSTALL_DIR}/lib/libharfbuzz.a)
# ^ Resolving cyclic dependency freetype/harfbuzz/freetype by specifying
# harfbuzz twice.

# This builds zziplib without any sample programmes.
# Enabling ZZIPCOMPAT is required, because the build
# otherwise fails (see https://github.com/gdraheim/zziplib/issues/109).
# It is possible to remove this dependency if only non-zipped
# models are used (most notably Ogre's included default models are
# all zipped). For that, it is necessary to pass OGRE_CONFIG_ENABLE_ZIP=OFF
# to Ogre's build system below.
ExternalProject_Add(zziplib
  DEPENDS zlib
  URL "https://github.com/gdraheim/zziplib/archive/refs/tags/v0.13.72.tar.gz"
  URL_HASH SHA256=93ef44bf1f1ea24fc66080426a469df82fa631d13ca3b2e4abaeab89538518dc
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/zziplib
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
             -DCMAKE_INSTALL_PREFIX=${RPG_DEPS_INSTALL_DIR}
             -DCMAKE_PREFIX_PATH=${RPG_DEPS_INSTALL_DIR}
             -DBUILD_SHARED_LIBS=OFF
             -DBUILD_STATIC_LIBS=ON
             -DZZIPBINS=OFF
             -DZZIPCOMPAT=ON
             -DZZIPDOCS=OFF
             -DZZIPSDL=OFF
             -DZZIPTEST=OFF
             -DZZIPWRAP=OFF)
list(APPEND RPG_DEPENDENCIES
  ${RPG_DEPS_INSTALL_DIR}/lib/libzzip-0.a)

# Considerations:
# - RENDERSYSTEM_GL is the OpenGL2 renderer. This is not needed.
# - RENDERSYSTEM_GL3PLUS is the modern OpenGL renderer and used instead.
# - IMGUI is not used, so disable it.
# - Bites is Ogre's window and input setup library, hardwired
#   to use SDL for window and event management. This project does the
#   system interaction in another way, most notably to circumvent SDL's
#   large dependency tree.
# - Tools should be used from the host system. They can add further
#   dependencies like Zziplib pugixml which would need to be built.
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
  DEPENDS zlib zziplib freetype
  URL "https://github.com/OGRECave/ogre/archive/v1.12.11.zip"
  URL_HASH SHA256=b37af7c6bec50f07126cad1c07538cfb2b1ae5093a0c1bc2f97326f004957bea
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/ogre
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
             -DOGRE_CONFIG_ENABLE_ZIP=ON
             -DOGRE_INSTALL_DOCS=OFF
             -DOGRE_INSTALL_SAMPLES=OFF
             -DOGRE_NODELESS_POSITIONING=OFF
             -DOGRE_ENABLE_PRECOMPILED_HEADERS=OFF)
# If the samples are turned off (which they are for dependency
# avoidance), Ogre's sample media are not installed at the
# installation step. At this early stage of development, they are
# however necessary to see if things render at all. Thus, manually
# install them. The following call to ExternalProject_Add_Step() can
# probably be removed once only our own models are used.
ExternalProject_Add_Step(ogre3d install-sample-media
  DEPENDEES build
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${RPG_DEPS_SRC_DIR}/ogre/Samples/Media ${RPG_DEPS_INSTALL_DIR}/share/OGRE/Media/
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${RPG_DEPS_SRC_DIR}/ogre/Media ${RPG_DEPS_INSTALL_DIR}/share/OGRE/Media/)
list(APPEND RPG_DEPENDENCIES
  ${RPG_DEPS_INSTALL_DIR}/lib/libOgreMainStatic.a
  ${RPG_DEPS_INSTALL_DIR}/lib/libOgreGLSupportStatic.a
  ${RPG_DEPS_INSTALL_DIR}/lib/libOgreMeshLodGeneratorStatic.a
  ${RPG_DEPS_INSTALL_DIR}/lib/libOgreOverlayStatic.a
  ${RPG_DEPS_INSTALL_DIR}/lib/libOgrePagingStatic.a
  ${RPG_DEPS_INSTALL_DIR}/lib/libOgrePropertyStatic.a
  ${RPG_DEPS_INSTALL_DIR}/lib/libOgreRTShaderSystemStatic.a
  ${RPG_DEPS_INSTALL_DIR}/lib/libOgreTerrainStatic.a
  ${RPG_DEPS_INSTALL_DIR}/lib/libOgreVolumeStatic.a
  ${RPG_DEPS_INSTALL_DIR}/lib/OGRE/libCodec_STBIStatic.a
  ${RPG_DEPS_INSTALL_DIR}/lib/OGRE/libPlugin_BSPSceneManagerStatic.a
  ${RPG_DEPS_INSTALL_DIR}/lib/OGRE/libPlugin_OctreeSceneManagerStatic.a
  ${RPG_DEPS_INSTALL_DIR}/lib/OGRE/libPlugin_OctreeZoneStatic.a
  ${RPG_DEPS_INSTALL_DIR}/lib/OGRE/libPlugin_ParticleFXStatic.a
  ${RPG_DEPS_INSTALL_DIR}/lib/OGRE/libPlugin_PCZSceneManagerStatic.a
  ${RPG_DEPS_INSTALL_DIR}/lib/OGRE/libRenderSystem_GL3PlusStatic.a)

# Wayland support can (hopefully) easily be added later by enabling it here.
ExternalProject_Add(glfw
  URL "https://github.com/glfw/glfw/releases/download/3.3.3/glfw-3.3.3.zip"
  URL_HASH SHA256=723087ad45b40cd333be7d1a2cd5e09a28facb7f3acdb69f3e5613bd20543977
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/glfw
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
             -DCMAKE_INSTALL_PREFIX=${RPG_DEPS_INSTALL_DIR}
             -DCMAKE_PREFIX_PATH=${RPG_DEPS_INSTALL_DIR}
             -DBUILD_SHARED_LIBS=OFF
             -DBUILD_STATIC_LIBS=ON
             -DGLFW_BUILD_EXAMPLES=OFF
             -DGLFW_BUILD_TESTS=OFF
             -DGLFW_BUILD_DOCS=OFF
             -DGLFW_USE_WAYLAND=OFF)
list(APPEND RPG_DEPENDENCIES
  ${RPG_DEPS_INSTALL_DIR}/lib/libglfw3.a)

# Reverse the dependencies list to have it fit for the linker
list(REVERSE RPG_DEPENDENCIES)
message(STATUS "Compiling these dependencies: ${RPG_DEPENDENCIES}")

# Promote to caller
set(RPG_DEPENDENCIES ${RPG_DEPENDENCIES} PARENT_SCOPE)
