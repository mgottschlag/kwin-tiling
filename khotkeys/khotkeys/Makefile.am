# $Id$

bin_PROGRAMS = khotkeys
lib_LTLIBRARIES = khotkeys.la

khotkeys_SOURCES = dummy.cpp 
khotkeys_LDADD   = khotkeys.la ../shared/libshared.la -lksycoca
khotkeys_LDFLAGS = $(all_libraries) $(KDE_RPATH)

khotkeys_la_SOURCES = khkglobalaccel.cpp khotkeys.cpp main.cpp \
	khotkeys.skel
khotkeys_la_LIBADD   = -lksycoca
khotkeys_la_LDFLAGS = $(all_libraries) -module -avoid-version

noinst_HEADERS = khotkeys.h khkglobalaccel.h

INCLUDES= $(all_includes) -I$(srcdir)/../shared

METASOURCES = AUTO

dummy.cpp:
	echo > dummy.cpp
