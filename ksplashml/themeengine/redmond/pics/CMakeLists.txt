kde4_header()
include_directories(  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/ksplash/Themes/Redmond FILES Preview.png Background.png )

kde4_footer()



#original Makefile.am contents follow:

#EXTRA_DIST=Preview.png Background.png
#
#themedir = $(kde_datadir)/ksplash/Themes/Redmond
#theme_DATA = $(EXTRA_DIST)
