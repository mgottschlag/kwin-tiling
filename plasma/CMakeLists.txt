kde4_header()

project(plasma)

add_subdirectory( lib ) 
add_subdirectory( desktop ) 
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS=lib desktop
