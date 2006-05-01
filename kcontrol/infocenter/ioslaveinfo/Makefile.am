kde_module_LTLIBRARIES = kcm_ioslaveinfo.la

kcm_ioslaveinfo_la_SOURCES = kcmioslaveinfo.cpp

kcm_ioslaveinfo_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_ioslaveinfo_la_LIBADD = -lkdeui $(LIB_KIO)

AM_CPPFLAGS= $(all_includes)

METASOURCES = AUTO


xdg_apps_DATA = ioslaveinfo.desktop

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Settings/Information

#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Settings/Network/ioslaveinfo.desktop

