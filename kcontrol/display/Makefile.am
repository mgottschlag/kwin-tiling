# $Id$
# kdebase/kcontrol/display

INCLUDES = $(all_includes) -I$(top_srcdir)/kdesktop
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =  $(LIB_KDEUI) $(LIB_KFILE) $(LIB_KIMGIO) $(DPMSLIB) $(top_builddir)/kdesktop/libbgnd.la

SUBDIRS = . kcs ad pics maps

#bin_PROGRAMS = kcmdisplay

#kcmdisplay_SOURCES = colorscm.cpp backgnd.cpp display.cpp savescm.cpp \
#	scrnsave.cpp general.cpp main.cpp widgetcanvas.cpp  fonts.cpp \
#	energy.cpp bgdialogs.cpp advanced.cpp

METASOURCES = AUTO

noinst_HEADERS = backgnd.h display.h colorscm.h savescm.h scrnsave.h \
		general.h widgetcanvas.h fonts.h energy.h bgdialogs.h \
		advanced.h stdclient_bitmaps.h

EXTRA_DIST = README

lib_LTLIBRARIES = libkcm_display.la
libkcm_display_la_SOURCES = advanced.cpp fonts.cpp scrnsave.cpp backgnd.cpp \
	bgdialogs.cpp
libkcm_display_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_RPATH)
libkcm_display_la_LIBADD = $(LIB_KDEUI) $(LIB_KIMGIO) $(LIB_KFILE) \
	$(top_builddir)/kdesktop/libbgnd.la

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -kni18n -x $(includedir)/kde.pot $(kcmdisplay_SOURCES) && mv messages.po ../../po/kcmdisplay.pot
