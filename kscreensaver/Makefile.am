# $Id$
# Makefile.am for kscreensaver.
#

AM_CPPFLAGS = -UQT_NO_ASCII_CAST

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDEUI) -lm

bin_PROGRAMS = krandom.kss kblankscrn.kss

krandom_kss_SOURCES = random.cpp

kblankscrn_kss_SOURCES = blankscrn.cpp
kblankscrn_kss_LDADD = $(LIB_KDEUI) -lkscreensaver -lm 

METASOURCES = blankscrn.moc

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kscreensaver.pot

########## Meta objects ##########

desktop_DATA = KRandom.desktop KBlankscreen.desktop
desktopdir = $(kde_appsdir)/System/ScreenSavers

EXTRA_DIST = $(desktop_DATA)
