kde_module_LTLIBRARIES = libkcm_khotkeys.la

libkcm_khotkeys_la_SOURCES = kcmkhotkeys.cpp
libkcm_khotkeys_la_LIBADD  = ../shared/libshared.la -lksycoca
libkcm_khotkeys_la_LDFLAGS = $(all_libraries) -module -avoid-version

noinst_HEADERS = kcmkhotkeys.h

INCLUDES= $(all_includes) -I$(srcdir)/../shared

libkcm_khotkeys_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp -o $(podir)/khotkeys.pot

