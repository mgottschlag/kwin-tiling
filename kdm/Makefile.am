# backend needs to be conditional (in case the xdm core is adopted by x)
SUBDIRS = backend kfrontend

EXTRA_DIST = misc

PAM = @KDM_PAM_SERVICE@

install-data-local:
	-@test -n "$(DESTDIR)" || test -z "$(PAM)" || $(top_srcdir)/mkpamserv $(PAM)
