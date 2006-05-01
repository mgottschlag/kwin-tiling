INCLUDES= $(all_includes)
METASOURCES = AUTO
SUBDIRS = pics

# it's basicly impossible to get this done with all the X headers used ;(
KDE_OPTIONS = nofinal

bin_PROGRAMS = 
lib_LTLIBRARIES = 
kdeinit_LTLIBRARIES = kxkb.la
kde_module_LTLIBRARIES = kcm_keyboard.la

kcm_keyboard_la_SOURCES = extension.cpp rules.cpp kcmlayout.cpp pixmap.cpp kcmlayoutwidget.ui kcmmisc.cpp kcmmiscwidget.ui
kcm_keyboard_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_keyboard_la_LIBADD = -lxkbfile $(XTESTLIB) $(LIB_KIO)

#kxkb_la_includedir = .
#kxkb_la_include_HEADERS = kxkb.h
kxkb_la_SOURCES = kxkb.cpp rules.cpp extension.cpp pixmap.cpp kxkb.skel
kxkb_la_LDFLAGS = $(all_libraries) -module -avoid-version
kxkb_la_LIBADD = -lxkbfile -lX11 -lXext $(LIB_KDEUI)

xdg_apps_DATA = keyboard.desktop keyboard_layout.desktop

install-data-local: 
	$(mkinstalldirs) $(DESTDIR)$(kde_confdir)
	$(INSTALL_DATA) $(srcdir)/kxkb_groups $(DESTDIR)$(kde_confdir)/kxkb_groups

uninstall-local:
	-rm -f $(DESTDIR)$(kde_confdir)/kxkb_groups

servicesdir = $(kde_servicesdir)
services_DATA = kxkb.desktop


KDE_ICON=AUTO
