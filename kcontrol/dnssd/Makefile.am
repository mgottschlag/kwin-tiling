INCLUDES = -I.. $(all_includes) $(HOWL_CFLAGS)
METASOURCES = AUTO
kde_module_LTLIBRARIES =  kcm_kdnssd.la
kcm_kdnssd_la_LDFLAGS = -avoid-version -module -no-undefined $(all_libraries)
kcm_kdnssd_la_SOURCES = configdialog.ui kcmdnssd.cpp kcmdnssd.h

xdg_apps_DATA = kcm_kdnssd.desktop
kcm_kdnssd_la_LIBADD = ../dnssd/libkdnssd.la $(LIB_KDEUI)

messages: rc.cpp
	$(EXTRACTRC) *.ui *.rc >> rc.cpp
	$(XGETTEXT) $(kcm_kdnssd_la_SOURCES) rc.cpp -o $(podir)/kcmkdnssd.pot
