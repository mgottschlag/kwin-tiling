# $Id$

SUBDIRS = . pics

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkcm_background.la

libkcm_background_la_SOURCES = backgnd.cpp bgdialogs.cpp
libkcm_background_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_background_la_LIBADD = $(LIB_KFILE) libbgnd.la
libkcm_background_la_METASOURCES = backgnd.moc bgdialogs.moc

noinst_HEADERS = backgnd.h bgdialogs.h bgsettings.h bgrender.h

noinst_LTLIBRARIES = libbgnd.la
libbgnd_la_SOURCES = bgsettings.cc bgrender.cc
libbgnd_la_LIBADD = $(LIB_KSYCOCA)
libbgnd_la_LDFLAGS = -no-undefined $(all_libraries)
libbgnd_la_METASOURCES = bgrender.moc

messages:
	$(XGETTEXT) -kni18n $(libkcm_background_la_SOURCES) -o $(podir)/kcmbackground.pot

desktop_DATA =  background.desktop
desktopdir = $(kde_appsdir)/Settings/LookNFeel/Desktop

EXTRA_DIST = $(desktop_DATA)
