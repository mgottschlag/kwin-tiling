kde_module_LTLIBRARIES = kcm_kded.la

kcm_kded_la_SOURCES = kcmkded.cpp kxmlrpcdlg.cpp kxmlrpcdlgbase.ui

kcm_kded_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_kded_la_LIBADD =  $(LIB_KIO)

INCLUDES= $(all_includes)

kcm_kded_la_METASOURCES = AUTO

KDE_ICON = kded

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmkded.pot

kcmkded_data_DATA = kcmkded.desktop
kcmkded_datadir = $(kde_appsdir)/Settings/Components
