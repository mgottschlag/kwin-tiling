# $Id$

SUBDIRS= . pics

INCLUDES = $(all_includes) -I$(top_srcdir)/kcontrol/lib

kde_module_LTLIBRARIES = libkcm_screensaver.la

libkcm_screensaver_la_SOURCES = scrnsave.cpp
libkcm_screensaver_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_screensaver_la_LIBADD = $(LIB_KSYCOCA) $(DPMSLIB) -L$(top_builddir)/kcontrol/lib -lkcontroltools
libkcm_screensaver_la_METASOURCES = scrnsave.moc

noinst_HEADERS = scrnsave.h

messages:
	$(XGETTEXT) -kni18n $(libkcm_screensaver_la_SOURCES) -o $(podir)/kcmscreensaver.pot

lnf_DATA = screensaver.desktop
lnfdir = $(kde_appsdir)/Settings/LookNFeel

EXTRA_DIST = $(lnf_DATA)
