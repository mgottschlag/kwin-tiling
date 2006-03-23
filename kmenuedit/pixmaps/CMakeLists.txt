kde4_header()
include_directories(  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kmenuedit/icons FILES  )
install_files( ${DATA_INSTALL_DIR}/kcontroledit/icons FILES  )

kde4_footer()



#original Makefile.am contents follow:

#kmenuediticondir = $(kde_datadir)/kmenuedit/icons
#kmenuediticon_ICON = AUTO
#
#kcontrolediticondir = $(kde_datadir)/kcontroledit/icons
#kcontrolediticon_ICON = AUTO
