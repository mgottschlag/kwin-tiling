lib_LTLIBRARIES = libkcm_syssound.la

libkcm_syssound_la_SOURCES = main.cpp syssound.cpp

libkcm_syssound_la_LDFLAGS = $(all_libraries) -module -avoid-version 
libkcm_syssound_la_LIBADD = -lkdeui -lmediatool

INCLUDES= $(all_includes)
 
libkcm_syssound_la_METASOURCES = AUTO

KDE_ICON = ksyssound

DISTCLEANFILES = $(lib_syssound_METASOURCES)

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(libkcm_syssound_la_SOURCES) -o $(podir)/kcmsyssound.pot
