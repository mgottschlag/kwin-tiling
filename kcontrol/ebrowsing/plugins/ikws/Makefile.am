## Makefile.am of kdebase/kcontrol/ebrowsing/plugins/ikws

SUBDIRS = searchproviders

INCLUDES  = $(all_includes)
LDFLAGS   = $(all_libraries)

lib_LTLIBRARIES = libkuriikwsfilter.la libkurisearchfilter.la

libkuriikwsfilter_la_LDFLAGS = -version-info 1:0:0 -module -no-undefined 
libkuriikwsfilter_la_LIBADD = $(LIB_KSYCOCA) $(LIB_KDEUI)

libkurisearchfilter_la_LDFLAGS = -version-info 1:0:0 -module -no-undefined
libkurisearchfilter_la_LIBADD = $(LIB_KSYCOCA) $(LIB_KDEUI)

libkuriikwsfilter_la_SOURCES = kuriikwsfiltereng.cpp kuriikwsfilter.skel kuriikwsfilter.cpp searchprovider.cpp
libkurisearchfilter_la_SOURCES = kurisearchfilter.cpp kurisearchfilter.skel kuriikwsfiltereng.cpp ikwsopts.cpp searchproviderdlg.cpp searchprovider.cpp

service_DATA = kuriikwsfilter.desktop kurisearchfilter.desktop
servicedir = $(kde_servicesdir)

servicetypes_DATA = searchprovider.desktop
servicetypesdir = $(kde_servicetypesdir)

rc_DATA = kuriikwsfilterrc
rcdir = $(kde_confdir)

METASOURCES = AUTO

