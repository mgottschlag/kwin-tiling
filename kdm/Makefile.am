# backend needs to be conditional; chooser will go away.
# this does not support builddir, everything is done in srcdir!
SUBDIRS = kfrontend @kdmchooserdir@

EXTRA_DIST = misc

dist-hook:
	mkdir $(distdir)/backend
	cd $(srcdir)/backend; \
	for i in *.c *.h Imakefile; do \
	    cp -p $$i $(distdir)/backend; \
	done

$(srcdir)/backend/Makefile: $(srcdir)/backend/Imakefile
	cd $(srcdir)/backend; $(XMKMF) -a $(IMAKEFLAGS)
	rm -f $(srcdir)/backend/Makefile.bak

all-local: $(srcdir)/backend/Makefile
	$(MAKE) -C backend

clean-local: $(srcdir)/backend/Makefile
	$(MAKE) -C backend clean

distclean-local:
	rm -f $(srcdir)/backend/Makefile

install-exec-local:
	$(INSTALL_PROGRAM) $(srcdir)/backend/xdm $(DESTDIR)$(bindir)/kdm

uninstall-local:
	rm -f $(DESTDIR)$(bindir)/kdm

