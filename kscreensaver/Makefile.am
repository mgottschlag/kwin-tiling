# $Id$
# Makefile.am for kscreensave.   
#

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDEUI) -lm 

bin_PROGRAMS = kpolygon.kss kswarm.kss klissie.kss kbanner.kss \
	klines.kss kblankscrn.kss krock.kss kflame.kss klaser.kss \
	kpyro.kss khop.kss $(XPM_SAVERS) $(GL_SAVERS) kslip.kss \
	kforest.kss kblob.kss kbouboule.kss kscience.kss \
	kattraction.kss kslidescreen.kss kbsod.kss krandom.kss \
	kmatrix.kss

EXTRA_PROGRAMS = kbat.kss kmorph3d.kss kspace.kss kpipes.kss

KDE_ICON = kscreensaver

EXTRA_DIST = kscience.gif kscience-small.gif kscreensave-0.3.3.lsm

BASE_SRCS = main.cpp demowin.cpp saver.cpp helpers.cpp

XS_SRCS = xs_colors.c xs_hsv.c xs_visual.c
XL_SRCS = xlock.cpp

SUBDIRS = pixmaps bitmaps

krandom_kss_SOURCES = random.cpp

kbsod_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) bsod.cpp
kbsod_kss_LDADD= $(LDADD) $(XPMLIB)

kpolygon_kss_SOURCES = $(BASE_SRCS) polygon.cpp

kswarm_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) swarm.cpp

klissie_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) lissie.cpp

kbanner_kss_SOURCES = $(BASE_SRCS) banner.cpp  kgroupbox.cpp

kblankscrn_kss_SOURCES = $(BASE_SRCS) blankscrn.cpp

kbat_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) bat.cpp
kbat_kss_LDADD = $(LDADD) $(XPMLIB)

krock_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) rock.cpp

kslip_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) slip.cpp

kattraction_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) $(XS_SRCS) attraction.cpp

kslidescreen_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) $(XS_SRCS) slidescreen.cpp

kpyro_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) pyro.cpp

klines_kss_SOURCES = $(BASE_SRCS) lines.cpp

klaser_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) laser.cpp

khop_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) hop.cpp
kflame_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) flame.cpp
kforest_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) forest.cpp
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

kmatrix_kss_SOURCES = $(BASE_SRCS) $(XL_SRCS) matrix.cpp
kmatrix_kss_LDADD = $(LDADD) $(XPMLIB)


METASOURCES = saver.moc demowin.moc polygon.moc banner.moc\
		blankscrn.moc rock.moc flame.moc lines.moc laser.moc\
		pyro.moc hop.moc forest.moc swarm.moc bat.moc slip.moc\
		lissie.moc morph3d.moc blob.moc space.moc bouboule.moc \
		kgroupbox.moc science.moc pipes.moc attraction.moc \
        slidescreen.moc bsod.moc matrix.moc

noinst_HEADERS = saver.h demowin.h polygon.h banner.h\
		blankscrn.h xlock.h rock.h flame.h lines.h\
		laser.h pyro.h hop.h forest.h swarm.h bat.h slip.h\
		lissie.h blob.h space.h bouboule.h  kgroupbox.h science.h \
		xlockmore.h mode.h helpers.h morph3d.h pipes.h \
		attraction.h xs_colors.h xs_hsv.h xs_yarandom.h xs_utils.h \
        slidescreen.h amiga.xpm atari.xbm bsod.h mac.xbm xs_visual.h matrix.h 

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot *.cpp *.h && mv messages.po ../po/klock.pot

########## Meta objects ##########

data_DATA = kscience.gif kscience-small.gif
datadir = $(kde_datadir)/kscreensaver


