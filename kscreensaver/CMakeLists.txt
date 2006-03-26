kde4_header()

add_subdirectory( libkscreensaver ) 
add_subdirectory( applnk ) 
add_subdirectory( kblank_screensaver ) 
add_subdirectory( krandom_screensaver ) 
include_directories( ${KDE4_INCLUDES} )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = libkscreensaver applnk kblank_screensaver krandom_screensaver
#
