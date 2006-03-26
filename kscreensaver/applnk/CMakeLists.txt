kde4_header()
include_directories( ${KDE4_INCLUDES} )


########### install files ###############

install_files( ${XDG_DIRECTORY_DIR}/menu FILES kde-screensavers.menu )
install_files( ${XDG_DIRECTORY_DIR}/menu FILES kde-merge-screensavers.menu )
install_files( ${XDG_DIRECTORY_DIR} FILES kde-system-screensavers.directory  )

kde4_footer()



#original Makefile.am contents follow:

#xdg_menu_DATA = kde-screensavers.menu
#
#xdg_mergedmenu_DATA = kde-merge-screensavers.menu
#xdg_mergedmenudir = $(xdg_menudir)/applications-merged
#
#xdg_directory_DATA = kde-system-screensavers.directory 
