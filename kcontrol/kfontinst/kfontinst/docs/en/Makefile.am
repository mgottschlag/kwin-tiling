EXTRA_DIST = font-installer.html

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/kcontrol/
	$(INSTALL_DATA) $(srcdir)/font-installer.html $(DESTDIR)$(kde_htmldir)/en/kcontrol/font-installer.html

uninstall-local:
	-rm -f $(DESTDIR)$(kde_htmldir)/en/kcontrol/font-installer.html

