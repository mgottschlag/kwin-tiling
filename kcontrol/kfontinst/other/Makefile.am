SUBDIRS = StarOffice 

EXTRA_DIST = README psnames

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/kcmfontinst/
	$(INSTALL_DATA) $(srcdir)/psnames $(DESTDIR)$(kde_datadir)/kcmfontinst/psnames
	

uninstall-local:
	-rm -f $(DESTDIR)$(kde_datadir)/kcmfontinst/psnames

