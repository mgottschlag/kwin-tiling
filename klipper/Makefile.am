INCLUDES= $(all_includes)

bin_PROGRAMS = klipper
lib_LTLIBRARIES = klipper.la
kde_module_LTLIBRARIES = libklipperapplet.la

CLEANFILES = dummy.cpp

klipper_la_LIBADD = $(LIB_KDEUI) $(LIB_KSYCOCA) 
klipper_la_SOURCES = main.cpp toplevel.cpp urlgrabber.cpp configdialog.cpp toplevel.skel

klipper_la_METASOURCES = AUTO
klipper_la_LDFLAGS = $(all_libraries) -module -avoid-version

klipper_LDADD = klipper.la $(LIB_KSYCOCA)
klipper_SOURCES = dummy.cpp
klipper_LDFLAGS = $(all_libraries) $(KDE_RPATH)

libklipperapplet_la_LIBADD = klipper.la
libklipperapplet_la_SOURCES = applet.cpp
libklipperapplet_la_LDFLAGS = $(all_libraries) -module -avoid-version


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
update_DATA = klipperrc.upd klipper-1-2.pl

