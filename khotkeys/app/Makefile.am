bin_PROGRAMS =
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = khotkeys.la
kde_module_LTLIBRARIES = kded_khotkeys.la

khotkeys_la_SOURCES = app.cpp app.skel
khotkeys_la_LIBADD   = ../shared/libkhotkeys_shared.la $(LIB_KDECORE)
khotkeys_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined

kded_khotkeys_la_SOURCES = kded.cpp kded.skel
kded_khotkeys_la_LIBADD = ../shared/libkhotkeys_shared.la $(LIB_KDECORE)
kded_khotkeys_la_LDFLAGS = $(all_libraries) -module -avoid-version

noinst_HEADERS = app.h kded.h

INCLUDES = -I$(srcdir)/../shared $(all_includes)

METASOURCES = AUTO

servicesdir = $(kde_servicesdir)/kded
services_DATA = khotkeys.desktop
