INCLUDES= $(all_includes)

bin_PROGRAMS = klipper
lib_LTLIBRARIES = klipper.la
kde_module_LTLIBRARIES = klipper_panelapplet.la

CLEANFILES = dummy.cpp

klipper_la_LIBADD = $(LIB_KDEUI) $(LIB_KSYCOCA) 
klipper_la_SOURCES = main.cpp toplevel.cpp urlgrabber.cpp configdialog.cpp toplevel.skel \
	aboutwidget.cpp

klipper_la_METASOURCES = AUTO
klipper_la_LDFLAGS = $(all_libraries) -module -avoid-version

klipper_LDADD = klipper.la $(LIB_KSYCOCA)
klipper_SOURCES = dummy.cpp
klipper_LDFLAGS = $(all_libraries) $(KDE_RPATH)

klipper_panelapplet_la_LIBADD = klipper.la
klipper_panelapplet_la_SOURCES = applet.cpp
klipper_panelapplet_la_LDFLAGS = $(all_libraries) -module -avoid-version


appsdir = $(kde_appsdir)/Utilities
apps_DATA = klipper.desktop

autostartdir = $(prefix)/share/autostart
autostart_DATA = klipper.desktop

appldir = $(kde_datadir)/kicker/applets
appl_DATA = klipper.desktop

install-data-local:
	$(mkinstalldirs) $(kde_confdir)
	$(INSTALL_DATA) $(srcdir)/klipperrc.desktop $(kde_confdir)/klipperrc

KDE_ICON = klipper
dockicondir = $(kde_datadir)/klipper/pics
dockicon_DATA = klipper_dock.png

messages:
	$(XGETTEXT) *.cpp -o $(podir)/klipper.pot

dummy.cpp:
	echo > dummy.cpp

# update script:
updatedir = $(kde_datadir)/kconf_update
update_DATA = klipperrc.upd klipper-1-2.pl klipper-kde31.sh

