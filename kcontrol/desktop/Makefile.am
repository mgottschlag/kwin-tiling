lib_LTLIBRARIES = libkcm_virtualdesktops.la

libkcm_virtualdesktops_la_SOURCES = desktop.cpp 

libkcm_virtualdesktops_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_virtualdesktops_la_LIBADD = -lkdeui -lkfile 
INCLUDES= $(all_includes) 

libkcm_virtualdesktops_la_METASOURCES = desktop.moc

noinst_HEADERS = desktop.h

KDE_ICON = desktop

messages:	
	$(XGETTEXT) $(libkcm_virtualdesktops_la_SOURCES) -o $(podir)/kcmvirtualdesktops.pot

data_DATA = virtualdesktops.desktop
datadir = $(kde_appsdir)/Settings/LookNFeel/Desktop
