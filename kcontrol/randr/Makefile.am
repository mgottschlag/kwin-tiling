INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = libkcm_randr.la

noinst_LTLIBRARIES = librandrinternal.la

librandrinternal_la_SOURCES = ktimerdialog.cpp randr.cpp
librandrinternal_la_METASOURCES = AUTO

libkcm_randr_la_SOURCES = krandrmodule.cpp
libkcm_randr_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_randr_la_LIBADD = librandrinternal.la $(LIB_KFILE) $(LIB_XRANDR)

noinst_HEADERS = randr.h krandrmodule.h krandrtray.h krandrapp.h ktimerdialog.h

module_DATA = krandrmodule.desktop
moduledir = $(kde_appsdir)/Settings/Desktop

#install-data-local: uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Settings/Desktop
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop 
#	$(DESTDIR)$(kde_appsdir)/Settings/Desktop/krandrmodule.desktop

bin_PROGRAMS = krandrtray

krandrtray_SOURCES = main.cpp krandrtray.cpp krandrapp.cpp
krandrtray_LDFLAGS = $(all_libraries) $(KDE_RPATH)
krandrtray_LDADD = librandrinternal.la $(LIB_KFILE) $(LIB_XRANDR)

app_DATA = krandrtray.desktop
appdir = $(kde_appsdir)/System

EXTRA_DIST = $(module_DATA) $(app_DATA)

messages:
	$(XGETTEXT) -kni18n $(libkcm_randr_la_SOURCES) $(krandrtray_SOURCES) -o $(podir)/krandr.pot

