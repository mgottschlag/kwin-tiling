SUBDIRS = themeengine pics kcmksplash

INCLUDES = -I$(srcdir)/themeengine -I$(srcdir)/themeengine/default $(all_includes)

METASOURCES=AUTO

bin_PROGRAMS = ksplash
ksplash_SOURCES = wndmain.cpp ksplashiface.skel main.cpp
ksplash_LDFLAGS = $(all_libraries) $(KDE_RPATH)
ksplash_LDADD = $(LIB_KDEUI) themeengine/default/libthemedefault.la themeengine/libksplashthemes.la $(LIB_KIO)

noinst_HEADERS = ksplashiface.h wndmain.h

servicesdir = $(kde_servicesdir)
services_DATA = ksplash.desktop

