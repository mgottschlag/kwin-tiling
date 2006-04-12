kde4_header()
include_directories( ${CMAKE_SOURCE_DIR}/libkonq ${CMAKE_SOURCE_DIR}/workspace/kdmlib ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/buttons ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core ${CMAKE_SOURCE_DIR}/kicker/kicker/ui  ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui ${KDE4_INCLUDES} )


########### next target ###############

set(launcher_panelapplet_PART_SRCS  ${kdmlib_SRCS}  ${libkicker_core_SRCS} ${libkicker_buttons_SRCS} ${libkicker_ui_SRCS}
   quicklauncher.cpp 
   quickbutton.cpp 
   quickaddappsmenu.cpp 
   flowgridmanager.cpp 
   popularity.cpp 
   configdlg.cpp )

kde4_add_dcop_skels(launcher_panelapplet_PART_SRCS ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/kicker.h ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/panelextension.h )

kde4_add_ui3_files(launcher_panelapplet_PART_SRCS ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/appletitem.ui ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/appletview.ui ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/nonKDEButtonSettings.ui)


kde4_add_dcop_skels(launcher_panelapplet_PART_SRCS ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/k_mnu.h )


kde4_automoc(${launcher_panelapplet_PART_SRCS})

kde4_add_ui3_files(launcher_panelapplet_PART_SRCS configdlgbase.ui )

kde4_add_dcop_skels(launcher_panelapplet_PART_SRCS quicklauncher.h )

kde4_add_kcfg_files(launcher_panelapplet_PART_SRCS prefs.kcfgc )

kde4_add_plugin(launcher_panelapplet ${launcher_panelapplet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} launcher_panelapplet )

target_link_libraries(launcher_panelapplet DCOP ${KDE4_KDE3SUPPORT_LIBS}  konq kickermain )

install_targets(${LIB_INSTALL_DIR}/kde4 launcher_panelapplet )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES launcherapplet.kcfg )
install_files( ${DATA_INSTALL_DIR}/kicker/applets FILES quicklauncher.desktop )

kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker -I$(top_srcdir)/kicker/kicker/ui -I$(top_srcdir)/workspace/lib $(all_includes)
#
#kde_module_LTLIBRARIES = launcher_panelapplet.la
#
#launcher_panelapplet_la_SOURCES = quicklauncher.skel quicklauncher.cpp quickbutton.cpp quickaddappsmenu.cpp flowgridmanager.cpp popularity.cpp configdlgbase.ui prefs.kcfgc configdlg.cpp
#
#METASOURCES = AUTO
#noinst_HEADERS = quicklauncher.h quickbutton.h quickaddappsmenu.h easyvector.h quickbuttongroup.h flowgridmanager.h popularity.h configdlg.h
#
#lnkdir = $(kde_datadir)/kicker/applets
#lnk_DATA = quicklauncher.desktop
#
#EXTRA_DIST = $(lnk_DATA)
#
#launcher_panelapplet_la_LDFLAGS = -module $(KDE_RPATH) $(all_libraries) -avoid-version
#launcher_panelapplet_la_LIBADD = ../../kicker/core/libkicker_core.la  ../../kicker/buttons/libkicker_buttons.la \
#				../../kicker/ui/libkicker_ui.la ../../libkicker/libkickermain.la $(LIB_KIO) \
#				$(LIB_KSYCOCA) $(LIB_KDEUI)
#
#kde_kcfg_DATA = launcherapplet.kcfg
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o $(podir)/quicklauncher.pot
#
#srcdoc:
#	kdoc -a -p -H -d $$HOME/web/src/quicklauncher quicklauncher *.h -lqt -lkdecore -lkdeui
