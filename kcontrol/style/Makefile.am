# $Id$

INCLUDES = $(all_includes)

SUBDIRS = .

kde_module_LTLIBRARIES = libkcm_style.la

libkcm_style_la_SOURCES = kcmstyle.cpp stylepreview.ui menupreview.cpp
libkcm_style_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_style_la_LIBADD = $(LIB_KFILE) 
libkcm_style_la_METASOURCES = AUTO

noinst_HEADERS = kcmstyle.h stylepreview.h menupreview.h

messages: rc.cpp
	$(XGETTEXT) -kni18n *.cpp -o $(podir)/kcmstyle.pot

style_DATA = style.desktop
styledir = $(kde_appsdir)/Settings/LookNFeel

install-data-local:

EXTRA_DIST = $(style_DATA)
