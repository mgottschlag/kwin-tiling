kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kthememanager/themes/Plastik FILES Plastik.xml Plastik.preview.png )

kde4_footer()



#original Makefile.am contents follow:

#
#kthememanager_theme_data_DATA = Plastik.xml Plastik.preview.png
#
#kthememanager_theme_datadir = $(kde_datadir)/kthememanager/themes/Plastik
#
#EXTRA_DIST = $(kthememanager_theme_data_DATA)
