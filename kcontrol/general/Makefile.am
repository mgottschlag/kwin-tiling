# $Id$

INCLUDES = $(all_includes)

SUBDIRS = .

kde_module_LTLIBRARIES = libkcm_general.la

libkcm_general_la_SOURCES = main.cpp widgetsettings.ui
libkcm_general_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_general_la_LIBADD = $(LIB_KFILE) 
libkcm_general_la_METASOURCES = general.moc

noinst_HEADERS = widgetsettings.h

messages:
	$(XGETTEXT) -kni18n $(libkcm_general_la_SOURCES) -o $(podir)/kcmgeneral.pot

general_DATA = widgetsettings.desktop
generaldir = $(kde_appsdir)/Settings/LookNFeel

install-data-local:

EXTRA_DIST = $(general_DATA)
