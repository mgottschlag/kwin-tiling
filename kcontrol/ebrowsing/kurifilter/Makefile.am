# $Id$

INCLUDES = $(all_includes)

noinst_LTLIBRARIES = libkurifilter.la
libkurifilter_la_SOURCES = kurifilter.cpp kurifilterplugin.cpp
libkurifilter_la_LDFLAGS = -no-undefined $(all_libraries)

METASOURCES = AUTO

check_PROGRAMS = test
test_SOURCES = test.cpp
test_LDFLAGS  = $(all_libraries) -module -avoid-version
test_LDADD = libkurifilter.la -lkded

