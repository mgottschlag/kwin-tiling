kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install_files( ${LOCALE_INSTALL_DIR}/en_US FILES entry.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kcm_locale_data_DATA = entry.desktop
#kcm_locale_datadir = $(kde_locale)/en_US
