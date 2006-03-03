kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/pics/ FILES kside.png kside_tile.png )

kde4_footer()



#original Makefile.am contents follow:

#kicker_kmenuside_pics_data_DATA = kside.png kside_tile.png
#
#kicker_kmenuside_pics_datadir = $(kde_datadir)/kicker/pics/
#
#EXTRA_DIST = $(kicker_kmenuside_pics_data_DATA)
#
