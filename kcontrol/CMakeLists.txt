kde4_header()

project(kcontrol)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/background/  )

macro_optional_find_package(Freetype)

set(libkxftconfig_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/fonts/kxftconfig.cpp )


if( X11_Xrandr_FOUND )
   add_subdirectory( randr )
endif(X11_Xrandr_FOUND )

add_subdirectory( applnk ) 
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
add_subdirectory( crypto ) 
add_subdirectory( smserver ) 
add_subdirectory( keys ) 
add_subdirectory( componentchooser ) 
add_subdirectory( xinerama ) 
add_subdirectory( kthememanager ) 
add_subdirectory( hardware ) 
add_subdirectory( accessibility ) 
add_subdirectory( infocenter ) 
add_subdirectory( access )

if(FONTCONFIG_FOUND AND FREETYPE_FOUND)
	add_subdirectory( kfontinst )
endif(FONTCONFIG_FOUND AND FREETYPE_FOUND)	
	
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KSMCARD_SUBDIR)")



########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#if include_kcontrol_access
##ACCESS_SUBDIR=access
#endif
#
#if include_kcontrol_kfontinst
#FONTINST_SUBDIR=kfontinst
#endif
#
#if include_kcontrol_smartcard
#KSMCARD_SUBDIR=smartcard
#endif
#
#if include_kcontrol_randr
#RANDR_SUBDIR=randr
#endif
#
## readd kicker and taskbar later
## 'colors' and 'fonts' should come after 'krdb'
##
## FIXME : readd dnssd (and arts)
##
#SUBDIRS = applnk bell background samba krdb input kdm \
#	locale launch dateandtime style kded kcontrol knotify\
#	pics $(ACCESS_SUBDIR) colors energy fonts \
#	icons screensaver crypto smserver keys \
#	$(KSMCARD_SUBDIR) $(FONTINST_SUBDIR) $(RANDR_SUBDIR) \
#	componentchooser xinerama kthememanager hardware accessibility infocenter
