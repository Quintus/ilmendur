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

include(ExternalProject)

file(MAKE_DIRECTORY ${ILMENDUR_DEPS_INSTALL_DIR}/lib)
file(CREATE_LINK ${ILMENDUR_DEPS_INSTALL_DIR}/lib ${ILMENDUR_DEPS_INSTALL_DIR}/lib64 SYMBOLIC)

########################################
# Definition of external projects

ExternalProject_Add(zlib
  URL "http://www.zlib.net/zlib-1.2.12.tar.gz"
  URL_HASH SHA256=91844808532e5ce316b3c010929493c0244f3d37593afd6de04f71821d5136d9
  SOURCE_DIR ${ILMENDUR_DEPS_SRC_DIR}/zlib
  INSTALL_DIR ${ILMENDUR_DEPS_INSTALL_DIR}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${ILMENDUR_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${ILMENDUR_DEPS_INSTALL_DIR}/lib CFLAGS=-I${ILMENDUR_DEPS_INSTALL_DIR}/include ${ILMENDUR_DEPS_SRC_DIR}/zlib/configure
                    --prefix=${ILMENDUR_DEPS_INSTALL_DIR}
                    --static
  BUILD_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${ILMENDUR_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${ILMENDUR_DEPS_INSTALL_DIR}/lib CFLAGS=-I${ILMENDUR_DEPS_INSTALL_DIR}/include $(MAKE)
  INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install)

ExternalProject_Add(libpng
  DEPENDS zlib
  URL "http://prdownloads.sourceforge.net/libpng/libpng-1.6.37.tar.gz?download"
  URL_HASH SHA256=daeb2620d829575513e35fecc83f0d3791a620b9b93d800b763542ece9390fb4
  SOURCE_DIR ${ILMENDUR_DEPS_SRC_DIR}/libpng
  INSTALL_DIR ${ILMENDUR_DEPS_INSTALL_DIR}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${ILMENDUR_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${ILMENDUR_DEPS_INSTALL_DIR}/lib CPPFLAGS=-I${ILMENDUR_DEPS_INSTALL_DIR}/include ${ILMENDUR_DEPS_SRC_DIR}/libpng/configure
                     --prefix=${ILMENDUR_DEPS_INSTALL_DIR}
                     --enable-static
                     --disable-shared
  BUILD_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_LIBDIR=${ILMENDUR_DEPS_INSTALL_DIR}/lib/pkgconfig LDFLAGS=-L${ILMENDUR_DEPS_INSTALL_DIR}/lib CFLAGS=-I${ILMENDUR_DEPS_INSTALL_DIR}/include $(MAKE)
  INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install)

ExternalProject_Add(sdl
  DEPENDS zlib
  URL "https://libsdl.org/release/SDL2-2.0.22.tar.gz"
  URL_HASH SHA256=fe7cbf3127882e3fc7259a75a0cb585620272c51745d3852ab9dd87960697f2e
  SOURCE_DIR ${ILMENDUR_DEPS_SRC_DIR}/sdl
  INSTALL_DIR ${ILMENDUR_DEPS_INSTALL_DIR}
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
             -DCMAKE_INSTALL_PREFIX=${ILMENDUR_DEPS_INSTALL_DIR}
             -DCMAKE_PREFIX_PATH=${ILMENDUR_DEPS_INSTALL_DIR}
             -DSDL_SHARED=OFF
             -DSDL_STATIC=ON)

ExternalProject_Add(pugixml
  URL "https://github.com/zeux/pugixml/releases/download/v1.11.4/pugixml-1.11.4.tar.gz"
  URL_HASH SHA256=8ddf57b65fb860416979a3f0640c2ad45ddddbbafa82508ef0a0af3ce7061716
  SOURCE_DIR ${ILMENDUR_DEPS_SRC_DIR}/pugixml
  INSTALL_DIR ${ILMENDUR_DEPS_INSTALL_DIR}
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
             -DCMAKE_INSTALL_PREFIX=${ILMENDUR_DEPS_INSTALL_DIR}
             -DCMAKE_PREFIX_PATH=${ILMENDUR_DEPS_INSTALL_DIR}
             -DBUILD_SHARED_LIBS=OFF
             -DBUILD_STATIC_LIBS=ON
             -DBUILD_TESTS=OFF)

ExternalProject_Add(sdl_image
  DEPENDS sdl libpng
  URL "https://github.com/libsdl-org/SDL_image/releases/download/release-2.6.0/SDL2_image-2.6.0.tar.gz"
  URL_HASH SHA256=611c862f40de3b883393aabaa8d6df350aa3ae4814d65030972e402edae85aaa
  SOURCE_DIR ${ILMENDUR_DEPS_SRC_DIR}/sdl
  INSTALL_DIR ${ILMENDUR_DEPS_INSTALL_DIR}
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
             -DCMAKE_INSTALL_PREFIX=${ILMENDUR_DEPS_INSTALL_DIR}
             -DCMAKE_PREFIX_PATH=${ILMENDUR_DEPS_INSTALL_DIR}
             -DBUILD_SHARED_LIBS=OFF
             -DSDL2IMAGE_SAMPLES=OFF
             -DSDL2IMAGE_BACKEND_STB=OFF
             -DSDL2IMAGE_DEPS_SHARED=OFF
             -DSDL2IMAGE_VENDORED=OFF
             -DSDL2IMAGE_PNG=ON
             -DSDL2IMAGE_BMP=ON
             -DSDL2IMAGE_AVIF=OFF
             -DSDL2IMAGE_GIF=OFF
             -DSDL2IMAGE_JPG=OFF
             -DSDL2IMAGE_JPG_SAVE=OFF
             -DSDL2IMAGE_JXL=OFF
             -DSDL2IMAGE_LBM=OFF
             -DSDL2IMAGE_PCX=OFF
             -DSDL2IMAGE_PNM=OFF
             -DSDL2IMAGE_QOI=OFF
             -DSDL2IMAGE_SVG=OFF
             -DSDL2IMAGE_TGA=OFF
             -DSDL2IMAGE_TIF=OFF
             -DSDL2IMAGE_WEBP=OFF
             -DSDL2IMAGE_XCF=OFF
             -DSDL2IMAGE_XPM=OFF
             -DSDL2IMAGE_XV=OFF)

########################################
# Get linking order right
# The most basic libraries must come last in the linking list.

add_dependencies(ilmendur sdl sdl_image pugixml)
target_link_libraries(ilmendur ${ILMENDUR_DEPS_INSTALL_DIR}/lib/libSDL2_image.a
                               ${ILMENDUR_DEPS_INSTALL_DIR}/lib/libSDL2.a)

# Determine SDL's remaining dynamic dependencies, i.e. system basic libraries.
# Effectively, calculate what sdl-config --libs would return and add that
# to compile options.
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
  # As it turns out, SDL on Linux uses dlopen(3) to load all of its
  # dependencies, so do not link them in here. SDL can be persuaded to
  # not do that by passing -DSDL_LOADSO=OFF during the build
  # configuration, but SDL will then crash at runtime on Wayland with
  # the error "Could not initialize OpenGL / GLES library". It does
  # work on X11, though.

  # 1. Display server
  find_package(Wayland)
  find_package(X11 COMPONENTS Xrandr Xinerama Xkb Xfixes Xcursor Xi Xxf86vm)
#  if (X11_FOUND)
#    target_link_libraries(ilmendur
#      X11::Xrandr X11::Xinerama X11::Xfixes X11::Xcursor
#      X11::Xi X11::Xss X11::Xxf86vm X11::Xkb ${X11_xkbcommon_LIB} X11::X11)
#  endif()
#  if (Wayland_FOUND)
#    target_link_libraries(ilmendur ${Wayland_LIBRARIES})
#  endif()
  if ((NOT X11_FOUND) AND (NOT Wayland_FOUND))
    message(SEND_ERROR "Either X11 or Wayland is required on non-Apple Unix.")
  endif()

  # 2. Audio system
  # There are other sound systems on Linux which SDL supports (OSS,
  # ALSA, Pipewire), but PulseAudio seems to be the current least
  # common denominator, so for now only require that one.
  find_package(Pulse REQUIRED)
  #target_link_libraries(ilmendur ${Pulse_LIBRARIES})

  # 3. Core system libraries
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  set(OpenGL_GL_PREFERENCE GLVND)
  find_package(Threads REQUIRED)
  find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL)
  target_link_libraries(ilmendur
    Threads::Threads
    ${CMAKE_DL_LIBS} rt)
endif()

target_link_libraries(ilmendur
  ${ILMENDUR_DEPS_INSTALL_DIR}/lib/libpugixml.a
  ${ILMENDUR_DEPS_INSTALL_DIR}/lib/libpng16.a
  ${ILMENDUR_DEPS_INSTALL_DIR}/lib/libz.a
  m)

########################################
# Misc

# inith (source files in tree)
add_compile_options(-DINI_ALLOW_MULTILINE=0 -DINI_HANDLER_LINENO=1)
