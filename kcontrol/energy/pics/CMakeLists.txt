kde4_header()



########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kcontrol/pics FILES energybig.png lo-energy.png )

kde4_footer()



#original Makefile.am contents follow:

#pics_DATA = energybig.png lo-energy.png
#picsdir = $(kde_datadir)/kcontrol/pics
#
#EXTRA_DIST = $(pics_DATA)
#
