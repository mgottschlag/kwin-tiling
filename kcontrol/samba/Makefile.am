lib_LTLIBRARIES = libkcm_samba.la

libkcm_samba_la_SOURCES = main.cpp ksmbstatus.cpp kcmsambaimports.cpp kcmsambalog.cpp kcmsambastatistics.cpp

libkcm_samba_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_samba_la_LIBADD = -lkdeui

INCLUDES= $(all_includes)

SUBDIRS = 

libkcm_samba_la_METASOURCES = AUTO

KDE_ICON = samba

messages:
	$(XGETTEXT) $(libkcm_samba_la_SOURCES) -o $(podir)/kcmsamba.pot
