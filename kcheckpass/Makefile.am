## Makefile.am for kcheckpass
## written by Christian Esken
##

INCLUDES= $(all_includes)

bin_PROGRAMS = kcheckpass

kcheckpass_SOURCES = kcheckpass.c \
	checkpass_etcpasswd.c checkpass_pam.c checkpass_shadow.c \
	checkpass_osfc2passwd.c checkpass_aix.c
kcheckpass_LDADD = $(all_libraries) -lkdefakes $(PASSWDLIBS) $(LIBSOCKET)

noinst_HEADERS = kcheckpass.h

EXTRA_DIST = README

PAM = $(KCHECKPASS_PAM_SERVICE)

install-data-local:
	-@test -n "$(DESTDIR)" || test -z "$(PAM)" || $(top_srcdir)/mkpamserv $(PAM)

install-exec-hook:
	@(chown root $(DESTDIR)$(bindir)/kcheckpass && chmod 4755 $(DESTDIR)$(bindir)/kcheckpass) \
	|| echo "Error: Could not install kcheckpass as setuid root (possibly you won't be able to unlock)!!\n"

	
