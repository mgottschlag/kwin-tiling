# $Id$
# Makefile.am for kscreensave.
#

AM_CPPFLAGS = -UQT_NO_ASCII_CAST

INCLUDES = $(all_includes) $(GLINC) -I$(top_srcdir)/kscreensaver/libkscreensaver
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDEUI) ../libkscreensaver/libkscreensaver.la -lm

bin_PROGRAMS = kbanner.kss kblankscrn.kss kpolygon.kss kslideshow.kss \
                klines.kss klorenz.kss kblob.kss kvm.kss kmatrix.kss

kbanner_kss_SOURCES = banner.cpp
kblankscrn_kss_SOURCES = blankscrn.cpp
kpolygon_kss_SOURCES = polygon.cpp
kslideshow_kss_SOURCES = slideshow.cpp
kslideshow_kss_LDADD = $(LIB_KFILE) $(LDADD) $(LIB_KSYCOCA) $(XPMLIB)
klines_kss_SOURCES = lines.cpp
klorenz_kss_SOURCES = lorenz.cpp
kblob_kss_SOURCES = blob.cpp
kvm_kss_SOURCES = kvm.cpp vm.c vm_random.c
kmatrix_kss_SOURCES = matrix.cpp

METASOURCES = banner.moc blankscrn.moc polygon.moc slideshow.moc lines.moc \
              lorenz.moc blob.moc kvm.moc matrix.moc

noinst_HEADERS = banner.h blankscrn.h polygon.h slideshow.h lines.h \
                 lorenz.h blob.h kvm.h vm.h vm_random.h matrix.h

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/klock.pot

########## Meta objects ##########

data_DATA = 
datadir = $(kde_datadir)/kscreensaver

desktop_DATA = KBanner.desktop KBlankscreen.desktop KPolygon.desktop \
                KSlideshow.desktop KLines.desktop KLorenz.desktop \
                KBlob.desktop KVm.desktop KMatrix.desktop
desktopdir = $(kde_appsdir)/System/ScreenSavers

EXTRA_DIST = $(desktop_DATA) vm.xpm vm.xbm
