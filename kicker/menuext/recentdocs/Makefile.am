INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker -I$(top_srcdir)/workspace/lib/ $(all_includes)

kde_module_LTLIBRARIES = kickermenu_recentdocs.la

kickermenu_recentdocs_la_SOURCES = recentdocsmenu.cpp
kickermenu_recentdocs_la_LDFLAGS = $(all_libraries) -module -avoid-version
kickermenu_recentdocs_la_LIBADD = $(LIB_KDEUI) $(top_builddir)/workspace/kicker/libkicker/libkickermain.la $(top_builddir)/workspace/lib/libkworkspace.la

kickermenu_recentdocs_la_METASOURCES = AUTO

desktopmenu_DATA = recentdocs.desktop
desktopmenudir = $(kde_datadir)/kicker/menuext

