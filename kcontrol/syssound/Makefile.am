lib_LTLIBRARIES = libkcm_syssound.la

libkcm_syssound_la_SOURCES = main.cpp syssound.cpp

libkcm_syssound_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libkcm_syssound_la_LIBADD = -lkdeui

INCLUDES= $(all_includes)
 
libkcm_syssound_la_METASOURCES = AUTO

KDE_ICON = ksyssound bell

DISTCLEANFILES = $(lib_syssound_METASOURCES)

messages:
	$(XGETTEXT) $(libkcm_syssound_la_SOURCES) -o $(podir)/kcmsyssound.pot

data_DATA = syssound.desktop
datadir = $(kde_appsdir)/Settings/Sound
