kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kcmlocale/pics FILES background.png )

kde4_footer()



#original Makefile.am contents follow:

#pics_DATA = background.png
#picsdir = $(kde_datadir)/kcmlocale/pics
