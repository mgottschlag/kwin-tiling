
INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker $(all_includes)

kde_module_LTLIBRARIES = systemtray_panelapplet.la

systemtray_panelapplet_la_SOURCES = systemtrayapplet.cpp systemtrayapplet.skel

systemtray_panelapplet_la_METASOURCES = AUTO
noinst_HEADERS = systemtrayapplet.h

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = systemtrayapplet.desktop

EXTRA_DIST = $(lnk_DATA)

systemtray_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
systemtray_panelapplet_la_LIBADD = ../../libkicker/libkickermain.la $(LIB_KDEUI) $(LIB_KIO)

KDE_ICON=AUTO


srcdoc:
	kdoc -a -p -H -d $$HOME/web/src/ksystemtrayapplet ksystemtrayapplet *.h -lqt -lkdecore -lkdeui -lkfile
