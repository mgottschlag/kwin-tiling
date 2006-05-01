INCLUDES = -I$(top_srcdir)/libkonq -I$(top_srcdir)/workspace/kicker/libkicker -I$(top_srcdir)/workspace/kicker/core $(all_includes)

kde_module_LTLIBRARIES = trash_panelapplet.la
trash_panelapplet_la_SOURCES = trashbutton.cpp trashapplet.cpp 

METASOURCES = AUTO

noinst_HEADERS = \
  trashapplet.h trashbutton.h

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = trashapplet.desktop

EXTRA_DIST = $(lnk_DATA)

trash_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
trash_panelapplet_la_LIBADD = ../../libkicker/libkickermain.la ../../../../libkonq/libkonq.la $(LIB_KDEUI) $(LIB_KIO) $(LIB_KUTILS)


