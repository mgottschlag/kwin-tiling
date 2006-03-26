kde4_header()
include_directories( ${KDE4_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kthememanager/themes/Redmond FILES Redmond.xml Redmond.preview.png )

kde4_footer()



#original Makefile.am contents follow:

#
#kthememanager_theme_data_DATA = Redmond.xml Redmond.preview.png
#
#kthememanager_theme_datadir = $(kde_datadir)/kthememanager/themes/Redmond
#
#EXTRA_DIST = $(kthememanager_theme_data_DATA)
