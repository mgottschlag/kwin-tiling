kde4_header()
include_directories(  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#conf_def = $(top_srcdir)/kdm/config.def
#conf_proc = $(top_srcdir)/kdm/confproc.pl
#ref: $(conf_def) $(conf_proc)
#	$(PERL) -w $(conf_proc) --doc $(conf_def) $(srcdir)/kdmrc-ref.docbook
#
#KDE_LANG = en
#KDE_DOCS = AUTO
