SUBDIRS = shared kcontrol

bin_PROGRAMS =
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = khotkeys.la

khotkeys_la_SOURCES = khotkeys.cpp main.cpp \
	khotkeys.skel
khotkeys_la_LIBADD   = shared/libshared.la $(LIB_KDECORE)
khotkeys_la_LDFLAGS = $(all_libraries) -module -avoid-version

noinst_HEADERS = khotkeys.h

INCLUDES = -I$(srcdir)/shared $(all_includes)

METASOURCES = AUTO

autostart_DATA = khotkeys.desktop
autostartdir = $(prefix)/share/autostart

EXTRA_DIST = $(autostart_DATA) COPYING AUTHORS
