kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kcontrol/about FILES top-right-kcontrol.png  	main.html  	kcontrol.css )

kde4_footer()



#original Makefile.am contents follow:

#about_DATA = \
#	top-right-kcontrol.png \
#	main.html \
#	kcontrol.css
#
#aboutdir = $(kde_datadir)/kcontrol/about
