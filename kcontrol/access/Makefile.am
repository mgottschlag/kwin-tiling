APPSDIR = $(kde_appsdir)/Utilities
INCLUDES= $(all_includes)
METASOURCES = AUTO

bin_PROGRAMS = kaccess

kaccess_SOURCES = main.cpp kaccess.cpp

kaccess_LDFLAGS = $(all_libraries) $(KDE_RPATH)

kaccess_LDADD = -lkdecore -lkdeui -lqt

DISTCLEANFILES = $(kaccess_METASOURCES)


lib_LTLIBRARIES = libkcm_access.la

libkcm_access_la_SOURCES = kcmaccess.cpp 

libkcm_access_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_access_la_LIBADD = $(LIB_KDEUI) $(LIB_KSYCOCA) $(LIB_KFILE)

noinst_HEADERS = kcmaccess.h

data_DATA = kcmaccess.desktop
datadir = $(kde_appsdir)/Settings/Personalization

#KDE_ICON = sample


messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kaccess_SOURCES) && mv messages.po ../po/kaccess.pot
	$(XGETTEXT) $(libkcm_access_la_SOURCES) -o $(podir)/kcmaccess.pot


