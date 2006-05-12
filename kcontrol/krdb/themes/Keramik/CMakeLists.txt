kde4_header()



########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kthememanager/themes/Keramik FILES Keramik.xml Keramik.preview.png )

kde4_footer()



#original Makefile.am contents follow:

#
#kthememanager_theme_data_DATA = Keramik.xml Keramik.preview.png
#
#kthememanager_theme_datadir = $(kde_datadir)/kthememanager/themes/Keramik
#
#EXTRA_DIST = $(kthememanager_theme_data_DATA)
