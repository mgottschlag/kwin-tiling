lib_LTLIBRARIES = libkcm_iconthemes.la

libkcm_iconthemes_la_SOURCES = iconthemes.cpp 

libkcm_iconthemes_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_iconthemes_la_LIBADD = -lkdeui -lkfile
INCLUDES= $(all_includes) 

libkcm_iconthemes_la_METASOURCES = AUTO

noinst_HEADERS = iconthemes.h

KDE_ICON = iconthemes

messages:
	$(XGETTEXT) $(libkcm_iconthemes_la_SOURCES) -o $(podir)/kcmiconthemes.pot

data_DATA = iconthemes.desktop
datadir = $(kde_appsdir)/Settings/LookNFeel/Themes
