kde_module_LTLIBRARIES = kfile_font.la

kfile_font_la_SOURCES = KFileFont.cpp
kfile_font_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIBFREETYPE_RPATH) -module $(KDE_PLUGIN)
kfile_font_la_LIBADD = $(LIB_KIO) $(LIBFREETYPE_LIBS) ../lib/libkfontinst.la

kdelnkdir = $(kde_servicesdir)
kde_services_DATA = kfile_font.desktop

INCLUDES = $(LIBFREETYPE_CFLAGS) -I$(srcdir)/../lib -I$(srcdir)/../kio -I$(srcdir)/../../fonts $(all_includes)

METASOURCES = AUTO
noinst_HEADERS = KFileFont.h

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kfile_font.pot
