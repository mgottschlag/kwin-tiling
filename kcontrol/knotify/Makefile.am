lib_LTLIBRARIES = libkcm_knotify.la

libkcm_knotify_la_SOURCES = main.cpp knotify.cpp eventview.cpp eventconfig.cpp

libkcm_knotify_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libkcm_knotify_la_LIBADD = -lkdeui
INCLUDES= $(all_includes)
 
libkcm_knotify_la_METASOURCES = AUTO

noinst_HEADERS = knotify.h eventview.h eventconfig.h

messages:
	$(XGETTEXT) $(libkcm_knotify_la_SOURCES) -o $(podir)/kcmnotify.pot

data_DATA = kcmnotify.desktop
datadir = $(kde_appsdir)/Settings/LookNFeel
