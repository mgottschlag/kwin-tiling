# $Id$
# kdebase/kcontrol/kwm

SUBDIRS = pics

INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDEUI) 

lib_LTLIBRARIES = libkcm_kwinmouse.la
libkcm_kwinmouse_la_SOURCES = mouse.cpp
libkcm_kwinmouse_la_LDFLAGS = -module -avoid-version $(all_libraries)
libkcm_kwinmouse_la_METASOURCES = AUTO

bin_PROGRAMS = kcmkwm
kcmkwm_SOURCES = main.cpp titlebar.cpp windows.cpp desktop.cpp advanced.cpp
kcmkwm_METASOURCES = titlebar.moc windows.moc desktop.moc advanced.moc
noinst_HEADERS = windows.h desktop.h titlebar.h geom.h mouse.h advanced.h

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmkwm_SOURCES) && mv messages.po ../../po/kcmkwm.pot

