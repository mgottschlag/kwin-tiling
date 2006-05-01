INCLUDES = -I$(srcdir)/../../taskbar -I$(srcdir)/../../taskmanager -I$(srcdir)/../../libkicker $(all_includes)

kde_module_LTLIBRARIES = taskbar_panelapplet.la

taskbar_panelapplet_la_SOURCES = taskbarapplet.cpp

taskbar_panelapplet_la_METASOURCES = AUTO
taskbar_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
taskbar_panelapplet_la_LIBADD = $(LIB_KDEUI) ../../taskbar/libtaskbar.la ../../libkicker/libkickermain.la

noinst_HEADERS = taskbarapplet.h

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = taskbarapplet.desktop

EXTRA_DIST = $(lnk_DATA)


srcdoc:
	kdoc -a -p -H -d $$HOME/web/src/ktaskbarapplet ktaskbarapplet *.h -lqt -lkdecore -lkdeui -lkfile
