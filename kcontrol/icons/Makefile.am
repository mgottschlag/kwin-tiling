# $Id$

INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = libkcm_icons.la

libkcm_icons_la_SOURCES = iconthemes.cpp icons.cpp main.cpp
libkcm_icons_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_icons_la_LIBADD = $(LIB_KFILE)
libkcm_icons_la_METASOURCES = icons.moc iconthemes.moc main.moc


noinst_HEADERS = icons.h iconthemes.h main.h

check_PROGRAMS = testicons
testicons_SOURCES = testicons.cpp icons.cpp
testicons_LDFLAGS = $(all_libraries)
testicons_LDADD = $(LIB_KDEUI)

messages:
	$(XGETTEXT) -kni18n $(libkcm_icons_la_SOURCES) -o $(podir)/kcmicons.pot

lnf_DATA = icons.desktop
lnfdir = $(kde_appsdir)/Settings/LookNFeel/

EXTRA_DIST = $(lnf_DATA)
