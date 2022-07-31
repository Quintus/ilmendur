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

pkg_check_modules(PKG_WAYLAND_CLIENT wayland-client)
pkg_check_modules(PKG_WAYLAND_CURSOR wayland-cursor)
pkg_check_modules(PKG_WAYLAND_EGL    wayland-egl)

find_path(WAYLAND_CLIENT_INCLUDE_DIR wayland-client.h HINTS ${PKG_WAYLAND_CLIENT_INCLUDE_DIRS})

find_library(WAYLAND_CLIENT_LIBRARY wayland-client HINTS ${PKG_WAYLAND_CLIENT_LIBRARY_DIRS})
find_library(WAYLAND_CURSOR_LIBRARY wayland-cursor HINTS ${PKG_WAYLAND_CURSOR_LIBRARY_DIRS})
find_library(WAYLAND_EGL_LIBRARY    wayland-egl    HINTS ${PKG_WAYLAND_EGL_LIBRARY_DIRS})

set(Wayland_INCLUDE_DIRS ${WAYLAND_CLIENT_INCLUDE_DIR})
set(Wayland_LIBRARIES ${WAYLAND_EGL_LIBRARY} ${WAYLAND_CURSOR_LIBRARY} ${WAYLAND_CLIENT_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Wayland DEFAULT_MSG Wayland_LIBRARIES Wayland_INCLUDE_DIRS)
mark_as_advanced(Wayland_LIBRARIES Wayland_INCLUDE_DIRS)
