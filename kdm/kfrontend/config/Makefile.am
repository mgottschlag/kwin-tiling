EXTRA_DIST = kdmrc.in Xservers.in Xaccess Xsession Xstartup.in Xreset.in Xsetup

install-data-local:
	@if test ! -d $(DESTDIR)$(kde_confdir)/kdm; then \
	    $(mkinstalldirs) $(DESTDIR)$(kde_confdir)/kdm; \
	    for i in $(EXTRA_DIST:.in=); do \
		echo $(INSTALL_DATA) $(srcdir)/$$i $(DESTDIR)$(kde_confdir)/kdm/$$i; \
		$(INSTALL_DATA) $(srcdir)/$$i $(DESTDIR)$(kde_confdir)/kdm/$$i; \
	    done; \
	fi
