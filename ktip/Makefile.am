INCLUDES = $(all_includes)

SUBDIRS = pics

bin_PROGRAMS = ktip
ktip_SOURCES = ktipwindow.cpp
ktip_LDADD = $(LIB_KDEUI)
ktip_LDFLAGS = $(all_libraries) $(KDE_RPATH)

METASOURCES = AUTO
KDE_ICON = AUTO

xdg_apps_DATA = ktip.desktop

autostart_DATA = ktip.desktop
autostartdir = $(datadir)/autostart

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Toys
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Toys/ktip.desktop


appdata_DATA = tips
appdatadir=$(kde_datadir)/kdewizard

