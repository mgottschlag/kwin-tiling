EXTRA_DIST = kdmrc.in Xservers.in Xaccess Xsession Xstartup.in Xreset.in Xsetup

install-data-local:
	@if test ! -d $(DESTDIR)$(kde_confdir)/kdm; then \
	    $(mkinstalldirs) $(DESTDIR)$(kde_confdir)/kdm; \
	    for i in $(EXTRA_DIST); do \
		f=$${i%.in}; \
		echo $(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(kde_confdir)/kdm/$$f; \
		$(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(kde_confdir)/kdm/$$f; \
	    done; \
	fi
