SUBDIRS = StarOffice 

EXTRA_DIST = README psnames

install-exec-local:
	$(mkinstalldirs) $(kde_datadir)/kfontinst/
	$(INSTALL_DATA) $(srcdir)/psnames $(kde_datadir)/kfontinst/psnames
	

uninstall-local:
	-rm -f $(kde_datadir)/kfontinst/psnames

