INCLUDES = -I$(srcdir)/../../libkicker -I$(top_srcdir)/libkonq $(all_includes)

kde_module_LTLIBRARIES = sidebar_panelextension.la

sidebar_panelextension_la_SOURCES = sidebarextension.cpp

sidebar_panelextension_la_METASOURCES = AUTO
sidebar_panelextension_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
sidebar_panelextension_la_LIBADD =  ../../libkicker/libkickermain.la $(LIB_KDEUI) $(LIB_KPARTS) ../../../../libkonq/libkonq.la


lnkdir = $(kde_datadir)/kicker/extensions
lnk_DATA = sidebarextension.desktop

