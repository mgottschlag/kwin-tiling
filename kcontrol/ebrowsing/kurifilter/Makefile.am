# $Id$

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkurifilter.la
libkurifilter_la_SOURCES = kurifilter.cpp kurifilterplugin.cpp
libkurifilter_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module -no-undefined
libkurifilter_la_LIBADD = -lkio

METASOURCES = AUTO

check_PROGRAMS = test
test_SOURCES = test.cpp
test_LDFLAGS  = $(all_libraries) -module -avoid-version
test_LDADD = libkurifilter.la -lkded

