kde_module_LTLIBRARIES = khotkeys_arts.la

khotkeys_arts_la_SOURCES = soundrecorder_arts.cpp voicerecorder_arts.cpp
khotkeys_arts_la_LIBADD  = ../shared/libkhotkeys_shared.la $(LIB_ARTS)
khotkeys_arts_la_LDFLAGS = $(all_libraries) $(X_LDFLAGS) -module -avoid-version -no-undefined

INCLUDES = $(all_includes) -I$(srcdir)/../shared/ -I$(srcdir)/../kcontrol -I../kcontrol

noinst_HEADERS = soundrecorder_arts.h

METASOURCES = AUTO
