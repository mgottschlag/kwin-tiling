EXTRA_DIST = odbc.xpm

kde_icon_DATA = odbc.xpm

picsdir = $(kde_datadir)/odbc/pics
pics_DATA = odbc.xpm

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/odbc.xpm $(kde_minidir)/odbc.xpm

uninstall-local:
	-rm -f $(kde_minidir)/odbc.xpm
