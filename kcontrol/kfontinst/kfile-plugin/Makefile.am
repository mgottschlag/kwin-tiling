kde_module_LTLIBRARIES = kfile_font.la

kfile_font_la_SOURCES = kfile_font.cpp
kfile_font_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_font_la_LIBADD = $(LIB_KSYCOCA)

kdelnkdir = $(kde_servicesdir)
kde_services_DATA = kfile_font.desktop

LDFLAGS = $(all_libraries) $(KDE_RPATH)
INCLUDES = $(FREETYPE_INC) $(all_includes) -I$(srcdir)/../kfontinst

METASOURCES = AUTO
noinst_HEADERS = kfile_font.h

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kfile_font.pot
