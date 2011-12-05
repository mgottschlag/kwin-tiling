#   Copyright (C) 2011 Ivan Cukic <ivan.cukic(at)kde.org>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License version 2,
#   or (at your option) any later version, as published by the Free
#   Software Foundation
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details
#
#   You should have received a copy of the GNU General Public
#   License along with this program; if not, write to the
#   Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

# cmake macro to see if we have libKActivities

# KACTIVITIES_INCLUDE_DIRS
# KACTIVITIES_LIBRARY
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set( _KActivities_FIND_QUIETLY  ${KActivities_FIND_QUIETLY} )
find_package( KActivities QUIET NO_MODULE )
set( KActivities_FIND_QUIETLY ${_KActivities_FIND_QUIETLY} )

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( KActivities DEFAULT_MSG KActivities_CONFIG )

mark_as_advanced(KACTIVITIES_INCLUDE_DIRS KACTIVITIES_LIBRARY)

