INCLUDES = $(all_includes)
kde_module_LTLIBRARIES = kcm_smartcard.la

kcm_smartcard_la_SOURCES = smartcard.cpp smartcardbase.ui nosmartcardbase.ui smartcard.skel

kcm_smartcard_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined

kcm_smartcard_la_LIBADD = -lkdeui -lksmartcard $(LIB_KIO)

kcm_smartcard_la_METASOURCES = AUTO

noinst_HEADERS = smartcard.h

KDE_ICON = smartcard

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmsmartcard.pot

data_DATA = smartcard.desktop
datadir = $(kde_appsdir)/Settings/Peripherals
