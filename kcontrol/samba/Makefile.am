kde_module_LTLIBRARIES = kcm_samba.la

kcm_samba_la_SOURCES = main.cpp ksmbstatus.cpp kcmsambaimports.cpp kcmsambalog.cpp kcmsambastatistics.cpp

kcm_samba_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_samba_la_LIBADD = $(LIB_KFILE)

INCLUDES= $(all_includes)

kcm_samba_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) $(kcm_samba_la_SOURCES) -o $(podir)/kcmsamba.pot

data_DATA = smbstatus.desktop
datadir = $(kde_appsdir)/Settings/Information

