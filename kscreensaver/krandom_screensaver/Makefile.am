# Makefile.am for krandom screensaver.
#

AM_CPPFLAGS = -UQT_NO_ASCII_CAST

INCLUDES = -I$(top_srcdir)/workspace/kscreensaver/libkscreensaver $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)

bin_PROGRAMS = krandom.kss

krandom_kss_SOURCES = random.cpp
krandom_kss_LDADD = $(LIB_KDEUI) -lm

METASOURCES = AUTO


########## Meta objects ##########

desktop_DATA = krandom.desktop
desktopdir = $(kde_appsdir)/System/ScreenSavers

EXTRA_DIST = $(desktop_DATA)
