kde4_header()
include_directories( ${KDE4_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kthememanager/themes/YellowOnBlue FILES YellowOnBlue.preview.png YellowOnBlue.xml )

kde4_footer()



#original Makefile.am contents follow:

#
#kthememanager_theme_data_DATA = YellowOnBlue.preview.png YellowOnBlue.xml
#
#kthememanager_theme_datadir = $(kde_datadir)/kthememanager/themes/YellowOnBlue
#
#EXTRA_DIST = $(kthememanager_theme_data_DATA)
