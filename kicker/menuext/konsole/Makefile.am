INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker -I$(top_srcdir)/workspace/lib/ $(all_includes)

kde_module_LTLIBRARIES = kickermenu_konsole.la

kickermenu_konsole_la_SOURCES = konsole_mnu.cpp konsolebookmarkhandler.cpp konsolebookmarkmenu.cpp
kickermenu_konsole_la_LDFLAGS = $(all_libraries) -module -avoid-version
kickermenu_konsole_la_LIBADD = $(LIB_KIO) $(top_builddir)/workspace/lib/libkworkspace.la

kickermenu_konsole_la_METASOURCES = AUTO

desktopmenu_DATA = konsolemenu.desktop
desktopmenudir = $(kde_datadir)/kicker/menuext

messages:
	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_konsole.pot
