kde_module_LTLIBRARIES = kcm_usb.la

kcm_usb_la_SOURCES = kcmusb.cpp usbdevices.cpp usbdb.cpp

kcm_usb_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_usb_la_LIBADD = $(LIB_KDEUI)

AM_CPPFLAGS= $(all_includes) 

METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp *.i18n -o $(podir)/kcmusb.pot

xdg_apps_DATA = kcmusb.desktop

usb_DATA = usb.ids
usbdir = $(kde_datadir)/kcmusb

