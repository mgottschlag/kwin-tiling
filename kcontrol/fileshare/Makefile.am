kde_module_LTLIBRARIES = kcm_fileshare.la

kcm_fileshare_la_SOURCES = fileshare.cpp

kcm_fileshare_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_fileshare_la_LIBADD = -lkdeui 

INCLUDES= $(all_includes)

kcm_fileshare_la_METASOURCES = fileshare.moc

KDE_ICON = fileshare

messages:
	$(XGETTEXT) $(kcm_fileshare_la_SOURCES) -o $(podir)/kcmfileshare.pot

data_DATA = fileshare.desktop
datadir = $(kde_appsdir)/Settings/System

sbin_SCRIPTS = fileshareset
install-exec-hook:
		@(ln -fs $(sbindir)/fileshareset $(DESTDIR)/$(sbindir)/filesharelist)
	        @(chown root $(DESTDIR)/$(sbindir)/fileshareset && chmod 4755 $(DESTDIR)/$(sbindir)/fileshareset) || echo "Was not able to make fileshareset setuid root"
