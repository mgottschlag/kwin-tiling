lib_LTLIBRARIES = libkcm_kicker.la

libkcm_kicker_la_SOURCES = main.cpp panel.cpp 

libkcm_kicker_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_kicker_la_LIBADD = $(LIB_KIMGIO) $(LIB_KFILE)
INCLUDES= $(all_includes) 

libkcm_kicker_la_METASOURCES = AUTO

noinst_HEADERS = main.h panel.h

KDE_ICON = go

messages:
	$(XGETTEXT) $(libkcm_kicker_la_SOURCES) -o $(podir)/kcmkicker.pot

data_DATA = panel.desktop
datadir = $(kde_appsdir)/Settings/LookNFeel

EXTRA_DIST = $(data_DATA)
