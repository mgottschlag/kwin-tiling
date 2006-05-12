kde4_header()

add_subdirectory( core )
add_subdirectory( ui )
add_subdirectory( buttons )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/buttons
  ${CMAKE_SOURCE_DIR}/workspace/kdmlib
  ${CMAKE_SOURCE_DIR}/libkonq
  ${CMAKE_CURRENT_SOURCE_DIR}/core
  ${CMAKE_CURRENT_SOURCE_DIR}/ui
  
)


########### next target ###############

kde4_add_dcop_skels(libkicker_core_SRCS core/kicker.h core/panelextension.h )

kde4_add_ui3_files(libkicker_ui_SRCS ui/appletitem.ui ui/appletview.ui ui/nonKDEButtonSettings.ui)

kde4_add_dcop_skels(libkicker_ui_SRCS ui/k_mnu.h )

set(kicker_KDEINIT_SRCS ${libkicker_buttons_SRCS} ${libkicker_ui_SRCS} ${libkicker_core_SRCS} ${kdmlib_SRCS} )

kde4_automoc(${kicker_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kicker ${kicker_KDEINIT_SRCS})

target_link_libraries(kdeinit_kicker  ${KDE4_KIO_LIBS} kickermain ${KDE4_KUTILS_LIBS} kworkspace konq )

install_targets(${LIB_INSTALL_DIR} kdeinit_kicker )

target_link_libraries( kicker kdeinit_kicker )
install(TARGETS kicker DESTINATION bin)

########### next target ###############

set(kicker_reverseLayout_SRCS kicker-3.4-reverseLayout.cpp)

kde4_automoc(${kicker_reverseLayout_SRCS})

kde4_add_executable( kicker-3.4-reverseLayout ${kicker_reverseLayout_SRCS})

target_link_libraries( kicker-3.4-reverseLayout ${KDE4_KDEUI_LIBS})

install_targets(/lib/kconf_update_bin/ kicker-3.4-reverseLayout)

########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kcmkicker.desktop )
install_files( /share/autostart FILES panel.desktop )
install_files( ${KCONF_UPDATE_INSTALL_DIR} FILES kickerrc.upd )
install_files( ${KCONF_UPDATE_INSTALL_DIR} FILES kicker-3.1-properSizeSetting.pl kicker-3.5-taskbarEnums.pl kicker-3.5-kconfigXTize.pl )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#SUBDIRS = core ui buttons .
#
#bin_PROGRAMS =
#lib_LTLIBRARIES =
#kdeinit_LTLIBRARIES = kicker.la
#
#CLEANFILES = dummy.cpp
#
#kicker_la_LIBADD = core/libkicker_core.la  buttons/libkicker_buttons.la \
#	ui/libkicker_ui.la ../libkicker/libkickermain.la $(LIB_KIO) $(LIB_KUTILS) $(top_builddir)/workspace/lib/libkworkspace.la
#
#kicker_la_SOURCES = dummy.cpp
#kicker_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#
#autostart_DATA = panel.desktop
#autostartdir = $(datadir)/autostart
#
#xdg_apps_DATA = kcmkicker.desktop
#
#messages: rc.cpp
#	$(EXTRACTRC) ui/*ui >> rc.cpp
#	$(EXTRACTRC) core/*.kcfg >> rc.cpp
#	$(XGETTEXT) buttons/*.cpp core/*.cpp ui/*.cpp *.cpp -o $(podir)/kicker.pot
#
#dummy.cpp:
#	echo > $@
# 
#kconf_PROGRAMS = kicker-3.4-reverseLayout
#kconfdir = $(libdir)/kconf_update_bin
#
#kicker_3_4_reverseLayout_SOURCES = kicker-3.4-reverseLayout.cpp
#kicker_3_4_reverseLayout_LDADD   = $(LIB_QT) $(LIB_KDECORE)
#kicker_3_4_reverseLayout_LDFLAGS = $(all_libraries)
#
#updatedir = $(kde_datadir)/kconf_update
#update_DATA = kickerrc.upd
#update_SCRIPTS = kicker-3.1-properSizeSetting.pl kicker-3.5-taskbarEnums.pl kicker-3.5-kconfigXTize.pl
#
