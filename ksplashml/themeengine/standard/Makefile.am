SUBDIRS = pics

INCLUDES =  -I$(top_srcdir)/ksplashml/themeengine $(all_includes)

noinst_LTLIBRARIES = libthemestandard.la
libthemestandard_la_SOURCES = themestandard.cpp wndicon.cpp wndstatus.cpp
libthemestandard_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libthemestandard_la_LIBADD = $(LIB_KDEUI) ../libksplashthemes.la

kde_module_LTLIBRARIES = ksplashstandard.la

ksplashstandard_la_SOURCES = themestandardlib.cpp
ksplashstandard_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(KDE_PLUGIN) -module
ksplashstandard_la_LIBADD = $(LIB_KDEUI) ../libksplashthemes.la libthemestandard.la
METASOURCES = AUTO

noinst_HEADERS = themestandard.h wndicon.h wndstatus.h

servicesdir = $(kde_servicesdir)
services_DATA = ksplashstandard.desktop

themedir = $(kde_datadir)/ksplash/Themes/Standard
theme_DATA = Theme.rc
