lib_LTLIBRARIES = libkcm_input.la

libkcm_input_la_SOURCES = main.cpp keyboard.cpp mouse.cpp

libkcm_input_la_LDFLAGS  = $(all_libraries) -module -avoid-version 
libkcm_input_la_LIBADD = -lkdeui

INCLUDES= $(all_includes)

SUBDIRS = pics

libkcm_input_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) $(libkcm_input_la_SOURCES) -o $(podir)/kcminput.pot
