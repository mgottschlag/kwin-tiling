
AM_CPPFLAGS= $(all_includes)
METASOURCES = AUTO

bin_PROGRAMS =
lib_LTLIBRARIES =
kde_module_LTLIBRARIES = kcm_access.la
kdeinit_LTLIBRARIES = kaccess.la

kaccess_la_SOURCES = kaccess.cpp main.cpp
kaccess_la_LDFLAGS = $(all_libraries) -module -avoid-version
kaccess_la_LIBADD = $(LIB_KDEUI)

kcm_access_la_SOURCES = kcmaccess.cpp 
kcm_access_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_access_la_LIBADD = $(LIB_KIO)

noinst_HEADERS = kcmaccess.h

xdg_apps_DATA = kcmaccess.desktop

services_DATA = kaccess.desktop
servicesdir = $(kde_servicesdir)

eventsrcdir = $(kde_datadir)/kaccess/
eventsrc_DATA = eventsrc

messages:
	$(XGETTEXT) $(kaccess_la_SOURCES) -o $(podir)/kaccess.pot
	$(XGETTEXT) $(kcm_access_la_SOURCES) -o $(podir)/kcmaccess.pot
