kde4_header()

add_subdirectory( actions ) 
include_directories( ${KDE4_INCLUDES} )


########### install files ###############


kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#KDE_ICON = kdisknav package_favourite panel window_list kmenu kicker 
#
#SUBDIRS = actions
