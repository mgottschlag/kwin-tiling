# Makefile.am for kdebase/ksplashml/kcmksplash
SUBDIRS=pics

INCLUDES = $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kcm_ksplashthemes.la

kcm_ksplashthemes_la_SOURCES = installer.cpp main.cpp
noinst_HEADERS = installer.h

kcm_ksplashthemes_la_LIBADD = $(LIB_KIO)  $(LIB_KDE3SUPPORT)
kcm_ksplashthemes_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module

xdg_apps_DATA = ksplashthememgr.desktop

