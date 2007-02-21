include(ConfigureChecks.cmake)
configure_file(config-unix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-unix.h )

set(kdmlib_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kdmlib/dmctl.cpp )


set( backgroundlib_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/kcontrol/background/bgrender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kcontrol/background/bgsettings.cpp
)

set( kcmbackgroundlib_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/kcontrol/background/bgdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kcontrol/background/bgadvanced.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kcontrol/background/bgmonitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kcontrol/background/bgwallpaper.cpp
)

include_directories( ${CMAKE_BINARY_DIR}/workspace/kicker/libkicker ${CMAKE_CURRENT_SOURCE_DIR}/kicker/libkicker ${CMAKE_CURRENT_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/libkonq ${CMAKE_CURRENT_BINARY_DIR} )

# libs
add_subdirectory( lib )
add_subdirectory( kdmlib )
add_subdirectory( plasma/lib )

# core apps
add_subdirectory( kcheckpass )
add_subdirectory( kscreensaver )
add_subdirectory( ksmserver )
add_subdirectory( kcminit )
add_subdirectory( ksplashml )
add_subdirectory( ksysguard )
add_subdirectory( ktip )
add_subdirectory( klipper )
add_subdirectory( kmenuedit )
add_subdirectory( kwin )
add_subdirectory( plasma )
add_subdirectory( khotkeys )

# apps we need to figure out what to do with
add_subdirectory( kcontrol )
add_subdirectory( ksystraycmd )

# new apps that need approval
add_subdirectory( krunner )

# data
add_subdirectory( wallpapers )
add_subdirectory( menu )

# apps on the chopping block
add_subdirectory( kdesktop )
add_subdirectory( kicker)
add_subdirectory( kpager )

if(X11_Xau_FOUND AND X11_Xdmcp_FOUND)
	add_subdirectory( kdm )
else(X11_Xau_FOUND AND X11_Xdmcp_FOUND)
	MESSAGE(STATUS "Xau lib or Xdmcp lib was missing. kdm will not compile")
endif(X11_Xau_FOUND AND X11_Xdmcp_FOUND)

SET(krunner_DEPENDS "plasma")

########### install files ###############
install(PROGRAMS startkde DESTINATION ${BIN_INSTALL_DIR})
