## Makefile.am of kdebase/kdesktop/lock

INCLUDES = $(all_includes)
kdesktop_lock_LDFLAGS  = $(all_libraries) $(KDE_RPATH)
kdesktop_lock_LDADD    = $(LIB_KIO)

####### Files

bin_PROGRAMS = kdesktop_lock

kdesktop_lock_SOURCES = lockprocess.cc lockdlg.cc main.cc

noinst_HEADERS = lockprocess.h lockdlg.h

METASOURCES = AUTO

####### Build rules

messages:
	$(XGETTEXT) *.cc *.cpp *.h -o $(podir)/kdesktop_lock.pot

PAM = @KSCREENSAVER_PAM_SERVICE@

install-data-local:
	-@test -n "$(DESTDIR)" || test -z "$(PAM)" || $(top_srcdir)/mkpamserv $(PAM)
