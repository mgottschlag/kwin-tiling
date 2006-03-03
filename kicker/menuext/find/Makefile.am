INCLUDES = $(all_includes) -I$(top_srcdir)/workspace/kicker/libkicker -I$(top_srcdir)/workspace/lib/

kde_module_LTLIBRARIES = kickermenu_find.la

kickermenu_find_la_SOURCES = findmenu.cpp
kickermenu_find_la_LDFLAGS = $(all_libraries) -module -avoid-version
kickermenu_find_la_LIBADD = $(LIB_KDEUI) $(top_builddir)/workspace/kicker/libkicker/libkickermain.la $(top_builddir)/workspace/lib/libkworkspace.la

kickermenu_find_la_METASOURCES = AUTO

desktopmenu_DATA = find.desktop
desktopmenudir = $(kde_datadir)/kicker/menuext

findmenu_data_DATA = kfind.desktop websearch.desktop
findmenu_datadir = $(kde_datadir)/kicker/menuext/find

messages:
	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_find.pot
