kde_module_LTLIBRARIES = kcm_nic.la

kcm_nic_la_SOURCES = nic.cpp

kcm_nic_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_nic_la_LIBADD = -lkdeui $(LIB_KIO) $(LIBSOCKET)

INCLUDES= $(all_includes)

kcm_nic_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) $(kcm_nic_la_SOURCES) -o $(podir)/kcmnic.pot

data_DATA = nic.desktop
datadir = $(kde_appsdir)/Settings/Information


