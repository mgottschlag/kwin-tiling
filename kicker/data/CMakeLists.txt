kde4_header()

add_subdirectory( icons ) 
add_subdirectory( app_start_anim ) 
add_subdirectory( kmenu_side ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = icons app_start_anim kmenu_side
