# $Id$
# kdebase/kcontrol/display

INCLUDES = $(all_includes) -I$(top_srcdir)/kdesktop
LDADD =  $(LIB_KDEUI) $(LIB_KFILE) $(LIB_KIMGIO) 

SUBDIRS = . kcs ad pics maps

bin_PROGRAMS = kcmdisplay

kcmdisplay_SOURCES = display.cpp general.cpp main.cpp
kcmdisplay_LDFLAGS = $(all_libraries) $(KDE_RPATH)

METASOURCES = AUTO

noinst_HEADERS = backgnd.h display.h colorscm.h scrnsave.h \
		general.h widgetcanvas.h fonts.h energy.h bgdialogs.h \
		advanced.h stdclient_bitmaps.h

EXTRA_DIST = README

lib_LTLIBRARIES = libkcm_display.la
libkcm_display_la_SOURCES = advanced.cpp fonts.cpp scrnsave.cpp backgnd.cpp \
	bgdialogs.cpp energy.cpp colorscm.cpp widgetcanvas.cpp
libkcm_display_la_LDFLAGS = -module -avoid-version $(all_libraries) 
libkcm_display_la_LIBADD = $(LIB_KDEUI) $(LIB_KIMGIO) $(LIB_KFILE) $(DPMSLIB) \
	$(top_builddir)/kdesktop/libbgnd.la

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -kni18n -x $(includedir)/kde.pot $(kcmdisplay_SOURCES) && mv messages.po ../../po/kcmdisplay.pot
