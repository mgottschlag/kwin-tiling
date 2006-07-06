


########### install files ###############

install( FILES kde-screensavers.menu kde-merge-screensavers.menu DESTINATION ${XDG_DIRECTORY_DIR}/menus/ )
install( FILES kde-merge-screensavers.menu DESTINATION ${XDG_DIRECTORY_DIR}/menus/applications-merged )
install( FILES kde-system-screensavers.directory DESTINATION ${XDG_DIRECTORY_DIR}  )




#original Makefile.am contents follow:

#xdg_menu_DATA = kde-screensavers.menu
#
#xdg_mergedmenu_DATA = kde-merge-screensavers.menu
#xdg_mergedmenudir = $(xdg_menudir)/applications-merged
#
#xdg_directory_DATA = kde-system-screensavers.directory 
