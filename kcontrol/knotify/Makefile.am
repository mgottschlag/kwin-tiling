lib_LTLIBRARIES = libkcm_knotify.la

libkcm_knotify_la_SOURCES = main.cpp knotify.cpp eventview.cpp

libkcm_knotify_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libkcm_knotify_la_LIBADD = -lkdeui -lmediatool
INCLUDES= $(all_includes)
 
libkcm_knotify_la_METASOURCES = AUTO

noinst_HEADERS = knotify.h eventview.h

KDE_ICON = knotify

DISTCLEANFILES = $(lib_knotify_METASOURCES)

messages:
	$(XGETTEXT) $(libkcm_knotify_la_SOURCES) -o $(podir)/knotify.pot
