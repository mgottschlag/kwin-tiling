# $Id$
# kdebase/kcontrol/display

INCLUDES = $(all_includes) -I$(srcdir)/../../kdesktop
LDADD =  $(LIB_KDEUI) $(LIB_KFILE) $(LIB_KIMGIO) libbgnd.la

SUBDIRS = . kcs ad pics

noinst_HEADERS = backgnd.h  colorscm.h scrnsave.h general.h widgetcanvas.h \
	fonts.h energy.h bgdialogs.h advanced.h stdclient_bitmaps.h \
	bgsettings.h bgrender.h

lib_LTLIBRARIES = libkcm_display.la
libkcm_display_la_SOURCES = advanced.cpp fonts.cpp scrnsave.cpp backgnd.cpp \
	bgdialogs.cpp energy.cpp colorscm.cpp widgetcanvas.cpp general.cpp
libkcm_display_la_LDFLAGS = -module -avoid-version $(all_libraries) 
libkcm_display_la_LIBADD = $(LIB_KDEUI) $(LIB_KIMGIO) $(LIB_KFILE) $(DPMSLIB) \
	libbgnd.la

noinst_LTLIBRARIES = libbgnd.la
libbgnd_la_SOURCES = bgsettings.cc bgrender.cc
libbgnd_la_LDFLAGS = -static

check_PROGRAMS = test
test_SOURCES = $(libkcm_display_la_SOURCES) test.cpp
test.cpp:
	echo "int main() {};" > test.cpp
test_LDFLAGS  = $(all_libraries) -module -avoid-version
test_LDADD = $(LIB_KDEUI) $(LIB_KIMGIO) $(LIB_KFILE) $(DPMSLIB) libbgnd.la

METASOURCES = AUTO
EXTRA_DIST = README

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -kni18n -x $(includedir)/kde.pot \
	$(libkcm_display_la_SOURCES) && mv messages.po $(podir)/kcmdisplay.pot
