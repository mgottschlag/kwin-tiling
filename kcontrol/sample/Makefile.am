lib_LTLIBRARIES = libkcm_sample.la

libkcm_sample_la_SOURCES = sample.cpp 

libkcm_sample_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_sample_la_LIBADD = -lkdeui  
INCLUDES= $(all_includes) 

libkcm_sample_la_METASOURCES = AUTO

noinst_HEADERS = sample.h

KDE_ICON = sample

messages:
	$(XGETTEXT) $(libkcm_sample_la_SOURCES) -o $(podir)/kcmsample.pot

