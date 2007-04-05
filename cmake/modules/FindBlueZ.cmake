# - Try to find BlueZ
# Once done this will define
#
#  BLUEZ_FOUND - system has BlueZ

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2007, Will Stephenson, <wstephenson@kde.org>
# Copyright (c) 2007, Daniel Gollub, <dgollub@suse.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (NOT WIN32)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   INCLUDE(UsePkgConfig)
   PKGCONFIG(bluez _BlueZIncDir _BlueZLinkDir _BlueZLinkFlags _BlueZCflags)
ENDIF (NOT WIN32)

MESSAGE(STATUS "Found BlueZ")
SET(BLUEZ_FOUND TRUE)

IF (NOT BLUEZ_FOUND)
   IF (BLUEZ_FIND_REQUIRED)
      MESSAGE(SEND_ERROR "Could NOT find BlueZ")
   ENDIF (BLUEZ_FIND_REQUIRED)
ENDIF (NOT BLUEZ_FOUND)
