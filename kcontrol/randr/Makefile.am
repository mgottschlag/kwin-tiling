INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = libkcm_randr.la

libkcm_randr_la_SOURCES = randr.cpp krandrmodule.cpp
libkcm_randr_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_randr_la_LIBADD = $(LIB_KFILE) $(LIB_XRANDR)

libkcm_randr_la_METASOURCES = AUTO

noinst_HEADERS = randr.h krandrmodule.h

messages:
	$(XGETTEXT) -kni18n $(libkcm_randr_la_SOURCES) -o $(podir)/kcmrandr.pot

theme_DATA = randr.desktop
themedir = $(kde_appsdir)/Settings/Desktop

#update_DATA = kcmdisplayrc.upd
#updatedir = $(kde_datadir)/kconf_update

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Settings/Desktop
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel/randr.desktop

EXTRA_DIST = $(theme_DATA)
