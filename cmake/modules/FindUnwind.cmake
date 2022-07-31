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

pkg_check_modules(PKG_UNWIND libunwind)

find_path(UNWIND_INCLUDE_DIR libunwind.h HINTS ${PKG_UNWIND_INCLUDE_DIRS})

find_library(UNWIND_LIBRARY unwind HINTS ${PKG_UNWIND_LIBRARY_DIRS})
find_library(UNWIND_GENERIC_LIBRARY unwind-generic HINTS ${PKG_UNWIND_LIBRARY_DIRS})

set(Unwind_INCLUDE_DIRS ${UNWIND_INCLUDE_DIR})
set(Unwind_LIBRARIES ${UNWIND_LIBRARY} ${UNWIND_GENERIC_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Unwind DEFAULT_MSG Unwind_LIBRARIES Unwind_INCLUDE_DIRS)
mark_as_advanced(Unwind_LIBRARIES Unwind_INCLUDE_DIRS)
