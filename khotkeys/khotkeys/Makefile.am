bin_PROGRAMS = khotkeys
lib_LTLIBRARIES = libkhotkeys.la

khotkeys_SOURCES = main.cpp 
khotkeys_LDADD   = libkhotkeys.la ../shared/libshared.la -lksycoca
khotkeys_LDFLAGS = $(all_libraries) $(KDE_RPATH)

libkhotkeys_la_SOURCES = khkglobalaccel.cpp khotkeys.cpp main.cpp \
	khotkeys.skel
libkhotkeys_la_LIBADD   = -lksycoca
libkhotkeys_la_LDFLAGS = $(all_libraries) -module -avoid-version

noinst_HEADERS = khotkeys.h khkglobalaccel.h

INCLUDES= $(all_includes) -I$(srcdir)/../shared

METASOURCES = AUTO
