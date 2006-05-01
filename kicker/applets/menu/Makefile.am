INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker $(all_includes)

kde_module_LTLIBRARIES = menu_panelapplet.la

menu_panelapplet_la_SOURCES = menuapplet.cpp menuapplet.skel

noinst_HEADERS = menuapplet.h

menu_panelapplet_la_METASOURCES = AUTO

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = menuapplet.desktop

EXTRA_DIST = $(lnk_DATA)

menu_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
menu_panelapplet_la_LIBADD = $(LIB_KDEUI) $(top_builddir)/workspace/kicker/libkicker/libkickermain.la

