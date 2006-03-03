kde4_header()

add_subdirectory( systemtray ) 
add_subdirectory( minipager ) 
add_subdirectory( taskbar ) 
add_subdirectory( run ) 
add_subdirectory( launcher ) 
add_subdirectory( naughty ) 
add_subdirectory( lockout ) 
add_subdirectory( menu ) 
add_subdirectory( media ) 
add_subdirectory( trash ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

## network
#SUBDIRS = systemtray minipager taskbar run launcher naughty lockout menu media trash
