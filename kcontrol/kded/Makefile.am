kde_module_LTLIBRARIES = kcm_kded.la

kcm_kded_la_SOURCES = kcmkded.cpp

kcm_kded_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_kded_la_LIBADD =  $(LIB_KDEUI)

INCLUDES= $(all_includes)

kcm_kded_la_METASOURCES = kded.moc

KDE_ICON = kded

messages:
	$(XGETTEXT) $(kcm_kded_la_SOURCES) -o $(podir)/kcmbell.pot

data_DATA = kcmkded.desktop
datadir = $(kde_appsdir)/Settings/System
