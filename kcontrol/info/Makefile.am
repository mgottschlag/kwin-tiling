SUBDIRS = pics

lib_LTLIBRARIES = libkcm_info.la

libkcm_info_la_SOURCES = main.cpp memory.cpp

libkcm_info_la_LDFLAGS = $(all_libraries) -module -avoid-version
# libkcm_info_la_LIBADD = $(LIBALIB)

INCLUDES= $(all_includes)
 
libkcm_info_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kcminfo.pot
