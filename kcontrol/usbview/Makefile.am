kde_module_LTLIBRARIES = kcm_usb.la

kcm_usb_la_SOURCES = kcmusb.cpp usbdevices.cpp usbdb.cpp

kcm_usb_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_usb_la_LIBADD = $(LIB_KDEUI)

INCLUDES= $(all_includes) 

kcm_usb_la_METASOURCES = AUTO

KDE_ICON = AUTO

messages:
	$(XGETTEXT) *.cpp *.i18n -o $(podir)/kcmusb.pot

kcmusb_data_DATA = kcmusb.desktop
kcmusb_datadir = $(kde_appsdir)/Settings/Information

usb_DATA = usb.ids
usbdir = $(kde_datadir)/kcmusb

