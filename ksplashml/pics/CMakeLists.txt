
add_subdirectory( themes ) 
include_directories(  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/ksplash/pics FILES splash.png )




#original Makefile.am contents follow:

#SUBDIRS=themes
#EXTRA_DIST=splash.png
# 
#ksplashml_pics_datadir = $(kde_datadir)/ksplash/pics
#ksplashml_pics_data_DATA = $(EXTRA_DIST)
