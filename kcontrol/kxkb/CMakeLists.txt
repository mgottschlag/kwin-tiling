kde4_header()

add_subdirectory( pics ) 
include_directories( ${KDE4_INCLUDES} )


########### next target ###############

set(kxkb_KDEINIT_SRCS 
   kxkb.cpp 
   rules.cpp 
   extension.cpp 
   pixmap.cpp )

kde4_automoc(${kxkb_KDEINIT_SRCS})

kde4_add_dcop_skels(kxkb_KDEINIT_SRCS kxkb.h )

kde4_add_kdeinit_executable( kxkb ${kxkb_KDEINIT_SRCS})

target_link_libraries(kdeinit_kxkb  ${KDE4_KDECORE_LIBS} xkbfile X11 Xext kdeui )

install_targets(${LIB_INSTALL_DIR} kdeinit_kxkb )

target_link_libraries( kxkb kdeinit_kxkb )
install_targets(/bin kxkb )

########### next target ###############

set(kcm_keyboard_PART_SRCS 
   extension.cpp 
   rules.cpp 
   kcmlayout.cpp 
   pixmap.cpp 
   kcmmisc.cpp )

kde4_automoc(${kcm_keyboard_PART_SRCS})

kde4_add_ui3_files(kcm_keyboard_PART_SRCS kcmlayoutwidget.ui kcmmiscwidget.ui )

kde4_add_plugin(kcm_keyboard ${kcm_keyboard_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_keyboard )

target_link_libraries(kcm_keyboard  ${KDE4_KDECORE_LIBS} xkbfile kio )

install_targets(${PLUGIN_INSTALL_DIR} kcm_keyboard )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES keyboard.desktop keyboard_layout.desktop )
install_files( ${SERVICES_INSTALL_DIR} FILES kxkb.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )
install(FILES kxkb_groups DESTINATION ${CMAKE_INSTALL_PREFIX}${CONFIG_INSTALL_DIR} )
kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= $(all_includes)
#METASOURCES = AUTO
#SUBDIRS = pics
#
## it's basicly impossible to get this done with all the X headers used ;(
#KDE_OPTIONS = nofinal
#
#bin_PROGRAMS = 
#lib_LTLIBRARIES = 
#kdeinit_LTLIBRARIES = kxkb.la
#kde_module_LTLIBRARIES = kcm_keyboard.la
#
#kcm_keyboard_la_SOURCES = extension.cpp rules.cpp kcmlayout.cpp pixmap.cpp kcmlayoutwidget.ui kcmmisc.cpp kcmmiscwidget.ui
#kcm_keyboard_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_keyboard_la_LIBADD = -lxkbfile $(XTESTLIB) $(LIB_KIO)
#
##kxkb_la_includedir = .
##kxkb_la_include_HEADERS = kxkb.h
#kxkb_la_SOURCES = kxkb.cpp rules.cpp extension.cpp pixmap.cpp kxkb.skel
#kxkb_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kxkb_la_LIBADD = -lxkbfile -lX11 -lXext $(LIB_KDEUI)
#
#xdg_apps_DATA = keyboard.desktop keyboard_layout.desktop
#
#install-data-local: 
#	$(mkinstalldirs) $(DESTDIR)$(kde_confdir)
#	$(INSTALL_DATA) $(srcdir)/kxkb_groups $(DESTDIR)$(kde_confdir)/kxkb_groups
#
#uninstall-local:
#	-rm -f $(DESTDIR)$(kde_confdir)/kxkb_groups
#
#servicesdir = $(kde_servicesdir)
#services_DATA = kxkb.desktop
#
#messages: rc.cpp
#	$(XGETTEXT) kxkb.cpp rules.cpp extension.cpp pixmap.cpp kxkbbindings.cpp -o $(podir)/kxkb.pot
#	$(XGETTEXT) rules.cpp kcmlayout.cpp pixmap.cpp kcmmisc.cpp rc.cpp kxkbbindings.cpp -o $(podir)/kcmlayout.pot 
#
#KDE_ICON=AUTO
