EXTRA_DIST = microsoft-cp1252.xpp README staroffice.patch

install-exec-local:
	$(mkinstalldirs) $(kde_datadir)/kfontinst/StarOffice
	$(INSTALL_DATA) $(srcdir)/microsoft-cp1252.xpp $(kde_datadir)/kfontinst/StarOffice/microsoft-cp1252.xpp
	

uninstall-local:
	-rm -f $(kde_datadir)/kfontinst/StarOffice/microsoft-cp1252.xpp
