lib_LTLIBRARIES = libkcm_numberandnames.la

libkcm_numberandnames_la_SOURCES = desktop.cpp 

libkcm_numberandnames_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_numberandnames_la_LIBADD = -lkdeui -lkfile 
INCLUDES= $(all_includes) 

libkcm_numberandnames_la_METASOURCES = desktop.moc

noinst_HEADERS = desktop.h

KDE_ICON = desktop

messages:	
	$(XGETTEXT) $(libkcm_numberandnames_la_SOURCES) -o $(podir)/kcmnumberandnames.pot

data_DATA = numberandnames.desktop
datadir = $(kde_appsdir)/Settings/LookNFeel/Desktop
