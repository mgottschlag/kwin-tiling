
AM_CPPFLAGS = -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST -DQT_NO_COMPAT
INCLUDES = $(all_includes) -I..

SUBDIRS = default

lib_LTLIBRARIES = libkcm_locale.la
libkcm_locale_la_SOURCES = ktagcombobox.cpp klocaleadv.cpp main.cpp toplevel.cpp locale.cpp klangcombo.cpp klocalesample.cpp localenum.cpp localemon.cpp localetime.cpp 

METASOURCES = ktagcombobox.moc toplevel.moc locale.moc klangcombo.moc localenum.moc localemon.moc localetime.moc
noinst_HEADERS = ktagcombobox.h klocaleadv.h toplevel.h locale.h klangcombo.h klocalesample.h localenum.h localemon.h localetime.h

libkcm_locale_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_locale_la_LIBADD = $(LIB_KDEUI)

KDE_ICON = locale

messages:
	$(XGETTEXT) $(libkcm_locale_la_SOURCES) -o $(podir)/kcmlocale.pot

