lib_LTLIBRARIES = libkcm_khotkeys.la

libkcm_khotkeys_la_SOURCES = kcmkhotkeys.cpp
libkcm_khotkeys_la_LIBADD  = ../shared/libshared.la -lksycoca -lkdeui \
	-lkdecore -lqt -lXext -lX11 $(LIBSOCKET)
libkcm_khotkeys_la_LDFLAGS = $(all_libraries) -module -avoid-version

noinst_HEADERS = kcmkhotkeys.h

INCLUDES= $(all_includes) -I../shared

libkcm_khotkeys_la_METASOURCES = AUTO

DISTCLEANFILES = $(libkcm_khotkeys_la_METASOURCES)
