lib_LTLIBRARIES = libkfontviewpart.la

libkfontviewpart_la_SOURCES = FontViewPart.cpp FontViewPartFactory.cpp FontPreview.cpp
libkfontviewpart_la_LDFLAGS = -module $(KDE_PLUGIN)
libkfontviewpart_la_LIBADD = $(LIB_KPARTS) ../lib/libkfontinst.la

noinst_HEADERS  = FontViewPart.h FontViewPartFactory.h FontPreview.h

kde_services_DATA = kfontviewpart.desktop

INCLUDES = -I../lib -I../../fonts $(all_includes) $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS)
METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kfontviewpart.pot
