
# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
INCLUDE(UsePkgConfig)

PKGCONFIG(libUSB _libUSBIncDir _libUSBLinkDir _libUSBLinkFlags _libUSBCflags)



FIND_LIBRARY(LIBUSB_LIBRARY NAMES usb 
  PATHS
  ${_libUSBLinkDir}
  /usr/lib
  /usr/local/lib
)

set( LIBUSB_LIBRARIES ${LIBUSB_LIBRARY} )

if (LIBUSB_LIBRARIES)
   set( LIBUSB_FOUND TRUE)
endif (LIBUSB_LIBRARIES)



if (LIBUSB_FOUND)
  if (NOT libUSB_FIND_QUIETLY)
    message(STATUS "Found LIBUSB: ${LIBUSB_LIBRARIES}")
  endif (NOT libUSB_FIND_QUIETLY)
else (LIBUSB_FOUND)
  if (libUSB_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find LIBUSB")
  endif (libUSB_FIND_REQUIRED)
endif (LIBUSB_FOUND)

MARK_AS_ADVANCED(
   LIBUSB_LIBRARIES 
	)

