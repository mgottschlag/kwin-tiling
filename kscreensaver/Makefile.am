# $Id$
# Makefile.am for kscreensave.
#

AM_CPPFLAGS = -UQT_NO_ASCII_CAST

INCLUDES = $(all_includes) $(GLINC)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDEUI) -lm

bin_PROGRAMS = kswarm.kss krock.kss \
	kpyro.kss khop.kss $(GL_SAVERS) \
	kbouboule.kss kscience.kss \
	kslidescreen.kss krandom.kss kbsod.kss

EXTRA_PROGRAMS = kmorph3d.kss kspace.kss kpipes.kss


BASE_SRCS = main.cpp demowin.cpp saver.cpp helpers.cpp

XS_SRCS = xs_colors.c xs_hsv.c xs_visual.c
XL_SRCS = xlock.cpp

SUBDIRS = libkscreensaver kdesavers kxsconfig

krandom_kss_SOURCES = random.cpp

kbsod_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) bsod.cpp
kbsod_kss_LDADD= $(LDADD) $(XPMLIB)

kswarm_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) swarm.cpp

krock_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) rock.cpp

kslidescreen_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) $(XS_SRCS) slidescreen.cpp

kpyro_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) pyro.cpp

khop_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) hop.cpp
kbouboule_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) bouboule.cpp
kmorph3d_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) morph3d.cpp
kmorph3d_kss_LDADD = $(LDADD) $(GLLIB)
kpipes_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) pipes.cpp
kpipes_kss_LDADD = $(LDADD) $(GLLIB)
kspace_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) space.cpp
kspace_kss_LDADD = $(LDADD) $(GLLIB)
kscience_kss_SOURCES = $(BASE_SRCS) science.cpp
kscience_kss_LDADD = $(LDADD) -lm

METASOURCES = saver.moc demowin.moc rock.moc \
		pyro.moc hop.moc swarm.moc morph3d.moc space.moc bouboule.moc \
		science.moc pipes.moc slidescreen.moc bsod.moc

noinst_HEADERS = saver.h demowin.h xlock.h rock.h \
		pyro.h hop.h swarm.h space.h bouboule.h  science.h \
		xlockmore.h mode.h helpers.h morph3d.h pipes.h \
		xs_colors.h xs_hsv.h xs_yarandom.h xs_utils.h \
		slidescreen.h amiga.xpm atari.xbm bsod.h mac.xbm xs_visual.h \
		vroot.h

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/klock.pot

########## Meta objects ##########

data_DATA = kscience.gif kscience-small.gif
datadir = $(kde_datadir)/kscreensaver

desktop_DATA = KBouboule.desktop KBsod.desktop \
	KHop.desktop KMorph3D.desktop KPipes.desktop \
	KPyro.desktop KRandom.desktop KRock.desktop \
	KScience.desktop KSlidescreen.desktop \
	KSpace.desktop KSwarm.desktop
desktopdir = $(kde_appsdir)/System/ScreenSavers

EXTRA_DIST = kscience.gif kscience-small.gif kscreensave-0.3.3.lsm $(desktop_DATA)
