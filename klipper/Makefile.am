INCLUDES= $(all_includes)

bin_PROGRAMS = klipper
lib_LTLIBRARIES = klipper.la

CLEANFILES = dummy.cpp

klipper_la_LIBADD = $(LIB_KDEUI)
klipper_la_SOURCES = main.cpp toplevel.cpp urlgrabber.cpp configdialog.cpp \
	keditablelistview.cpp qclipboard_x11.cpp

klipper_la_METASOURCES = AUTO
klipper_la_LDFLAGS = $(all_libraries) -module -avoid-version

klipper_LDADD = klipper.la
klipper_SOURCES = dummy.cpp
klipper_LDFLAGS = $(all_libraries) $(KDE_RPATH)


appsdir = $(kde_appsdir)/Utilities
apps_DATA = klipper.desktop
data_DATA = klipperrc

datadir = $(kde_confdir)
EXTRA_DIST = $(data_DATA)

KDE_ICON = klipper

messages:
	$(XGETTEXT) $(klipper_la_SOURCES) -o $(podir)/klipper.pot

dummy.cpp:
	echo > dummy.cpp

