# cmake macro to see if we have libKActivities
#
# KACTIVITIES_INCLUDE_DIRS
# KACTIVITIES_LIBRARY
#
# Copyright (C) 2011 Ivan Cukic <ivan.cukic(at)kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set( _KActivities_FIND_QUIETLY  ${KActivities_FIND_QUIETLY} )
find_package( KActivities QUIET NO_MODULE )
set( KActivities_FIND_QUIETLY ${_KActivities_FIND_QUIETLY} )

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( KActivities DEFAULT_MSG KActivities_CONFIG )

mark_as_advanced(KACTIVITIES_INCLUDE_DIRS KACTIVITIES_LIBRARY)

