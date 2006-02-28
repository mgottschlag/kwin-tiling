kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kcontrol/pics FILES monitor.png )

kde4_footer()



#original Makefile.am contents follow:

#pics_DATA = monitor.png
#picsdir = $(kde_datadir)/kcontrol/pics
#
#EXTRA_DIST = $(pics_DATA)
#
