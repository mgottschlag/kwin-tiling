kde_module_LTLIBRARIES = fontthumbnail.la

fontthumbnail_la_SOURCES = FontThumbnail.cpp
fontthumbnail_la_LIBADD = $(LIBFREETYPE_LIBS) $(LIB_KDECORE) $(LIB_KIO) ../lib/libkfontinst.la
fontthumbnail_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIBFREETYPE_RPATH) -module $(KDE_PLUGIN)
METASOURCES = AUTO

noinst_HEADERS = FontThumbnail.h

kdelnkdir = $(kde_servicesdir)

kde_services_DATA = fontthumbnail.desktop

AM_CPPFLAGS= -I$(srcdir)/../lib -I$(srcdir)/../../fonts $(all_includes) $(LIBFREETYPE_CFLAGS)
