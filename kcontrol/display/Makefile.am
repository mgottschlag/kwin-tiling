# $Id$
# kdebase/kcontrol/display

INCLUDES = $(all_includes)

#SUBDIRS = . kcs ad pics

lib_LTLIBRARIES = libkcm_background.la libkcm_colors.la libkcm_energy.la libkcm_fonts.la libkcm_icons.la libkcm_screensaver.la libkcm_style.la

libkcm_background_la_SOURCES = backgnd.cpp bgdialogs.cpp
libkcm_background_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_background_la_LIBADD = $(LIB_KFILE) libbgnd.la
libkcm_background_la_METASOURCES = backgnd.moc bgdialogs.moc

libkcm_colors_la_SOURCES = colorscm.cpp widgetcanvas.cpp
libkcm_colors_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_colors_la_LIBADD = $(LIB_KDEUI)
libkcm_colors_la_METASOURCES = colorscm.moc widgetcanvas.moc

libkcm_energy_la_SOURCES = energy.cpp
libkcm_energy_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_energy_la_LIBADD = $(LIB_KDEUI) $(DPMSLIB)
libkcm_energy_la_METASOURCES = energy.moc

libkcm_fonts_la_SOURCES = fonts.cpp
libkcm_fonts_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_fonts_la_LIBADD = $(LIB_KDEUI) $(DPMSLIB)
libkcm_fonts_la_METASOURCES = fonts.moc

libkcm_icons_la_SOURCES = icnmain.cpp icngeneral.cpp icnlabel.cpp
libkcm_icons_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_icons_la_LIBADD = $(LIB_KFILE)
libkcm_icons_la_METASOURCES = icnmain.moc icngeneral.moc icnlabel.moc

libkcm_screensaver_la_SOURCES = scrnsave.cpp
libkcm_screensaver_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_screensaver_la_LIBADD = $(LIB_KDEUI) $(DPMSLIB)
libkcm_screensaver_la_METASOURCES = scrnsave.moc

libkcm_style_la_SOURCES = general.cpp
libkcm_style_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_style_la_LIBADD = $(LIB_KFILE)
libkcm_style_la_METASOURCES = general.moc


noinst_HEADERS = backgnd.h  colorscm.h scrnsave.h general.h widgetcanvas.h \
	fonts.h energy.h bgdialogs.h stdclient_bitmaps.h \
	bgsettings.h bgrender.h bgdefaults.h icnmain.h icngeneral.h icnlabel.h

noinst_LTLIBRARIES = libbgnd.la
libbgnd_la_SOURCES = bgsettings.cc bgrender.cc
libbgnd_la_LIBADD = $(LIB_KSYCOCA)
libbgnd_la_LDFLAGS = -no-undefined $(all_libraries)
libbgnd_la_METASOURCES = bgrender.moc

check_PROGRAMS = testicons
testicons_SOURCES = testicons.cpp icons.cpp
testicons_LDFLAGS = $(all_libraries)
testicons_LDADD = $(LIB_KDEUI)

messages:
	$(XGETTEXT) -kni18n $(libkcm_background_la_SOURCES) -o $(podir)/kcmbackground.pot
	$(XGETTEXT) -kni18n $(libkcm_colors_la_SOURCES) -o $(podir)/kcmcolors.pot
	$(XGETTEXT) -kni18n $(libkcm_energy_la_SOURCES) -o $(podir)/kcmenergy.pot
	$(XGETTEXT) -kni18n $(libkcm_fonts_la_SOURCES) -o $(podir)/kcmfonts.pot
	$(XGETTEXT) -kni18n $(libkcm_icons_la_SOURCES) -o $(podir)/kcmicons.pot
	$(XGETTEXT) -kni18n $(libkcm_screensaver_la_SOURCES) -o $(podir)/kcmscreensaver.pot
	$(XGETTEXT) -kni18n $(libkcm_style_la_SOURCES) -o $(podir)/kcmstyle.pot

lnf_DATA = colors.desktop fonts.desktop screensaver.desktop
lnfdir = $(kde_appsdir)/Settings/LookNFeel

theme_DATA = style.desktop
themedir = $(kde_appsdir)/Settings/LookNFeel/Themes

desktop_DATA = desktop.desktop background.desktop borders.desktop icons.desktop
desktopdir = $(kde_appsdir)/Settings/LookNFeel/Desktop

power_DATA = energy.desktop
powerdir = $(kde_appsdir)/Settings/PowerControl

EXTRA_DIST = README $(lnf_DATA) $(theme_DATA) $(desktop_DATA) $(power_DATA)
