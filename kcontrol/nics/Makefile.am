kde_module_LTLIBRARIES = libkcm_nic.la

libkcm_nic_la_SOURCES = nic.cpp

libkcm_nic_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_nic_la_LIBADD = -lkdeui -lkfile $(LIBSOCKET)

INCLUDES= $(all_includes)

SUBDIRS = 

libkcm_nic_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) $(libkcm_nic_la_SOURCES) -o $(podir)/kcmnic.pot

data_DATA = nic.desktop
datadir = $(kde_appsdir)/Settings/Information


