kde_module_LTLIBRARIES = kfile_font.la

kfile_font_la_SOURCES = KFileFont.cpp
kfile_font_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIBFREETYPE_RPATH) -module $(KDE_PLUGIN)
kfile_font_la_LIBADD = $(LIB_KIO) $(LIBFREETYPE_LIBS) ../lib/libkfontinst.la

kdelnkdir = $(kde_servicesdir)
kde_services_DATA = kfile_font.desktop

AM_CPPFLAGS = -I$(srcdir)/../lib -I$(srcdir)/../../fonts $(all_includes) $(LIBFREETYPE_CFLAGS)

METASOURCES = AUTO
noinst_HEADERS = KFileFont.h
