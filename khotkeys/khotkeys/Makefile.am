# $Id$

bin_PROGRAMS =
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = khotkeys.la

khotkeys_la_SOURCES = khotkeys.cpp main.cpp khotkeys.skel
khotkeys_la_LIBADD   = ../shared/libshared.la $(LIB_KSYCOCA)
khotkeys_la_LDFLAGS = $(all_libraries) -module -avoid-version

autostart_DATA = khotkeys.desktop
autostartdir = $(prefix)/share/autostart

noinst_HEADERS = khotkeys.h

INCLUDES= -I$(srcdir)/../shared $(all_includes)

METASOURCES = AUTO
