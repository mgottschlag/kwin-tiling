INCLUDES = -I$(srcdir)/../../taskmanager \
	-I$(srcdir)/../../libkicker -I../../libkicker $(all_includes)

kde_module_LTLIBRARIES = minipager_panelapplet.la

minipager_panelapplet_la_SOURCES = pagerapplet.cpp pagerbutton.cpp pagersettings.kcfgc

minipager_panelapplet_la_METASOURCES = AUTO
noinst_HEADERS = pagerapplet.h pagerbutton.h

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = minipagerapplet.desktop

EXTRA_DIST = $(lnk_DATA) 

minipager_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
minipager_panelapplet_la_LIBADD = $(LIB_KDEUI) \
	../../taskmanager/libtaskmanager.la \
	../../libkicker/libkickermain.la


srcdoc:
	kdoc -a -p -H -d $$HOME/web/src/kminipagerapplet kminipagerapplet *.h -lqt -lkdecore -lkdeui -lkfile
