EXTRA_DIST = kdmrc.in Xservers.in Xaccess \
    Xsession Xstartup.in Xreset.in Xsetup.in Xwilling

install-data-local:
	@if test ! -d $(DESTDIR)$(kde_confdir)/kdm; then \
	    $(mkinstalldirs) $(DESTDIR)$(kde_confdir)/kdm; \
	    for i in kdmrc Xaccess Xservers; do \
		echo $(INSTALL_DATA) $(srcdir)/$$i $(DESTDIR)$(kde_confdir)/kdm/$$i; \
		$(INSTALL_DATA) $(srcdir)/$$i $(DESTDIR)$(kde_confdir)/kdm/$$i; \
	    done; \
	    for i in Xsetup Xstartup Xreset Xsession Xwilling; do \
		echo $(INSTALL_SCRIPT) $(srcdir)/$$i $(DESTDIR)$(kde_confdir)/kdm/$$i; \
		$(INSTALL_SCRIPT) $(srcdir)/$$i $(DESTDIR)$(kde_confdir)/kdm/$$i; \
	    done; \
	    if test ! -d /var/lib/kdm; then \
		echo mkdir -p /var/lib/kdm; \
		mkdir -p /var/lib/kdm; \
	    fi
	fi
