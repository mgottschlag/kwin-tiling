if include_kdm_chooser
chooser_SUBDIR=chooser
endif

# backend needs to be conditional; chooser will go away.
SUBDIRS = backend kfrontend $(chooser_SUBDIR)

EXTRA_DIST = misc

PAM = @KDM_PAM_SERVICE@

install-data-local:
	-@test -n "$(DESTDIR)" || test -z "$(PAM)" || $(top_srcdir)/mkpamserv $(PAM)
