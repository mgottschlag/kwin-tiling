# $Id$
# kdebase/kcontrol/kcontrol

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIBCOMPAT) $(LIB_KDEUI)

SUBDIRS = pics

bin_PROGRAMS = kcontrol

kcontrol_SOURCES = configlist.cpp main.cpp toplevel.cpp mainwidget.cpp \
		   kswallow.cpp
METASOURCES =	toplevel.moc mainwidget.moc configlist.moc \
		kswallow.moc
noinst_HEADERS = kswallow.h toplevel.h configlist.h mainwidget.h

KDE_ICON = kcontrol

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcontrol_SOURCES) && mv messages.po ../../po/kcontrol.pot


