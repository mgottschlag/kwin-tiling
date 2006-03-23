kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdewizard/pics FILES wizard_small.png )

kde4_footer()



#original Makefile.am contents follow:

#ktip_pics_data_DATA = wizard_small.png
#ktip_pics_datadir = $(kde_datadir)/kdewizard/pics
