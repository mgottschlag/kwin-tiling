# $Id$
# kdebase/kcontrol/kwm

SUBDIRS = pics
METASOURCES = AUTO
INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkcm_kwinmouse.la libkcm_kwinoptions.la libkcm_kwindesktop.la
libkcm_kwinmouse_la_SOURCES = mouse.cpp
libkcm_kwinmouse_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_kwinmouse_la_LIBADD = $(LIB_KDEUI)

libkcm_kwinoptions_la_SOURCES = windows.cpp
libkcm_kwinoptions_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_kwinoptions_la_LIBADD = $(LIB_KDEUI)

libkcm_kwindesktop_la_SOURCES = desktop.cpp
libkcm_kwindesktop_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_kwindesktop_la_LIBADD = $(LIB_KDEUI)

# bin_PROGRAMS = kcmkwm
# kcmkwm_SOURCES = main.cpp titlebar.cpp desktop.cpp advanced.cpp
# noinst_HEADERS = windows.h desktop.h titlebar.h geom.h mouse.h advanced.h
noinst_HEADERS = windows.h desktop.h geom.h mouse.h
#kcmkwm_LDFLAGS = $(KDE_RPATH)  $(all_libraries)
#kcmkwm_LDADD = $(LIB_KDEUI)

messages:
	$(XGETTEXT) $(libkcm_kwinmouse_la_SOURCES) $(libkcm_kwinoptions_la_SOURCES) $(libkcm_kwindesktop_la_SOURCES) -o $(podir)/kcmkwm.pot

