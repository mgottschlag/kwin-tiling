## Makefile.am of kdebase/kcontrol/ebrowsing/plugins/shorturi

INCLUDES  =  -I$(srcdir)/../../kurifilter $(all_includes)
LDFLAGS   = $(all_libraries)

lib_LTLIBRARIES = libkshorturifilter.la

libkshorturifilter_la_LDFLAGS = -version-info 1:0:0 -module -no-undefined
libkshorturifilter_la_LIBADD = ../../kurifilter/libkurifilter.la $(LIB_KDEUI)

libkshorturifilter_la_SOURCES = kshorturifilter.cpp

service_DATA = kshorturifilter.desktop
servicedir = $(kde_servicesdir)

#rc_DATA = kshorturifilterrc
#rcdir = $(kde_confdir)

METASOURCES = AUTO
