EXTRA_DIST = microsoft-cp1252.xpp README staroffice.patch

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/kcmfontinst/StarOffice
	$(INSTALL_DATA) $(srcdir)/microsoft-cp1252.xpp $(DESTDIR)$(kde_datadir)/kcmfontinst/StarOffice/microsoft-cp1252.xpp
	

uninstall-local:
	-rm -f $(DESTDIR)$(kde_datadir)/kcmfontinst/StarOffice/microsoft-cp1252.xpp
