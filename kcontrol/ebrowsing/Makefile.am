
SUBDIRS = kurifilter plugins

INCLUDES = -I$(srcdir)/kurifilter -I$(srcdir)/plugins/ikws $(all_includes) 
LDFLAGS =  $(all_libraries)

METASOURCES = AUTO

lib_LTLIBRARIES = libkcm_kurifilt.la

libkcm_kurifilt_la_SOURCES = filteropts.cpp main.cpp

libkcm_kurifilt_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_kurifilt_la_LIBADD = kurifilter/libkurifilter.la plugins/ikws/libkuriikwsfilter.la \
                            plugins/shorturi/libkshorturifilter.la -lkdecore

messages:
	$(XGETTEXT) $(libkcm_kurifilt_la_SOURCES) -o $(podir)/kcmkurifilt.pot

