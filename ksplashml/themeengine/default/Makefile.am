
INCLUDES =  -I$(srcdir)/.. -I$(srcdir)/../.. $(all_includes)

noinst_LTLIBRARIES = libthemedefault.la
libthemedefault_la_SOURCES = themelegacy.cpp
libthemedefault_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libthemedefault_la_LIBADD = $(LIB_KDEUI) ../libksplashthemes.la

kde_module_LTLIBRARIES = ksplashdefault.la

ksplashdefault_la_SOURCES = main.cpp
ksplashdefault_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(KDE_PLUGIN) -module
ksplashdefault_la_LIBADD = $(LIB_KDEUI) ../libksplashthemes.la libthemedefault.la
METASOURCES = AUTO

servicesdir = $(kde_servicesdir)
services_DATA = ksplashdefault.desktop
