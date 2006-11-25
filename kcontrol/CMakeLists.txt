
project(kcontrol)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/background/  )

macro_optional_find_package(Freetype)
macro_optional_find_package(OpenSSL)

set(libkxftconfig_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/fonts/kxftconfig.cpp )


if( X11_Xrandr_FOUND )
   add_subdirectory( randr )
endif(X11_Xrandr_FOUND )

if(X11_Xaccess_FOUND)
	add_subdirectory( kxkb )
endif(X11_Xaccess_FOUND)    
add_subdirectory( menus )
add_subdirectory( bell )
add_subdirectory( background )
add_subdirectory( samba )
add_subdirectory( krdb )

add_subdirectory( input )
add_subdirectory( kdm )
add_subdirectory( locale )
add_subdirectory( launch )
add_subdirectory( dateandtime )
add_subdirectory( style )
add_subdirectory( kded )
add_subdirectory( kcontrol )
add_subdirectory( knotify )
add_subdirectory( pics )
add_subdirectory( colors )
add_subdirectory( energy )
add_subdirectory( fonts )
add_subdirectory( icons )
add_subdirectory( screensaver )
if(OPENSSL_FOUND)
add_subdirectory( crypto )
endif(OPENSSL_FOUND)
add_subdirectory( smserver )
add_subdirectory( keys )
add_subdirectory( componentchooser )
add_subdirectory( xinerama )
add_subdirectory( kthememanager )
add_subdirectory( hardware )
add_subdirectory( dnssd )
if(X11_Xaccess_FOUND)
	add_subdirectory( accessibility )
endif(X11_Xaccess_FOUND)	
if( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
	message(STATUS "TODO: infocenter is disabled on NetBSD, does not compile yet.")
else( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
	add_subdirectory( infocenter )
endif( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
add_subdirectory( access )

if(FONTCONFIG_FOUND AND FREETYPE_FOUND)
	add_subdirectory( kfontinst )
endif(FONTCONFIG_FOUND AND FREETYPE_FOUND)	
	
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KSMCARD_SUBDIR)")



########### install files ###############

