## Makefile.am for kcmpasswords

INCLUDES= $(all_includes)

kde_module_LTLIBRARIES = kcm_passwords.la
kcm_passwords_la_SOURCES = passwords.cpp
kcm_passwords_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_passwords_la_LIBADD = $(LIB_KDEUI) -lkdesu

METASOURCES = AUTO
noinst_HEADERS = passwords.h

messages:
	$(XGETTEXT) $(kcm_passwords_la_SOURCES) -o $(podir)/passwords.pot

data_DATA = passwords.desktop
datadir = $(kde_appsdir)/Settings/Personalization
