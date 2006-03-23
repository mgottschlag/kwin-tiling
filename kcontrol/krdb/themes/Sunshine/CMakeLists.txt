kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kthememanager/themes/Sunshine FILES Sunshine.xml Sunshine.preview.png )

kde4_footer()



#original Makefile.am contents follow:

#
#kthememanager_theme_data_DATA = Sunshine.xml Sunshine.preview.png
#
#kthememanager_theme_datadir = $(kde_datadir)/kthememanager/themes/Sunshine
#
#EXTRA_DIST = $(kthememanager_theme_data_DATA)
