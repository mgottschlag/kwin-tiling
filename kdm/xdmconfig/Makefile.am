EXTRA_DIST = xdm-config Xresources Xservers Xaccess Xsession \
	GiveConsole TakeConsole Xsetup_0

install-data-local:
	@if test ! -d $(DESTDIR)$(XDMDIR); then \
	    $(mkinstalldirs) $(DESTDIR)$(XDMDIR); \
	    for i in $(EXTRA_DIST); do \
		echo $(INSTALL_DATA) $(srcdir)/$$i $(DESTDIR)$(XDMDIR)/$$i; \
		$(INSTALL_DATA) $(srcdir)/$$i $(DESTDIR)$(XDMDIR)/$$i; \
	    done; \
	fi
