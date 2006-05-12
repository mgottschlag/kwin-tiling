include_directories(  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/ksplash/Themes/Simple FILES Theme.rc Preview.png )




#original Makefile.am contents follow:

#EXTRA_DIST=Theme.rc Preview.png
#
#ksplashml_pics_default_datadir = $(kde_datadir)/ksplash/Themes/Simple
#ksplashml_pics_default_data_DATA = $(EXTRA_DIST)
