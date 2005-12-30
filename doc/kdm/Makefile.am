conf_def = $(top_srcdir)/kdm/config.def
conf_proc = $(top_srcdir)/kdm/confproc.pl
ref: $(conf_def) $(conf_proc)
	$(PERL) -w $(conf_proc) --doc $(conf_def) $(srcdir)/kdmrc-ref.docbook

KDE_LANG = en
KDE_DOCS = AUTO
