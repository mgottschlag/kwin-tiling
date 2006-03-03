kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/workspace/kcontrol/background/ )

set(libkrd_SRCS ${CMAKE_SOURCE_DIR}/workspace/kcontrol/krdb/krdb.cpp)

set( backgroundlib_SRCS 
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/background/bgrender.cpp 
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/background/bgsettings.cpp 
)



set( kcmbackgroundlib_SRCS 
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/background/bgdialog.cpp 
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/background/bgadvanced.cpp 
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/background/bgmonitor.cpp 
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/background/bgwallpaper.cpp 
)

kde4_automoc(${kcmbackgroundlib_SRCS})

kde4_add_ui3_files(kcmbackgroundlib_SRCS ${CMAKE_SOURCE_DIR}/workspace/kcontrol/background/bgdialog_ui.ui ${CMAKE_SOURCE_DIR}/workspace/kcontrol/background/bgwallpaper_ui.ui ${CMAKE_SOURCE_DIR}/workspace/kcontrol/background/bgadvanced_ui.ui )


if( X11_Xrandr_FOUND )
	add_subdirectory( randr )
endif(X11_Xrandr_FOUND )
add_subdirectory( applnk ) 
add_subdirectory( bell ) 
#add_subdirectory( background ) 
add_subdirectory( samba ) 
add_subdirectory( krdb ) 

#add_subdirectory( input ) 
#add_subdirectory( kdm ) 
add_subdirectory( locale ) 
add_subdirectory( launch ) 
add_subdirectory( dateandtime ) 
#add_subdirectory( style ) 
add_subdirectory( kded ) 
add_subdirectory( kcontrol ) 
add_subdirectory( knotify ) 
add_subdirectory( pics ) 
add_subdirectory( colors ) 
#add_subdirectory( energy ) 
#add_subdirectory( fonts ) 
add_subdirectory( icons ) 
add_subdirectory( screensaver ) 
add_subdirectory( crypto ) 
add_subdirectory( smserver ) 
add_subdirectory( keys ) 
add_subdirectory( componentchooser ) 
#add_subdirectory( xinerama ) 
add_subdirectory( kthememanager ) 
add_subdirectory( hardware ) 
add_subdirectory( accessibility ) 
add_subdirectory( infocenter ) 

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(ACCESS_SUBDIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KSMCARD_SUBDIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(FONTINST_SUBDIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: Readd input )" )
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: Readd kdm )" )
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: Readd style )" )
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: Readd energy )" )
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: Readd fonts )" )
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: Readd xinerama )" )



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
