INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = libkcm_randr.la

libkcm_randr_la_METASOURCES = ktimerdialog.moc krandrmodule.moc
libkcm_randr_la_SOURCES = ktimerdialog.cpp randr.cpp krandrmodule.cpp
libkcm_randr_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_randr_la_LIBADD = $(LIB_KFILE) $(LIB_XRANDR)

noinst_HEADERS = randr.h krandrmodule.h krandrtray.h krandrapp.h ktimerdialog.h

module_DATA = krandrmodule.desktop
moduledir = $(kde_appsdir)/Settings/Desktop

#install-data-local: uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Settings/Desktop
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop 
#	$(DESTDIR)$(kde_appsdir)/Settings/Desktop/krandrmodule.desktop

bin_PROGRAMS = krandrtray

krandrtray_METASOURCES = ktimerdialog.moc krandrtray.moc krandrapp.moc
krandrtray_SOURCES = ktimerdialog.cpp main.cpp randr.cpp krandrtray.cpp krandrapp.cpp
krandrtray_LDFLAGS = $(all_libraries) $(KDE_RPATH)
krandrtray_LDADD = $(LIB_KFILE) $(LIB_XRANDR)

app_DATA = krandrtray.desktop
appdir = $(kde_appsdir)/System

EXTRA_DIST = $(module_DATA) $(app_DATA)

messages:
	$(XGETTEXT) -kni18n $(libkcm_randr_la_SOURCES) $(krandrtray_SOURCES) -o $(podir)/krandr.pot

