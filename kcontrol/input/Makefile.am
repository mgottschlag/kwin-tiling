# $Id$
# kdebase/kcontrol/input

SUBDIRS = pics

INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIB_KDEUI) 

bin_PROGRAMS = kcminput

kcminput_SOURCES = main.cpp mouse.cpp keyboard.cpp 
METASOURCES = mouse.moc keyboard.moc 
noinst_HEADERS = mouse.h keyboard.h geom.h 

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcminput_SOURCES) && mv messages.po ../../po/kcminput.pot

