## Makefile.am of kdebase/kdesktop/lock

INCLUDES = -I.. -I$(top_srcdir)/workspace/lib -I$(top_srcdir)/kcheckpass -I$(top_srcdir)/kdmlib $(GLINC) $(all_includes)
kdesktop_lock_LDFLAGS  = $(all_libraries) $(KDE_RPATH)
kdesktop_lock_LDADD    = ../libkdesktopsettings.la ../../kdmlib/libdmctl.la $(LIB_KIO) $(LIB_XF86MISC) $(GLLIB)

####### Files

bin_PROGRAMS = kdesktop_lock

kdesktop_lock_SOURCES = lockprocess.cc lockdlg.cc autologout.cc main.cc

noinst_HEADERS = lockprocess.h lockdlg.h autologout.h main.h

METASOURCES = AUTO

lockprocess.o: ../kdesktopsettings.h

####### Build rules

PAM = @KSCREENSAVER_PAM_SERVICE@

install-data-local:
	-@test -n "$(DESTDIR)" || test -z "$(PAM)" || $(top_srcdir)/mkpamserv $(PAM)
