
INCLUDES = $(all_includes) -I..
LDFLAGS  = $(all_libraries) $(KDE_RPATH)
LDADD    = $(LIB_KDEUI) $(LIB_KDECORE) 

SUBDIRS = pics default

bin_PROGRAMS = kcmlocale

kcmlocale_SOURCES = main.cpp locale.cpp klangcombo.cpp klocalesample.cpp localeadv.cpp
METASOURCES = locale.moc klangcombo.moc localeadv.moc
noinst_HEADERS = locale.h klangcombo.h klocalesample.h localeadv.h
EXTRA_DIST = kcmlocalerc 

KDE_ICON = locale

kde_conf_DATA = kcmlocalerc

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmlocale_SOURCES) && mv messages.po ../../po/kcmlocale.pot


