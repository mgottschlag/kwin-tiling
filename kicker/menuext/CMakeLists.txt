kde4_header()

add_subdirectory( find ) 
add_subdirectory( kdeprint ) 
add_subdirectory( prefmenu ) 
add_subdirectory( recentdocs ) 
add_subdirectory( konsole ) 
add_subdirectory( konq-profiles ) 
add_subdirectory( remote ) 
add_subdirectory( system ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = find kdeprint prefmenu recentdocs konsole konq-profiles remote system
#
