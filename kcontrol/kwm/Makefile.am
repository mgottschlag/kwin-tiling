# $Id$
# kdebase/kcontrol/kwm

SUBDIRS = pics

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkcm_kwinmouse.la libkcm_kwinoptions.la
libkcm_kwinmouse_la_SOURCES = mouse.cpp
libkcm_kwinmouse_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_kwinmouse_la_LIBADD = $(LIB_KDEUI)

METASOURCES = AUTO

libkcm_kwinoptions_la_SOURCES = windows.cpp
libkcm_kwinoptions_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_kwinoptions_la_METASOURCES = AUTO
libkcm_kwinoptions_la_LIBADD = $(LIB_KDEUI)

bin_PROGRAMS = kcmkwm
kcmkwm_SOURCES = main.cpp titlebar.cpp desktop.cpp advanced.cpp
noinst_HEADERS = windows.h desktop.h titlebar.h geom.h mouse.h advanced.h
kcmkwm_LDFLAGS = $(KDE_RPATH)  $(all_libraries)
kcmkwm_LDADD = $(LIB_KDEUI)

messages:
	$(XGETTEXT) $(kcmkwm_SOURCES) -o $(podir)/kcmkwm.pot

