bin_PROGRAMS = khotkeys
lib_LTLIBRARIES = khotkeys.la

khotkeys_SOURCES = main.cpp 
khotkeys_LDADD   = khotkeys.la -lksycoca
khotkeys_LDFLAGS = $(all_libraries) $(KDE_RPATH)

khotkeys_la_SOURCES = khkglobalaccel.cpp khotkeys.cpp main.cpp \
	khotkeys.skel
khotkeys_la_LIBADD   = ../shared/libshared.la -lksycoca
khotkeys_la_LDFLAGS = $(all_libraries) -module -avoid-version

noinst_HEADERS = khotkeys.h khkglobalaccel.h

INCLUDES= $(all_includes) -I$(srcdir)/../shared

METASOURCES = AUTO
