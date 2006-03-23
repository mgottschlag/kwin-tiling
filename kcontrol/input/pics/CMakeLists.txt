kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kcminput/pics FILES mouse_rh.png mouse_lh.png )

kde4_footer()



#original Makefile.am contents follow:

#kcminput_pics_data_DATA = mouse_rh.png mouse_lh.png
#kcminput_pics_datadir = $(kde_datadir)/kcminput/pics
#
