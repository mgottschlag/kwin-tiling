#AM_CPPFLAGS = -DQT_NO_CAST_ASCII -DQT_NO_COMPAT

INCLUDES = $(all_includes)

SUBDIRS = default pics

kde_module_LTLIBRARIES = kcm_locale.la
kcm_locale_la_SOURCES = main.cpp toplevel.cpp kcmlocale.cpp klanguagebutton.cpp klocalesample.cpp localenum.cpp localemon.cpp localetime.cpp localeother.cpp
METASOURCES = AUTO
noinst_HEADERS = toplevel.h kcmlocale.h klanguagebutton.h klocalesample.h localenum.h localemon.h localetime.h localeother.h 

kcm_locale_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_locale_la_LIBADD = $(LIB_KDEUI)

messages:
	$(XGETTEXT) $(kcm_locale_la_SOURCES) -o $(podir)/kcmlocale.pot

xdg_apps_DATA = language.desktop
