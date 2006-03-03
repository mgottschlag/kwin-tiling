kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${XDG_DIRECTORY_DIR} FILES kde-settings.menu kde-information.menu kde-kcontrol.menu)
install_files( ${XDG_DIRECTORY_DIR} FILES kde-information.directory  kde-settings-accessibility.directory  kde-settings-components.directory  kde-settings-desktop.directory  kde-settings.directory  kde-settings-hardware.directory  kde-settings-looknfeel.directory  kde-settings-network.directory  kde-settings-power.directory  kde-settings-security.directory  kde-settings-sound.directory  kde-settings-system.directory  kde-settings-webbrowsing.directory )

kde4_footer()



#original Makefile.am contents follow:

#xdg_menu_DATA = kde-settings.menu kde-information.menu
#
#xdg_mergedmenu_DATA = kde-kcontrol.menu
#xdg_mergedmenudir = $(xdg_menudir)/applications-merged
#
#xdg_directory_DATA = \
#kde-information.directory \
#kde-settings-accessibility.directory \
#kde-settings-components.directory \
#kde-settings-desktop.directory \
#kde-settings.directory \
#kde-settings-hardware.directory \
#kde-settings-looknfeel.directory \
#kde-settings-network.directory \
#kde-settings-power.directory \
#kde-settings-security.directory \
#kde-settings-sound.directory \
#kde-settings-system.directory \
#kde-settings-webbrowsing.directory
