lib_LTLIBRARIES = libkcm_info.la

libkcm_info_la_SOURCES = main.cpp memory.cpp

libkcm_info_la_LDFLAGS  = -module -avoid-version 

INCLUDES= $(all_includes)
 
libkcm_info_la_METASOURCES = AUTO

DISTCLEANFILES = $(lib_bell_METASOURCES)

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(libkcm_info_la_SOURCES) -o ../../po/libkcm_info.pot
