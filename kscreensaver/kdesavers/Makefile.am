# $Id$
# Makefile.am for kscreensave.
#

AM_CPPFLAGS = -UQT_NO_ASCII_CAST

INCLUDES = $(all_includes) $(GLINC) ../libkscreensaver
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDEUI) ../libkscreensaver/libkscreensaver.la -lm

bin_PROGRAMS = kbanner.kss kblankscrn.kss kpolygon.kss kslideshow.kss \
                klines.kss

#EXTRA_PROGRAMS = kmorph3d.kss kspace.kss kpipes.kss kmatrix.kss kvm.kss


kbanner_kss_SOURCES = banner.cpp
kblankscrn_kss_SOURCES = blankscrn.cpp
kpolygon_kss_SOURCES = polygon.cpp
kslideshow_kss_SOURCES = slideshow.cpp
kslideshow_kss_LDADD = $(LIB_KFILE) $(LDADD) $(LIB_KSYCOCA) $(XPMLIB)
klines_kss_SOURCES = lines.cpp

METASOURCES = banner.moc blankscrn.moc polygon.moc slideshow.moc lines.moc

noinst_HEADERS = banner.h blankscrn.h polygon.h slideshow.h lines.h

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/klock.pot

########## Meta objects ##########

data_DATA = 
datadir = $(kde_datadir)/kscreensaver

desktop_DATA = KBanner.desktop KBlankscreen.desktop KPolygon.desktop \
                KSlideshow.desktop KLines.desktop
desktopdir = $(kde_appsdir)/System/ScreenSavers

EXTRA_DIST = $(desktop_DATA)
