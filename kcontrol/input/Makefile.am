lib_LTLIBRARIES = libkcm_input.la

libkcm_input_la_SOURCES = main.cpp keyboard.cpp mouse.cpp

libkcm_input_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_input_la_LIBADD = -lkdeui

INCLUDES= $(all_includes)

libkcm_input_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) $(libkcm_input_la_SOURCES) -o $(podir)/kcminput.pot

data_DATA = keyboard.desktop mouse.desktop
datadir = $(kde_appsdir)/Settings/Peripherals
