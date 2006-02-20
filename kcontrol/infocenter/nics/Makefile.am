kde_module_LTLIBRARIES = kcm_nic.la

kcm_nic_la_SOURCES = nic.cpp

kcm_nic_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_nic_la_LIBADD = -lkdeui $(LIB_KIO) $(LIBSOCKET)

AM_CPPFLAGS= $(all_includes)

METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kcmnic.pot

xdg_apps_DATA = nic.desktop
