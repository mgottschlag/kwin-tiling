lib_LTLIBRARIES = libkcm_bell.la

libkcm_bell_la_SOURCES = bell.cpp

libkcm_bell_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_bell_la_LIBADD = -lkdeui

INCLUDES= $(all_includes)

libkcm_bell_la_METASOURCES = AUTO

KDE_ICON = bell

DISTCLEANFILES = $(lib_bell_METASOURCES)

messages:
	$(XGETTEXT) $(libkcm_bell_la_SOURCES) -o $(podir)/kcmbell.pot
