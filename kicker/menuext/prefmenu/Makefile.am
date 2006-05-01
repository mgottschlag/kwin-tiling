INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker -I$(top_builddir)/workspace/kicker/libkicker -I$(top_srcdir)/workspace/lib/ $(all_includes)

kde_module_LTLIBRARIES = kickermenu_prefmenu.la

kickermenu_prefmenu_la_SOURCES = prefmenu.cpp
kickermenu_prefmenu_la_LDFLAGS = $(all_libraries) -module -avoid-version
kickermenu_prefmenu_la_LIBADD = $(LIB_KDEUI) $(top_builddir)/workspace/kicker/libkicker/libkickermain.la $(top_builddir)/workspace/lib/libkworkspace.la

kickermenu_prefmenu_la_METASOURCES = AUTO

desktopmenu_DATA = prefmenu.desktop
desktopmenudir = $(kde_datadir)/kicker/menuext


prefmenu.lo: ../../libkicker/kickerSettings.h
