kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files(  FILES entry.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kcm_locale_data_DATA = entry.desktop
#kcm_locale_datadir = $(kde_locale)/en_US
