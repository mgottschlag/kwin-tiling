INCLUDES = $(all_includes) -I$(top_srcdir)/workspace/kicker/libkicker

kde_module_LTLIBRARIES = kickermenu_kdeprint.la

kickermenu_kdeprint_la_SOURCES = print_mnu.cpp
kickermenu_kdeprint_la_LDFLAGS = $(all_libraries) -module -avoid-version
kickermenu_kdeprint_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO) -lkdeprint $(top_builddir)/workspace/kicker/libkicker/libkickermain.la

kickermenu_kdeprint_la_METASOURCES = AUTO

desktopmenu_DATA = printmenu.desktop
desktopmenudir = $(kde_datadir)/kicker/menuext

