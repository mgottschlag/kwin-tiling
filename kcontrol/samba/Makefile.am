lib_LTLIBRARIES = libkcm_samba.la

libkcm_samba_la_SOURCES = main.cpp ksmbstatus.cpp kcmsambaimports.cpp kcmsambalog.cpp kcmsambastatistics.cpp

libkcm_samba_la_LDFLAGS  = $(all_libraries) -module -avoid-version 
libkcm_samba_la_LIBADD = -lkdeui

INCLUDES= $(all_includes)

SUBDIRS = 

libkcm_samba_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(libkcm_samba_la_SOURCES) -o $(podir)/kcmsamba.pot
