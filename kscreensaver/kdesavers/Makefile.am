# $Id$
# Makefile.am for kscreensave.
#

AM_CPPFLAGS = -UQT_NO_ASCII_CAST

INCLUDES = $(all_includes) $(GLINC)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDEUI) -lkscreensaver -lm -lGL -lGLU

bin_PROGRAMS = kbanner.kss kblankscrn.kss kpolygon.kss kslideshow.kss \
                klines.kss klorenz.kss kblob.kss kvm.kss kmatrix.kss \
                kscience.kss kfountain.kss

kbanner_kss_SOURCES = banner.cpp
kblankscrn_kss_SOURCES = blankscrn.cpp
kpolygon_kss_SOURCES = polygon.cpp
kslideshow_kss_SOURCES = slideshow.cpp
kslideshow_kss_LDADD = $(LIB_KFILE) $(LDADD) $(LIB_KSYCOCA)
klines_kss_SOURCES = lines.cpp
klorenz_kss_SOURCES = lorenz.cpp
kblob_kss_SOURCES = blob.cpp
kvm_kss_SOURCES = kvm.cpp vm.c vm_random.c
kmatrix_kss_SOURCES = matrix.cpp
kscience_kss_SOURCES = science.cpp
kfountain_kss_SOURCES = fountain.cpp

METASOURCES = banner.moc blankscrn.moc polygon.moc slideshow.moc lines.moc \
              lorenz.moc blob.moc kvm.moc matrix.moc science.moc fountain.moc

#METASOURCES = AUTO

noinst_HEADERS = banner.h blankscrn.h polygon.h slideshow.h lines.h\
                 lorenz.h blob.h kvm.h vm.h vm_random.h matrix.h science.h \
		 fountain.h

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/klock.pot

########## Meta objects ##########

data_DATA = kscience.png  particle.png
datadir = $(kde_datadir)/kscreensaver

desktop_DATA = KBanner.desktop KBlankscreen.desktop KPolygon.desktop \
                KSlideshow.desktop KLines.desktop KLorenz.desktop \
                KBlob.desktop KVm.desktop KMatrix.desktop KScience.desktop\
		KFountain.desktop

desktopdir = $(kde_appsdir)/System/ScreenSavers

EXTRA_DIST = $(desktop_DATA) vm.xpm vm.xbm kscience.png  particle.png
