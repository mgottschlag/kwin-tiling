# $Id$
# kdebase/kcontrol/kwm

SUBDIRS = pics

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDEUI)

lib_LTLIBRARIES = libkcm_kwinmouse.la libkcm_kwinoptions.la
libkcm_kwinmouse_la_SOURCES = mouse.cpp
libkcm_kwinmouse_la_LDFLAGS = -module -avoid-version $(all_libraries)
libkcm_kwinmouse_la_METASOURCES = AUTO

libkcm_kwinoptions_la_SOURCES = windows.cpp
libkcm_kwinoptions_la_LDFLAGS = -module -avoid-version $(all_libraries)
libkcm_kwinoptions_la_METASOURCES = AUTO

bin_PROGRAMS = kcmkwm
kcmkwm_SOURCES = main.cpp titlebar.cpp desktop.cpp advanced.cpp
METASOURCES = titlebar.moc windows.moc desktop.moc advanced.moc
noinst_HEADERS = windows.h desktop.h titlebar.h geom.h mouse.h advanced.h

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmkwm_SOURCES) && mv messages.po $(podir)/kcmkwm.pot

