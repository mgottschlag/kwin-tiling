kde4_header()

add_subdirectory( libkicker ) 
add_subdirectory( taskmanager ) 
add_subdirectory( taskbar ) 
add_subdirectory( kicker ) 
add_subdirectory( extensions ) 
add_subdirectory( applets ) 
add_subdirectory( menuext ) 
add_subdirectory( proxy ) 
add_subdirectory( data ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = libkicker taskmanager taskbar kicker extensions applets menuext proxy data
#
