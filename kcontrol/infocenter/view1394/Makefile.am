kde_module_LTLIBRARIES = kcm_view1394.la

kcm_view1394_la_SOURCES = view1394widget.ui view1394.cpp

kcm_view1394_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_view1394_la_LIBADD = $(LIB_KDEUI) -lraw1394

AM_CPPFLAGS= $(all_includes) 

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmview1394.pot

xdg_apps_DATA = kcmview1394.desktop

view1394_vendor_DATA = oui.db
view1394_vendordir = $(kde_datadir)/kcmview1394

