kde_module_LTLIBRARIES = kcm_khotkeys.la

kcm_khotkeys_la_SOURCES = kcmkhotkeys.cpp
kcm_khotkeys_la_LIBADD  = ../shared/libshared.la $(LIB_KIO)
kcm_khotkeys_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

noinst_HEADERS = kcmkhotkeys.h

INCLUDES= -I$(srcdir)/../shared $(all_includes)

kcm_khotkeys_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp -o $(podir)/khotkeys.pot

