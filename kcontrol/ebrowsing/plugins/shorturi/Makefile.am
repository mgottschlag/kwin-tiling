## Makefile.am of kdebase/kcontrol/ebrowsing/plugins/shorturi

AM_CPPFLAGS = -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST

INCLUDES  =  $(all_includes)
LDFLAGS   = $(all_libraries)

kde_module_LTLIBRARIES = libkshorturifilter.la

libkshorturifilter_la_LDFLAGS = -version-info 1:0:0 -module -no-undefined
libkshorturifilter_la_LIBADD = $(LIB_KSYCOCA) $(LIB_KDEUI)

libkshorturifilter_la_SOURCES = kshorturifilter.cpp kshorturifilter.skel

service_DATA = kshorturifilter.desktop
servicedir = $(kde_servicesdir)

rc_DATA = kshorturifilterrc
rcdir = $(kde_confdir)

METASOURCES = AUTO
