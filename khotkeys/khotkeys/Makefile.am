# $Id$

bin_PROGRAMS = khotkeys
lib_LTLIBRARIES = khotkeys.la

khotkeys_la_SOURCES = khotkeys.cpp main.cpp khotkeys.skel
khotkeys_la_LIBADD   = ../shared/libshared.la $(LIB_KSYCOCA)
khotkeys_la_LDFLAGS = $(all_libraries) -module -avoid-version

khotkeys_SOURCES = dummy.cpp
khotkeys_LDADD   = khotkeys.la
khotkeys_LDFLAGS = $(all_libraries) $(KDE_RPATH)

autostart_DATA = khotkeys.desktop
autostartdir = $(prefix)/share/autostart

noinst_HEADERS = khotkeys.h khkglobalaccel.h

INCLUDES= -I$(srcdir)/../shared $(all_includes)

METASOURCES = AUTO

dummy.cpp:
	echo > dummy.cpp
