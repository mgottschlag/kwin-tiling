INCLUDES = -I$(top_srcdir)/libkonq -I$(top_srcdir)/workspace/kicker/libkicker -I$(top_srcdir)/workspace/kicker/core $(all_includes)

kde_module_LTLIBRARIES = media_panelapplet.la
media_panelapplet_la_SOURCES = preferencesdialog.cpp mediumbutton.cpp mediaapplet.cpp

METASOURCES = AUTO

noinst_HEADERS = mediaapplet.h mediumbutton.h preferencesdialog.h 

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = mediaapplet.desktop

EXTRA_DIST = $(lnk_DATA)

media_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
media_panelapplet_la_LIBADD = ../../../../libkonq/libkonq.la ../../libkicker/libkickermain.la $(LIB_KDEUI) $(LIB_KIO) $(LIB_KUTILS)


