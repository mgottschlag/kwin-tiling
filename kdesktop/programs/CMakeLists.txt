kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdesktop/programs FILES xearth.desktop xglobe.desktop xplanet.desktop )

kde4_footer()



#original Makefile.am contents follow:

#progdir=$(kde_datadir)/kdesktop/programs
#prog_DATA = xearth.desktop xglobe.desktop xplanet.desktop
