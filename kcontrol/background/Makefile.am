# $Id$

SUBDIRS = . pics

INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kcm_background.la

kcm_background_la_SOURCES = main.cpp
kcm_background_la_COMPILE_FIRST = backgndbase.h

kcm_background_la_LDFLAGS = $(KDE_RPATH) -module -avoid-version $(all_libraries) -no-undefined
kcm_background_la_LIBADD = libkcmbgnd.la libbgnd.la $(LIB_KIO)

METASOURCES = AUTO

noinst_HEADERS = backgnd.h bgdialogs.h bgsettings.h bgrender.h advancedDialog.h

noinst_LTLIBRARIES = libbgnd.la libkcmbgnd.la
libbgnd_la_SOURCES = bgrender.cc bgsettings.cc
libkcmbgnd_la_SOURCES = bgadvanced.ui backgndbase.ui bgdialogs.cpp advancedDialog.cpp backgnd.cpp 


messages: rc.cpp
	$(XGETTEXT) *.cc *.cpp -o $(podir)/kcmbackground.pot

desktop_DATA =  background.desktop
desktopdir = $(kde_appsdir)/Settings/LookNFeel

EXTRA_DIST = $(desktop_DATA)
