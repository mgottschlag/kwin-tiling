
project(kdm)
OPTION(KDE4_KDM_XCONSOLE   "build kdm with built-in xconsole" OFF)
OPTION(KDE4_VSYSLOG_SUPPORT "Build kdm with syslog support" OFF)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kdm.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdm.h )


add_subdirectory( kfrontend )
add_subdirectory( backend )

########### install files ###############

install( FILES README DESTINATION ${DATA_INSTALL_DIR}/doc/kdm )
MESSAGE(STATUS "Create pam service")



#original Makefile.am contents follow:

#SUBDIRS = . backend kfrontend
#
#PAM = @KDM_PAM_SERVICE@
#
#noinst_DATA=config.ci
#
#kdmdocdir = $(datadir)/doc/kdm
#kdmdoc_DATA = README
#
#install-data-local:
#	-@test -n "$(DESTDIR)" || test -z "$(PAM)" || { $(top_srcdir)/mkpamserv $(PAM) && $(top_srcdir)/mkpamserv -P $(PAM)-np; }
#
#config.ci: $(srcdir)/config.def $(srcdir)/confproc.pl
#	$(PERL) -w $(srcdir)/confproc.pl $(srcdir)/config.def $@
#
#CLEANFILES = config.ci
#
#### this is *only* for the kdm home page maintainer! ###
#hp:
#	scp README ChangeLog TODO devel-home:files/kdm
