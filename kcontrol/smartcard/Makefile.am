INCLUDES = $(all_includes)
kde_module_LTLIBRARIES = libkcm_smartcard.la

libkcm_smartcard_la_SOURCES = smartcard.cpp smartcardbase.ui nosmartcardbase.ui

libkcm_smartcard_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined


libkcm_smartcard_la_LIBADD = -lkdeui -lksmartcard $(LIB_KFILE)



libkcm_smartcard_la_LIBADD = -lkdeui -lkfile -lksmartcard


libkcm_smartcard_la_METASOURCES = AUTO

noinst_HEADERS = smartcard.h

KDE_ICON = smartcard

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmsmartcard.pot

data_DATA = smartcard.desktop
datadir = $(kde_appsdir)/Settings/Peripherals
