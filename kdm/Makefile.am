# frontend must come first, as it generates config.ci
SUBDIRS = kfrontend backend

PAM = @KDM_PAM_SERVICE@

kdmdocdir = $(datadir)/doc/kdm
kdmdoc_DATA = README

install-data-local:
	-@test -n "$(DESTDIR)" || test -z "$(PAM)" || { $(top_srcdir)/mkpamserv $(PAM) && $(top_srcdir)/mkpamserv -P $(PAM)-np; }

### this is *only* for the kdm home page maintainer! ###
hp:
	scp README ChangeLog TODO devel-home:files/kdm
