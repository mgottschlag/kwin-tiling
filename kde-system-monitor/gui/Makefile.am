
kdemimedir = $(kde_mimedir)/application
kdemime_DATA = x-kde-system-monitor.desktop

rcdir = $(kde_datadir)/kde-system-monitor
rc_DATA = kde-system-monitorui.rc

xdg_apps_DATA = kde-system-monitor.desktop

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = kde-system-monitorapplet.desktop

# claim, which subdirectories you want to install
SUBDIRS = ksgrd SensorDisplayLib

# set the include path for X, qt and KDE
INCLUDES = -I$(srcdir)/ksgrd -I$(srcdir)/SensorDisplayLib -ISensorDisplayLib -I$(top_srcdir)/workspace/kicker/libkicker/ $(all_includes)
 
####### This part is very kde-system-monitor specific
# you can add here more. This one gets installed 
bin_PROGRAMS = kde-system-monitor kpm

# Which sources should be compiled for kde-system-monitor.
kde-system-monitor_SOURCES = \
	SensorBrowser.cc \
	SensorFrame.cc \
	WorkSheet.cc \
	WorkSheetSettings.cc \
	Workspace.cc \
	kde-system-monitor.cc kde-system-monitor.skel

kde-system-monitor_LDADD = \
	ksgrd/libksgrd.la \
	SensorDisplayLib/libsensordisplays.la \
	$(LIB_KDEUI) $(LIB_KIO)
kde-system-monitor_LDFLAGS = $(all_libraries) $(KDE_RPATH)

kpm_SOURCES = kpm.c

appdatadir = $(kde_datadir)/kde-system-monitor
appdata_DATA = ProcessTable.sgrd SystemLoad.sgrd kde-system-monitor-applet.xml

# This stuff is now for the kicker applet
kde_module_LTLIBRARIES = sysguard_panelapplet.la

sysguard_panelapplet_la_SOURCES = \
	kde-system-monitor-applet.cc \
	kde-system-monitor-applet-settings.cc

sysguard_panelapplet_la_LIBADD = \
	ksgrd/libksgrd.la \
	SensorDisplayLib/libsensordisplays.la \
	$(top_builddir)/workspace/kicker/libkicker/libkickermain.la $(LIB_KDEUI) $(LIB_KIO)
sysguard_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

EXTRA_DIST = $(lnk_DATA)

# just to make sure, automake makes them 
METASOURCES = AUTO

