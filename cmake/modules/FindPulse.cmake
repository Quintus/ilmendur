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

pkg_check_modules(PKG_PULSE libpulse)
pkg_check_modules(PKG_PULSE_SIMPLE libpulse-simple)

find_path(PULSE_INCLUDE_DIR pulse/pulseaudio.h HINTS ${PKG_PULSE_INCLUDE_DIRS})

find_library(PULSE_LIBRARY pulse HINTS ${PKG_PULSE_LIBRARY_DIRS})
find_library(PULSE_SIMPLE_LIBRARY pulse-simple HINTS ${PKG_PULSE_SIMPLE_LIBRARY_DIRS})

set(Pulse_INCLUDE_DIRS ${PULSE_INCLUDE_DIR})
set(Pulse_LIBRARIES ${PULSE_LIBRARY} ${PULSE_SIMPLE_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Pulse DEFAULT_MSG Pulse_LIBRARIES Pulse_INCLUDE_DIRS)
mark_as_advanced(Pulse_LIBRARIES Pulse_INCLUDE_DIRS)
