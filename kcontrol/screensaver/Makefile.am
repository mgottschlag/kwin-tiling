# $Id$

SUBDIRS= . pics

INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kcm_screensaver.la

kcm_screensaver_la_SOURCES = scrnsave.cpp
kcm_screensaver_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_screensaver_la_LIBADD = $(LIB_KSYCOCA) $(DPMSLIB)
kcm_screensaver_la_METASOURCES = scrnsave.moc

noinst_HEADERS = scrnsave.h

messages:
	$(XGETTEXT) -kni18n $(kcm_screensaver_la_SOURCES) -o $(podir)/kcmscreensaver.pot

lnf_DATA = screensaver.desktop
lnfdir = $(kde_appsdir)/Settings/Desktop

EXTRA_DIST = $(lnf_DATA)
