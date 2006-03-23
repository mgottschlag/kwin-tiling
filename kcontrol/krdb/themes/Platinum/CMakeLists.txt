kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kthememanager/themes/Platinum FILES Platinum.xml Platinum.preview.png )

kde4_footer()



#original Makefile.am contents follow:

#
#kthememanager_theme_data_DATA = Platinum.xml Platinum.preview.png
#
#kthememanager_theme_datadir = $(kde_datadir)/kthememanager/themes/Platinum
#
#EXTRA_DIST = $(kthememanager_theme_data_DATA)
