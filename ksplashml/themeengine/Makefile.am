SUBDIRS = . default standard redmond simple

INCLUDES = -I$(srcdir)/.. $(all_includes)

lib_LTLIBRARIES = libksplashthemes.la

libksplashthemes_la_SOURCES = themeengine.cpp objkstheme.cpp
libksplashthemes_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libksplashthemes_la_LIBADD = $(LIB_KDEUI) $(LIB_XCURSOR)
METASOURCES = AUTO

# install this header for theme developers.
pkgincludedir = $(includedir)/ksplash
pkginclude_HEADERS=themeengine.h objkstheme.h

servicetypesdir = $(kde_servicetypesdir)
servicetypes_DATA = ksplashplugins.desktop

