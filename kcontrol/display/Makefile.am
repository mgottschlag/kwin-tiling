# $Id$
# kdebase/kcontrol/display

INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =  $(LIB_KFILE) $(LIB_KIMGIO) 

SUBDIRS = kcs ad pics

bin_PROGRAMS = kcmdisplay

kcmdisplay_SOURCES = backgnd.cpp display.cpp \
		colorscm.cpp savescm.cpp scrnsave.cpp \
		general.cpp main.cpp widgetcanvas.cpp \
		kresourceman.cpp

METASOURCES = backgnd.moc display.moc colorscm.moc savescm.moc \
		scrnsave.moc general.moc widgetcanvas.moc \
		kresourceman.moc

noinst_HEADERS = backgnd.h display.h colorscm.h savescm.h scrnsave.h \
		general.h widgetcanvas.h kresourceman.h

EXTRA_DIST = README

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmdisplay_SOURCES) && mv messages.po ../../po/kcmdisplay.pot
