# $Id$
# kdebase/kcontrol/display

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =  $(LIB_KFILE) $(LIB_KIMGIO) $(DPMSLIB)

SUBDIRS = kcs ad pics

bin_PROGRAMS = kcmdisplay

kcmdisplay_SOURCES = colorscm.cpp backgnd.cpp \
		display.cpp savescm.cpp scrnsave.cpp \
		general.cpp main.cpp widgetcanvas.cpp \
		fonts.cpp energy.cpp

METASOURCES = backgnd.moc display.moc colorscm.moc savescm.moc \
		scrnsave.moc general.moc widgetcanvas.moc \
		fonts.moc energy.moc

noinst_HEADERS = backgnd.h display.h colorscm.h savescm.h scrnsave.h \
		general.h widgetcanvas.h fonts.h energy.h

EXTRA_DIST = README

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmdisplay_SOURCES) && mv messages.po ../../po/kcmdisplay.pot
