kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kcontrol/pics FILES logo.png mini-world.png )

kde4_footer()



#original Makefile.am contents follow:

#
#pics_DATA = logo.png mini-world.png
#picsdir = $(kde_datadir)/kcontrol/pics
#
#EXTRA_DIST = $(pics_DATA)
#
