
INCLUDES = $(all_includes) 

data_DATA = AAAAAAGeneral.ad AAAMotif.ad AAATk.ad AAAXaw.ad Editres.ad Emacs.ad \
	ML.ad Nedit.ad Netscape.ad RVPlayer.ad WPerfect.ad XCalc.ad XTerm.ad XV.ad 

datadir = $(kde_datadir)/kdisplay/app-defaults

services_DATA = krdb.desktop
servicesdir = $(kde_servicesdir)

EXTRA_DIST = $(data_DATA)


lib_LTLIBRARIES = krdb.la

krdb_la_LDFLAGS = $(all_libraries) -module -avoid-version 
krdb_la_LIBADD =  $(LIB_KDEUI) 
krdb_la_SOURCES = krdb.cpp

bin_PROGRAMS = krdb
krdb_LDFLAGS = $(all_libraries) $(KDE_RPATH)
krdb_LDADD =   krdb.la
krdb_SOURCES = dummy.cpp

messages:
	$(XGETTEXT) *.cpp -o $(podir)/krdb.pot

dummy.cpp:
	echo > dummy.cpp
