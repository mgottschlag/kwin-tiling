kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### next target ###############

set(klipper_KDEINIT_SRCS dummy.cpp )

kde4_automoc(${klipper_KDEINIT_SRCS})

kde4_add_kdeinit_executable( klipper ${klipper_KDEINIT_SRCS})

target_link_libraries(kdeinit_klipper  ${QT_AND_KDECORE_LIBS} klipper_common )

install_targets(${LIB_INSTALL_DIR} kdeinit_klipper )

target_link_libraries( klipper kdeinit_klipper )
install_targets(/bin klipper )

########### next target ###############

set(klipper_panelapplet_PART_SRCS applet.cpp )

kde4_automoc(${klipper_panelapplet_PART_SRCS})

kde4_add_dcop_skels(klipper_panelapplet_PART_SRCS applet.h )

kde4_add_plugin(klipper_panelapplet ${klipper_panelapplet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} klipper_panelapplet )

target_link_libraries(klipper_panelapplet  ${QT_AND_KDECORE_LIBS} klipper_common )

install_targets(${LIB_INSTALL_DIR}/kde4 klipper_panelapplet )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES klipper.desktop )
install_files( /share/autostart FILES klipper.desktop )
install_files( ${DATA_INSTALL_DIR}/kicker/applets FILES klipper.desktop )
install_files( ${DATA_INSTALL_DIR}/kconf_update FILES klipperrc.upd klippershortcuts.upd )

kde4_install_icons( hicolor crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= $(all_includes)
#
#bin_PROGRAMS =
#lib_LTLIBRARIES =
#noinst_LTLIBRARIES = libklipper_common.la
#kdeinit_LTLIBRARIES = klipper.la
#kde_module_LTLIBRARIES = klipper_panelapplet.la
#
#CLEANFILES = dummy.cpp
#
#libklipper_common_la_LIBADD = $(LIB_KDEUI) $(LIB_KSYCOCA) $(LIB_XFIXES) -lkde3support
#libklipper_common_la_SOURCES = main.cpp toplevel.cpp urlgrabber.cpp configdialog.cpp \
#    toplevel.skel clipboardpoll.cpp history.cpp historyitem.cpp historystringitem.cpp klipperpopup.cpp popupproxy.cpp \
#    historyimageitem.cpp historyurlitem.cpp
#
#libklipper_common_la_METASOURCES = AUTO
#libklipper_common_la_LDFLAGS = $(all_libraries)
#
#klipper_la_LIBADD = libklipper_common.la 
#klipper_la_SOURCES = dummy.cpp
#klipper_la_LDFLAGS = $(all_libraries) -module -avoid-version
#
#klipper_panelapplet_la_LIBADD = libklipper_common.la
#klipper_panelapplet_la_SOURCES = applet.cpp applet.skel
#klipper_panelapplet_la_LDFLAGS = $(all_libraries) -module -avoid-version
#
#xdg_apps_DATA = klipper.desktop
#
#autostartdir = $(datadir)/autostart
#autostart_DATA = klipper.desktop
#
#appldir = $(kde_datadir)/kicker/applets
#appl_DATA = klipper.desktop
#
#install-data-local:
#	$(mkinstalldirs) $(DESTDIR)$(kde_confdir)
#	$(INSTALL_DATA) $(srcdir)/klipperrc.desktop $(DESTDIR)$(kde_confdir)/klipperrc
#
#KDE_ICON = klipper
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/klipper.pot
#
#dummy.cpp:
#	echo > dummy.cpp
#
## update script:
#updatedir = $(kde_datadir)/kconf_update
#update_DATA = klipperrc.upd klippershortcuts.upd
#update_SCRIPTS = klipper-1-2.pl klipper-kde31.sh
#
