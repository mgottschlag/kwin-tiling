kde4_header()



########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kthememanager/themes/HighContrastDark FILES HighContrastDark.preview.png HighContrastDark.xml )

kde4_footer()



#original Makefile.am contents follow:

#
#kthememanager_theme_data_DATA = HighContrastDark.preview.png HighContrastDark.xml
#
#kthememanager_theme_datadir = $(kde_datadir)/kthememanager/themes/HighContrastDark
#
#EXTRA_DIST = $(kthememanager_theme_data_DATA)
