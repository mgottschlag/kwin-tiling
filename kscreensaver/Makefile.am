# $Id$
# Makefile.am for kscreensave.
#

AM_CPPFLAGS = -UQT_NO_ASCII_CAST

INCLUDES = $(all_includes) $(GLINC)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDEUI) -lm

bin_PROGRAMS = kpolygon.kss kswarm.kss kbanner.kss \
	klines.kss kblankscrn.kss krock.kss \
	kpyro.kss khop.kss $(XPM_SAVERS) $(GL_SAVERS) \
	kblob.kss kbouboule.kss kscience.kss \
	kslidescreen.kss krandom.kss \
	kslideshow.kss kbsod.kss klorenz.kss

EXTRA_PROGRAMS = kmorph3d.kss kspace.kss kpipes.kss kmatrix.kss kvm.kss


BASE_SRCS = main.cpp demowin.cpp saver.cpp helpers.cpp

XS_SRCS = xs_colors.c xs_hsv.c xs_visual.c
XL_SRCS = xlock.cpp

SUBDIRS = pixmaps bitmaps kxsconfig

krandom_kss_SOURCES = random.cpp

kbsod_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) bsod.cpp
kbsod_kss_LDADD= $(LDADD) $(XPMLIB)

kpolygon_kss_SOURCES = $(BASE_SRCS) polygon.cpp

kswarm_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) swarm.cpp

kbanner_kss_SOURCES = $(BASE_SRCS) banner.cpp

kblankscrn_kss_SOURCES = $(BASE_SRCS) blankscrn.cpp

kslideshow_kss_SOURCES = $(BASE_SRCS) slideshow.cpp
kslideshow_kss_LDADD = $(LIB_KFILE) $(LDADD) $(LIB_KSYCOCA) $(XPMLIB)

krock_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) rock.cpp

kslidescreen_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) $(XS_SRCS) slidescreen.cpp

kpyro_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) pyro.cpp

klines_kss_SOURCES = $(BASE_SRCS) lines.cpp

klorenz_kss_SOURCES = $(BASE_SRCS) lorenz.cpp
klorenz_kss_LDADD= $(LDADD)

kmatrix_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) matrix.cpp
kmatrix_kss_LDADD= $(LDADD) $(XPMLIB)

khop_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) hop.cpp
kblob_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) blob.cpp
kbouboule_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) bouboule.cpp
kmorph3d_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) morph3d.cpp
kmorph3d_kss_LDADD = $(LDADD) $(GLLIB)
kpipes_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) pipes.cpp
kpipes_kss_LDADD = $(LDADD) $(GLLIB)
kspace_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) space.cpp
kspace_kss_LDADD = $(LDADD) $(GLLIB)
kscience_kss_SOURCES = $(BASE_SRCS) science.cpp
kscience_kss_LDADD = $(LDADD) -lm

kvm_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) vm_random.c vm.c kvm.cpp
kvm_kss_LDADD= $(LDADD) $(XPMLIB) -lm

METASOURCES = saver.moc demowin.moc polygon.moc banner.moc\
		blankscrn.moc rock.moc lines.moc\
		pyro.moc hop.moc swarm.moc \
		morph3d.moc blob.moc space.moc bouboule.moc \
		science.moc pipes.moc \
		slidescreen.moc bsod.moc slideshow.moc matrix.moc \
		kvm.moc lorenz.moc

noinst_HEADERS = saver.h demowin.h polygon.h banner.h\
		blankscrn.h xlock.h rock.h lines.h\
		pyro.h hop.h swarm.h \
		blob.h space.h bouboule.h  science.h \
		xlockmore.h mode.h helpers.h morph3d.h pipes.h \
		xs_colors.h xs_hsv.h xs_yarandom.h xs_utils.h \
		slidescreen.h amiga.xpm atari.xbm bsod.h mac.xbm xs_visual.h \
		slideshow.h vroot.h matrix.h vm_random.h vm.h kvm.h lorenz.h

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/klock.pot

########## Meta objects ##########

data_DATA = kscience.gif kscience-small.gif
datadir = $(kde_datadir)/kscreensaver

desktop_DATA = KBanner.desktop KBlankscreen.desktop \
	KBlob.desktop KBouboule.desktop KBsod.desktop \
	KHop.desktop KLines.desktop \
	KMatrix.desktop KMorph3D.desktop KPipes.desktop \
	KPolygon.desktop KPyro.desktop KRandom.desktop KRock.desktop \
	KScience.desktop KSlidescreen.desktop KSlideshow.desktop \
	KSpace.desktop KSwarm.desktop KVm.desktop KLorenz.desktop
desktopdir = $(kde_appsdir)/System/ScreenSavers

EXTRA_DIST = kscience.gif kscience-small.gif kscreensave-0.3.3.lsm $(desktop_DATA)
