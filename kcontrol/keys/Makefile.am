lib_LTLIBRARIES = libkcm_keys.la

libkcm_keys_la_SOURCES =  keyconfig.cpp savescm.cpp main.cpp
libkcm_keys_la_LIBADD   = -lkdeui
libkcm_keys_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined

INCLUDES= $(all_includes)

libkcm_keys_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) $(libkcm_keys_la_SOURCES) -o $(podir)/kcmkeys.pot

data_DATA = keys.desktop
datadir = $(kde_appsdir)/Settings/LookNFeel
