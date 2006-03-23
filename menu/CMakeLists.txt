kde4_header()

add_subdirectory( pics ) 
add_subdirectory( desktop ) 
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS=pics desktop
