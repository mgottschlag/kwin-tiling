# $Id$

INCLUDES = $(all_includes)

SUBDIRS = .

kde_module_LTLIBRARIES = libkcm_widgetsettings.la

libkcm_widgetsettings_la_SOURCES = main.cpp widgetsettings.ui
libkcm_widgetsettings_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_widgetsettings_la_LIBADD = $(LIB_KFILE) 
libkcm_widgetsettings_la_METASOURCES = AUTO

noinst_HEADERS = widgetsettings.h

messages: rc.cpp
	$(XGETTEXT) -kni18n *.cpp -o $(podir)/kcmwidgetsettings.pot

widgetsettings_DATA = widgetsettings.desktop
widgetsettingsdir = $(kde_appsdir)/Settings/LookNFeel

install-data-local:

EXTRA_DIST = $(widgetsettings_DATA)
