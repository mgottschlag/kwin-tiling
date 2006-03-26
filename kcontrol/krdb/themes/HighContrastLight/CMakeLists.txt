kde4_header()
include_directories( ${KDE4_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kthememanager/themes/HighContrastLight FILES HighContrastLight.preview.png HighContrastLight.xml )

kde4_footer()



#original Makefile.am contents follow:

#
#kthememanager_theme_data_DATA = HighContrastLight.preview.png HighContrastLight.xml
#
#kthememanager_theme_datadir = $(kde_datadir)/kthememanager/themes/HighContrastLight
#
#EXTRA_DIST = $(kthememanager_theme_data_DATA)
