# $Id$
# kdebase/kcontrol/display

INCLUDES = $(all_includes)

SUBDIRS = . kcs ad pics

noinst_HEADERS = backgnd.h  colorscm.h scrnsave.h general.h widgetcanvas.h \
	fonts.h energy.h bgdialogs.h advanced.h stdclient_bitmaps.h \
	bgsettings.h bgrender.h bgdefaults.h

lib_LTLIBRARIES = libkcm_display.la
libkcm_display_la_SOURCES = advanced.cpp fonts.cpp scrnsave.cpp backgnd.cpp \
	bgdialogs.cpp energy.cpp colorscm.cpp widgetcanvas.cpp general.cpp
libkcm_display_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_display_la_LIBADD = $(LIB_KFILE) $(DPMSLIB) libbgnd.la

noinst_LTLIBRARIES = libbgnd.la
libbgnd_la_SOURCES = bgsettings.cc bgrender.cc
libbgnd_la_LIBADD = $(LIB_KIMGIO)
libbgnd_la_LDFLAGS = -no-undefined $(all_libraries)

check_PROGRAMS = test
test_SOURCES = $(libkcm_display_la_SOURCES) test.cpp
test.cpp:
	echo "int main() {};" > test.cpp
test_LDFLAGS  = $(all_libraries) -module -avoid-version
test_LDADD = $(libkcm_display_la_LIBADD)

METASOURCES = AUTO
EXTRA_DIST = README

messages:
	$(XGETTEXT) -kni18n $(libkcm_display_la_SOURCES) -o $(podir)/kcmdisplay.pot
