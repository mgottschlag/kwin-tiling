# $Id$

SUBDIRS = . pics

INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kcm_background.la

kcm_background_la_SOURCES = bgdialogs.cpp backgnd.cpp main.cpp backgndbase.ui \
                            bgadvanced.ui advancedDialog.cpp
kcm_background_la_LDFLAGS = $(KDE_RPATH) -module -avoid-version $(all_libraries) -no-undefined
kcm_background_la_LIBADD = libbgnd.la $(LIB_KIO)

METASOURCES = AUTO

noinst_HEADERS = backgnd.h bgdialogs.h bgsettings.h bgrender.h advancedDialog.h

noinst_LTLIBRARIES = libbgnd.la
libbgnd_la_SOURCES = bgrender.cc bgsettings.cc

messages:
	$(XGETTEXT) -kni18n *.cc *.cpp -o $(podir)/kcmbackground.pot

desktop_DATA =  background.desktop
desktopdir = $(kde_appsdir)/Settings/Desktop

EXTRA_DIST = $(desktop_DATA)
