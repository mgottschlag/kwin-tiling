INCLUDES = $(all_includes)
kde_module_LTLIBRARIES = kcm_socks.la

kcm_socks_la_SOURCES = socks.cpp socksbase.ui 

kcm_socks_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_socks_la_LIBADD = -lkdeui $(LIB_KIO)

kcm_socks_la_METASOURCES = AUTO

noinst_HEADERS = socks.h

KDE_ICON = socks

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmsocks.pot

data_DATA = socks.desktop
datadir = $(kde_appsdir)/Settings/Network

update_DATA = socks.upd
updatedir = $(kde_datadir)/kconf_update