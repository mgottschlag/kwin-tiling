lib_LTLIBRARIES = libkcm_drkonqi.la

libkcm_drkonqi_la_SOURCES = main.cpp general.cpp customize.cpp

libkcm_drkonqi_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_drkonqi_la_LIBADD = -lkdeui

INCLUDES= $(all_includes)

SUBDIRS = 

libkcm_drkonqi_la_METASOURCES = AUTO

KDE_ICON = drkonqi

messages:
	$(XGETTEXT) $(libkcm_drkonqi_la_SOURCES) -o $(podir)/kcmdrkonqi.pot

data_DATA = drkonqi.desktop
datadir = $(kde_appsdir)/Settings/Applications

