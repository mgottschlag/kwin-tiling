# $Id$
# kdebase/kcontrol/display

INCLUDES = $(all_includes) -I$(top_srcdir)/kdesktop
LDFLAGS = -g $(all_libraries) $(KDE_RPATH)  -L$(top_srcdir)/kdesktop
LDADD =  $(LIB_KDEUI) $(LIB_KFILE) $(LIB_KIMGIO) $(DPMSLIB) -lbgnd

CXXFLAGS=-g
SUBDIRS = . kcs ad pics maps

bin_PROGRAMS = kcmdisplay

kcmdisplay_SOURCES = colorscm.cpp backgnd.cpp display.cpp savescm.cpp \
	scrnsave.cpp general.cpp main.cpp widgetcanvas.cpp  fonts.cpp \
	energy.cpp bgdialogs.cpp

METASOURCES = AUTO

noinst_HEADERS = backgnd.h display.h colorscm.h savescm.h scrnsave.h \
		general.h widgetcanvas.h fonts.h energy.h bgdialogs.h

EXTRA_DIST = README

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -kni18n -x $(includedir)/kde.pot $(kcmdisplay_SOURCES) && mv messages.po ../../po/kcmdisplay.pot
