# $Id$
# kdebase/kcontrol/display

INCLUDES = $(all_includes) -I$(top_srcdir)/kdesktop
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =  $(LIB_KDEUI) $(LIB_KFILE) $(LIB_KIMGIO) $(DPMSLIB) $(top_builddir)/kdesktop/libbgnd.la

CXXFLAGS=-g
SUBDIRS = . kcs ad pics maps

bin_PROGRAMS = kcmdisplay

kcmdisplay_SOURCES = colorscm.cpp backgnd.cpp display.cpp savescm.cpp \
	scrnsave.cpp general.cpp main.cpp widgetcanvas.cpp  fonts.cpp \
	energy.cpp bgdialogs.cpp advanced.cpp

METASOURCES = AUTO

noinst_HEADERS = backgnd.h display.h colorscm.h savescm.h scrnsave.h \
		general.h widgetcanvas.h fonts.h energy.h bgdialogs.h \
		advanced.h

EXTRA_DIST = README

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -kni18n -x $(includedir)/kde.pot $(kcmdisplay_SOURCES) && mv messages.po ../../po/kcmdisplay.pot
