kde_module_LTLIBRARIES = libkfontviewpart.la

libkfontviewpart_la_SOURCES = FontViewPart.cpp FontViewPartFactory.cpp FontPreview.cpp
libkfontviewpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkfontviewpart_la_LIBADD = $(LIB_KPARTS) ../lib/libkfontinst.la

noinst_HEADERS  = FontViewPart.h FontViewPartFactory.h FontPreview.h FontViewerApp.h

kde_services_DATA = kfontviewpart.desktop

INCLUDES = -I$(srcdir)/../lib -I$(srcdir)/../../fonts $(all_includes) $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS)
METASOURCES = AUTO

kfontview_LDADD = $(LIB_KPARTS)
kfontview_LDFLAGS = $(all_libraries) $(KDE_RPATH)

bin_PROGRAMS = kfontview
xdg_apps_DATA = kfontview.desktop

appdata_DATA = kfontviewpart.rc
appdatadir = $(kde_datadir)/kfontview

kfontview_SOURCES = FontViewerApp.cpp

KDE_ICON = kfontview textwaterfall

