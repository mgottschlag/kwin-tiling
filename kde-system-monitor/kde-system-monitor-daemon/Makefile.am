AUTOMAKE_OPTIONS = foreign

if include_kde-system-monitor-daemon_linux
linux_SUBDIR = Linux
endif
if include_kde-system-monitor-daemon_freebsd
freebsd_SUBDIR = FreeBSD
endif
if include_kde-system-monitor-daemon_netbsd
netbsd_SUBDIR = NetBSD
endif
if include_kde-system-monitor-daemon_solaris
solaris_SUBDIR = Solaris
endif
if include_kde-system-monitor-daemon_tru64
tru64_SUBDIR = Tru64
endif
if include_kde-system-monitor-daemon_irix
irix_SUBDIR = Irix
endif
if include_kde-system-monitor-daemon_openbsd
openbsd_SUBDIR = OpenBSD
endif

SUBDIRS = $(linux_SUBDIR) $(freebsd_SUBDIR) $(netbsd_SUBDIR)\
	$(solaris_SUBDIR) $(tru64_SUBDIR) $(irix_SUBDIR) $(openbsd_SUBDIR)

KDESYSTEMMONITORDAEMONRCFILE=$(sysconfdir)/kde-system-monitor-daemonrc
INCLUDES = -DKDESYSTEMMONITORDAEMONRCFILE="\"$(KSYSGUARDDRCFILE)\"" -DOSTYPE_$(UNAME) -I$(srcdir)/../CContLib -I$(srcdir)/$(UNAME)

bin_PROGRAMS = ksysguardd

kde-system-monitor-daemon_SOURCES = Command.c conf.c ksysguardd.c PWUIDCache.c
kde-system-monitor-daemon_LDFLAGS = $(all_libraries)
kde-system-monitor-daemon_LDADD =  $(top_builddir)/workspace/kde-system-monitor/ksysguardd/$(UNAME)/libksysguardd.a ../CContLib/libccont.a -lkdefakes_nonpic $(LIBHOSTS) $(LIB_XRES) $(LIB_X11) $(LIB_XEXT)

