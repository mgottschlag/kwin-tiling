## Makefile.am of kdebase/kcontrol/ebrowsing/plugins/localdomain

AM_CPPFLAGS = -DQT_NO_CAST_ASCII

INCLUDES  =  $(all_includes)
LDFLAGS   = $(all_libraries)

kde_module_LTLIBRARIES = liblocaldomainurifilter.la

liblocaldomainurifilter_la_LDFLAGS = -module $(KDE_PLUGIN)
liblocaldomainurifilter_la_LIBADD = $(LIB_KSYCOCA) $(LIB_KDEUI)

liblocaldomainurifilter_la_SOURCES = localdomainurifilter.cpp localdomainurifilter.skel

service_DATA = localdomainurifilter.desktop
servicedir = $(kde_servicesdir)

METASOURCES = AUTO

bin_PROGRAMS = klocaldomainurifilterhelper

klocaldomainurifilterhelper_SOURCES = klocaldomainurifilterhelper.c
