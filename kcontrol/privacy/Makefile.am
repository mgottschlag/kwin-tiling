INCLUDES    = $(all_includes)
kde_module_LTLIBRARIES = kcm_privacy.la

kcm_privacy_la_SOURCES = privacy.cpp kprivacysettings.ui kcmprivacydialog.ui kprivacymanager.cpp

kcm_privacy_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_privacy_la_LIBADD = $(LIB_KDEUI) -lkio

kcm_privacy_la_METASOURCES = AUTO

noinst_HEADERS = privacy.h

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/privacy.pot

privacy_DATA = privacy.desktop
privacydir = $(kde_appsdir)/Settings/Security

