
INCLUDES = $(all_includes) -I..
LDFLAGS  = $(all_libraries) $(KDE_RPATH)
LDADD    = $(LIB_KDEUI) $(LIB_KDECORE) 

SUBDIRS = pics default

lib_LTLIBRARIES = libkcm_locale.la
libkcm_locale_la_SOURCES = main.cpp locale.cpp klangcombo.cpp klocalesample.cpp localenum.cpp localemon.cpp localetime.cpp

METASOURCES = locale.moc klangcombo.moc localenum.moc localemon.moc localetime.moc
noinst_HEADERS = main.h locale.h klangcombo.h klocalesample.h localenum.h localemon.h localetime.h
EXTRA_DIST = kcmlocalerc 

libkcm_locale_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_RPATH)

KDE_ICON = locale

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmlocale_SOURCES) && mv messages.po ../../po/kcmlocale.pot


