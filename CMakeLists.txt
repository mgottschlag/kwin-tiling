kde4_header()

add_subdirectory( lib ) 
add_subdirectory( kwin ) 
add_subdirectory( kcontrol ) 
add_subdirectory( ksmserver ) 
add_subdirectory( ktip ) 
add_subdirectory( kpersonalizer ) 
add_subdirectory( ksysguard ) 
add_subdirectory( wallpapers ) 
add_subdirectory( menu ) 
add_subdirectory( kscreensaver ) 
add_subdirectory( kdesktop ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

### (C) 2005 Benjamin Meyer
#
#SUBDIRS = lib kwin kcontrol ksmserver ktip kpersonalizer ksysguard wallpapers menu kscreensaver kdesktop
