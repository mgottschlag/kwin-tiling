SUBDIRS = StarOffice 

EXTRA_DIST = README psnames

install-exec-local:
	$(mkinstalldirs) $(kde_datadir)/kcmfontinst/
	$(INSTALL_DATA) $(srcdir)/psnames $(kde_datadir)/kcmfontinst/psnames
	

uninstall-local:
	-rm -f $(kde_datadir)/kcmfontinst/psnames

