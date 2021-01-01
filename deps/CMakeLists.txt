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
# - For projects that are built with GNU Autotools, the
#   option `--with-sysroot' has to be passed to the configure
#   script and set to $RPG_DEPS_INSTALL_DIR.
# - For projects that are build with CMake, CMAKE_PREFIX_PATH
#   needs to be set, and sometimes project-specific options.
#   ExternalProject does not pick up the CMAKE_PREFIX_PATH
#   set globally below.
#
# After each call to ExternalProject, the required libraries
# should be added to the RPG_DEPENDENCIES list. Be careful to
# add libraries on which other libraries depend *before* those
# depending libraries. At the end of this file the RPG_DEPENDENCIES
# list is reversed so that proper linker order is maintained.

set(CMAKE_PREFIX_PATH ${RPG_DEPS_INSTALL_DIR})
include(ExternalProject)

ExternalProject_Add(zlib
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/zlib-1.2.11
  INSTALL_DIR ${RPG_DEPS_INSTALL_DIR}
  CONFIGURE_COMMAND ${RPG_DEPS_SRC_DIR}/zlib-1.2.11/configure
                    --prefix=${RPG_DEPS_INSTALL_DIR}
                    --static
  BUILD_COMMAND make
  INSTALL_COMMAND make install)
list(APPEND RPG_DEPENDENCIES
  ${RPG_DEPS_INSTALL_DIR}/lib/libz.a)

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
list(APPEND RPG_DEPENDENCIES
  ${RPG_DEPS_INSTALL_DIR}/lib/libpng16.a)

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
# No appending to RPG_DEPENDENCIES, because this library is not used as-is,
# see above

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
list(APPEND RPG_DEPENDENCIES
  ${RPG_DEPS_INSTALL_DIR}/lib/libicudata.a
  ${RPG_DEPS_INSTALL_DIR}/lib/libicuuc.a
  ${RPG_DEPS_INSTALL_DIR}/lib/libicui18n.a)

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
list(APPEND RPG_DEPENDENCIES
  ${RPG_DEPS_INSTALL_DIR}/lib/libharfbuzz.a
  ${RPG_DEPS_INSTALL_DIR}/lib/libharfbuzz-icu.a)

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
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/zziplib-0.13.71
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
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${RPG_DEPS_SRC_DIR}/ogre-1.12.10/Samples/Media ${RPG_DEPS_INSTALL_DIR}/share/OGRE/Media/
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${RPG_DEPS_SRC_DIR}/ogre-1.12.10/Media ${RPG_DEPS_INSTALL_DIR}/share/OGRE/Media/)
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
  SOURCE_DIR ${RPG_DEPS_SRC_DIR}/glfw-3.3.2
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
