# $Id$
# kdebase/kcontrol/kwm

SUBDIRS = pics

INCLUDES = $(all_includes)
LDADD = $(LIB_KDEUI)

lib_LTLIBRARIES = libkcm_kwinmouse.la libkcm_kwinoptions.la
libkcm_kwinmouse_la_SOURCES = mouse.cpp
libkcm_kwinmouse_la_LDFLAGS = -module -avoid-version $(all_libraries)

METASOURCES = AUTO

libkcm_kwinoptions_la_SOURCES = windows.cpp
libkcm_kwinoptions_la_LDFLAGS = -module -avoid-version $(all_libraries)
libkcm_kwinoptions_la_METASOURCES = AUTO

bin_PROGRAMS = kcmkwm
kcmkwm_SOURCES = main.cpp titlebar.cpp desktop.cpp advanced.cpp
noinst_HEADERS = windows.h desktop.h titlebar.h geom.h mouse.h advanced.h
kcmkwm_LDFLAGS = $(KDE_RPATH)  $(all_libraries)

messages:
	$(XGETTEXT) $(kcmkwm_SOURCES) -o $(podir)/kcmkwm.pot

