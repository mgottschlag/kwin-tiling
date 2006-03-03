INCLUDES = $(all_includes) -I$(top_srcdir)/workspace/kicker/libkicker -I$(top_builddir)/workspace/kicker/libkicker

kde_module_LTLIBRARIES = kickermenu_remotemenu.la

kickermenu_remotemenu_la_SOURCES = remotemenu.cpp remotemenu.skel
kickermenu_remotemenu_la_LDFLAGS = $(all_libraries) -module -avoid-version
kickermenu_remotemenu_la_LIBADD = $(LIB_KDEUI) $(top_builddir)/workspace/kicker/libkicker/libkickermain.la

kickermenu_remotemenu_la_METASOURCES = AUTO

desktopmenu_DATA = remotemenu.desktop
desktopmenudir = $(kde_datadir)/kicker/menuext

messages:
	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_remotemenu.pot

remotemenu.lo: ../../libkicker/kickerSettings.h
