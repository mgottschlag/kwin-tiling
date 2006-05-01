SUBDIRS = pics

INCLUDES =  -I$(top_srcdir)/ksplashml/themeengine $(all_includes)

ksplashredmond_la_SOURCES = previewredmond.cpp themeredmond.cpp
ksplashredmond_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module $(KDE_PLUGIN)
ksplashredmond_la_LIBADD  = $(LIB_KDEUI) $(LIB_KIO) ../libksplashthemes.la
METASOURCES = AUTO

kde_module_LTLIBRARIES = ksplashredmond.la

noinst_HEADERS 	= themeredmond.h previewredmond.h

servicesdir = $(kde_servicesdir)
services_DATA = ksplashredmond.desktop

themedir = $(kde_datadir)/ksplash/Themes/Redmond
theme_DATA = Theme.rc
