kde_module_LTLIBRARIES = fontthumbnail.la

fontthumbnail_la_SOURCES = FontThumbnail.cpp
fontthumbnail_la_LIBADD = $(FREETYPE_LIB) $(LIB_KDECORE) $(LIB_KIO)
fontthumbnail_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
METASOURCES = AUTO

noinst_HEADERS = FontThumbnail.h

kdelnkdir = $(kde_servicesdir)

kde_services_DATA = fontthumbnail.desktop

LDFLAGS = $(all_libraries) $(KDE_RPATH)
INCLUDES= $(FREETYPE_INC) $(all_includes) -I../kfontinst
