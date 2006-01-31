INCLUDES= $(all_includes)

bin_PROGRAMS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES = libklipper_common.la
kdeinit_LTLIBRARIES = klipper.la
kde_module_LTLIBRARIES = klipper_panelapplet.la

CLEANFILES = dummy.cpp

libklipper_common_la_LIBADD = $(LIB_KDEUI) $(LIB_KSYCOCA) $(LIB_XFIXES)
libklipper_common_la_SOURCES = main.cpp toplevel.cpp urlgrabber.cpp configdialog.cpp \
    toplevel.skel clipboardpoll.cpp history.cpp historyitem.cpp historystringitem.cpp klipperpopup.cpp popupproxy.cpp \
    historyimageitem.cpp

libklipper_common_la_METASOURCES = AUTO
libklipper_common_la_LDFLAGS = $(all_libraries)

klipper_la_LIBADD = libklipper_common.la
klipper_la_SOURCES = dummy.cpp
klipper_la_LDFLAGS = $(all_libraries) -module -avoid-version

klipper_panelapplet_la_LIBADD = libklipper_common.la
klipper_panelapplet_la_SOURCES = applet.cpp applet.skel
klipper_panelapplet_la_LDFLAGS = $(all_libraries) -module -avoid-version

xdg_apps_DATA = klipper.desktop

autostartdir = $(datadir)/autostart
autostart_DATA = klipper.desktop

appldir = $(kde_datadir)/kicker/applets
appl_DATA = klipper.desktop

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_confdir)
	$(INSTALL_DATA) $(srcdir)/klipperrc.desktop $(DESTDIR)$(kde_confdir)/klipperrc

KDE_ICON = klipper

messages:
	$(XGETTEXT) *.cpp -o $(podir)/klipper.pot

dummy.cpp:
	echo > dummy.cpp

# update script:
updatedir = $(kde_datadir)/kconf_update
update_DATA = klipperrc.upd klippershortcuts.upd
update_SCRIPTS = klipper-1-2.pl klipper-kde31.sh

