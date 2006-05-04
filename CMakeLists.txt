kde4_header()

set(kdmlib_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kdmlib/dmctl.cpp )

set(libkrd_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kcontrol/krdb/krdb.cpp)

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

include_directories( ${CMAKE_BINARY_DIR}/workspace/kicker/libkicker ${CMAKE_CURRENT_SOURCE_DIR}/kicker/libkicker ${CMAKE_CURRENT_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/libkonq )

add_subdirectory( lib ) 
add_subdirectory( kdmlib )
add_subdirectory( kwin ) 
add_subdirectory( ksmserver ) 
add_subdirectory( ktip ) 
add_subdirectory( kpersonalizer ) 
add_subdirectory( wallpapers ) 
add_subdirectory( menu ) 
add_subdirectory( kscreensaver ) 
add_subdirectory( kdesktop ) 
add_subdirectory( kicker)
add_subdirectory( ksysguard )
add_subdirectory( kcontrol )
add_subdirectory( ksystraycmd )
add_subdirectory( ksplashml )
if(X11_Xau_FOUND AND X11_Xdmcp_FOUND)
	add_subdirectory( kdm )
endif(X11_Xau_FOUND AND X11_Xdmcp_FOUND)

########### install files ###############
install(PROGRAMS startkde DESTINATION bin)


kde4_footer()



#original Makefile.am contents follow:

### (C) 2005 Benjamin Meyer
#
#SUBDIRS = lib kwin kcontrol ksmserver ktip kpersonalizer ksysguard wallpapers menu kscreensaver kdesktop
