## Makefile.am of kdebase/kcontrol/ebrowsing/plugins/ikws

INCLUDES  = $(all_includes)
LDFLAGS   = $(all_libraries)

lib_LTLIBRARIES = libkuriikwsfilter.la

libkuriikwsfilter_la_LDFLAGS = -version-info 1:0:0 -module -no-undefined 
libkuriikwsfilter_la_LIBADD = -lksycoca $(LIB_KDEUI)

libkuriikwsfilter_la_SOURCES = kuriikwsfiltereng.cpp kuriikwsfilter.skel kuriikwsfilter.cpp ikwsopts.cpp

service_DATA = kuriikwsfilter.desktop
servicedir = $(kde_servicesdir)

rc_DATA = kuriikwsfilterrc
rcdir = $(kde_confdir)

METASOURCES = AUTO

