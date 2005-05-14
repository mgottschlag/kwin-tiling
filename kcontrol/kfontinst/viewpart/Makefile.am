noinst_LTLIBRARIES = libkfontinstprint.la
libkfontinstprint_la_SOURCES = KfiPrint.cpp
libkfontinstprint_la_LDFLAGS = $(all_libraries)
libkfontinstprint_la_LIBADD = $(LIB_KDEPRINT) ../lib/libkfontinst.la

kde_module_LTLIBRARIES = libkfontviewpart.la

libkfontviewpart_la_SOURCES = FontViewPart.cpp FontViewPartFactory.cpp FontPreview.cpp
libkfontviewpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkfontviewpart_la_LIBADD = $(LIB_KPARTS) libkfontinstprint.la ../lib/libkfontinst.la

noinst_HEADERS  = FontViewPart.h FontViewPartFactory.h FontPreview.h FontViewerApp.h KfiPrint.h

kde_services_DATA = kfontviewpart.desktop

AM_CPPFLAGS = -I$(srcdir)/../lib -I$(srcdir)/../../fonts $(all_includes) $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS)
METASOURCES = AUTO

kfontview_LDADD = $(LIB_KPARTS)
kfontview_LDFLAGS = $(all_libraries) $(KDE_RPATH)

bin_PROGRAMS = kfontview
xdg_apps_DATA = kfontview.desktop

appdata_DATA = kfontviewpart.rc kfontviewui.rc
appdatadir = $(kde_datadir)/kfontview

kfontview_SOURCES = FontViewerApp.cpp


