INCLUDES = $(all_includes)

kde_module_LTLIBRARIES      = libkcm_kwintheme.la

libkcm_kwintheme_la_SOURCES = kwintheme.cpp
noinst_HEADERS              = kwintheme.h

libkcm_kwintheme_la_LDFLAGS = \
  -module -avoid-version $(all_libraries) -no-undefined

libkcm_kwintheme_la_LIBADD  = $(LIB_KDEUI)

METASOURCES = AUTO


messages:
	$(XGETTEXT) -kni18n $(libkcm_kwintheme_la_SOURCES) -o $(podir)/kcmkwintheme.pot

lnf_DATA = kwintheme.desktop
lnfdir = $(kde_appsdir)/Settings/LookNFeel

EXTRA_DIST = $(lnf_DATA)
