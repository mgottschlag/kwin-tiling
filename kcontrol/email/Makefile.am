INCLUDES= $(all_includes) -I/usr/include/coas

lib_LTLIBRARIES = libkcm_daemons.la

libkcm_daemons_la_SOURCES = daemons.cpp

libkcm_daemons_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_daemons_la_LIBADD = $(LIB_KDEUI) -lcoas -lcoas-services

libkcm_daemons_la_METASOURCES = AUTO

noinst_HEADERS = daemons.h

data_DATA = daemons.desktop
datadir = $(kde_appsdir)/Settings/COAS
EXTRA_DIST = $(data_DATA)