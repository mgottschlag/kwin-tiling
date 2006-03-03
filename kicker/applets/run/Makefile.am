INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker $(all_includes) -I$(top_srcdir)/workspace/lib $(all_includes)

kde_module_LTLIBRARIES = run_panelapplet.la

run_panelapplet_la_SOURCES = runapplet.cpp

METASOURCES = runapplet.moc
noinst_HEADERS = runapplet.h

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = runapplet.desktop

EXTRA_DIST = $(lnk_DATA)

run_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
run_panelapplet_la_LIBADD = $(top_builddir)/workspace/lib/libkworkspace.la $(LIB_KSYCOCA) $(LIB_KDEUI) $(top_builddir)/workspace/kicker/libkicker/libkickermain.la

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/krunapplet.pot
