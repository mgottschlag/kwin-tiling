include_directories(  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/ksplash/Themes/Default/locolor FILES Theme.rc splash_active_bar.png splash_bottom.png splash_inactive_bar.png splash_top.png )




#original Makefile.am contents follow:

#EXTRA_DIST=Theme.rc splash_active_bar.png splash_bottom.png splash_inactive_bar.png splash_top.png
#
#ksplashml_pics_classic_locolor_datadir = $(kde_datadir)/ksplash/Themes/Default/locolor
#ksplashml_pics_classic_locolor_data_DATA = $(EXTRA_DIST)
