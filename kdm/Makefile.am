# backend needs to be conditional; chooser will go away.
SUBDIRS = backend kfrontend @kdmchooserdir@

EXTRA_DIST = misc

install-data-local:
	@test -n "$(DESTDIR)" || $(top_srcdir)/mkpamserv @KDM_PAM_SERVICE@
