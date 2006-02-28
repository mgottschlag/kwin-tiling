kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kthememanager/themes/HighContrastLight-big FILES HighContrastLight-big.preview.png HighContrastLight-big.xml )

kde4_footer()



#original Makefile.am contents follow:

#
#kthememanager_theme_data_DATA = HighContrastLight-big.preview.png HighContrastLight-big.xml
#
#kthememanager_theme_datadir = $(kde_datadir)/kthememanager/themes/HighContrastLight-big
#
#EXTRA_DIST = $(kthememanager_theme_data_DATA)
