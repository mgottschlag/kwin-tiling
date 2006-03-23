kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kthememanager/themes/HighContrastDark-big FILES HighContrastDark-big.preview.png HighContrastDark-big.xml )

kde4_footer()



#original Makefile.am contents follow:

#
#kthememanager_theme_data_DATA = HighContrastDark-big.preview.png HighContrastDark-big.xml
#
#kthememanager_theme_datadir = $(kde_datadir)/kthememanager/themes/HighContrastDark-big
#
#EXTRA_DIST = $(kthememanager_theme_data_DATA)
