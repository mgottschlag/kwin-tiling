kde4_header()
include_directories( ${KDE4_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kcm_componentchooser FILES kcm_kemail.desktop kcm_terminal.desktop kcm_browser.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kcm_ktexteditor_DATA = kcm_kemail.desktop kcm_terminal.desktop kcm_browser.desktop
#kcm_ktexteditordir = $(kde_datadir)/kcm_componentchooser
#
