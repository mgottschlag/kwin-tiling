
project(kdm)


add_subdirectory( kfrontend )
MESSAGE(STATUS "Readd Backend when test about pam and co will add")
#add_subdirectory( backend )

########### install files ###############

install_files( /share/doc/kdm FILES README )




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
