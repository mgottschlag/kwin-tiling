
AM_CPPFLAGS = -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST
INCLUDES = $(all_includes) -I..
LDADD    = $(LIB_KDEUI) $(LIB_KDECORE) 

SUBDIRS = pics default

lib_LTLIBRARIES = libkcm_locale.la
libkcm_locale_la_SOURCES = main.cpp toplevel.cpp locale.cpp klangcombo.cpp klocalesample.cpp localenum.cpp localemon.cpp localetime.cpp 

METASOURCES = toplevel.moc locale.moc klangcombo.moc localenum.moc localemon.moc localetime.moc
noinst_HEADERS = toplevel.h locale.h klangcombo.h klocalesample.h localenum.h localemon.h localetime.h

libkcm_locale_la_LDFLAGS = -module -avoid-version $(all_libraries)

KDE_ICON = locale


check_PROGRAMS = test
test_SOURCES = $(libkcm_locale_la_SOURCES) test.cpp
test.cpp:
	echo "int main() {};" > test.cpp
test_LDFLAGS  = $(all_libraries) -module -avoid-version
test_LDADD = $(LDADD)


messages:
	$(XGETTEXT) -C -ktranslate -ki18n -kI18N_NOOP -x $(includedir)/kde.pot $(libkcm_locale_la_SOURCES) && mv messages.po $(podir)/kcmlocale.pot
