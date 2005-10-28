# Makefile.am for kblank screensaver.
#

AM_CPPFLAGS = -UQT_NO_ASCII_CAST

INCLUDES = $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)

bin_PROGRAMS = kblankscrn.kss

kblankscrn_kss_SOURCES = blankscrn.cpp
kblankscrn_kss_LDADD = $(LIB_KDEUI) -lkscreensaver -lm 

METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kscreensaver.pot

########## Meta objects ##########

desktop_DATA = kblank.desktop
desktopdir = $(kde_appsdir)/System/ScreenSavers

EXTRA_DIST = $(desktop_DATA)
