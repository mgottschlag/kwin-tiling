include_directories(  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/ksplash/Themes/None FILES Theme.rc )




#original Makefile.am contents follow:

#EXTRA_DIST=Theme.rc
#
#ksplashml_pics_default_datadir = $(kde_datadir)/ksplash/Themes/None
#ksplashml_pics_default_data_DATA = $(EXTRA_DIST)
