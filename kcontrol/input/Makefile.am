lib_LTLIBRARIES = libkcm_input.la

libkcm_input_la_SOURCES = main.cpp keyboard.cpp mouse.cpp

libkcm_input_la_LDFLAGS  = $(all_libraries) -module -avoid-version 
libkcm_input_la_LIBADD = -lkdeui

INCLUDES= $(all_includes)

libkcm_input_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(libkcm_imput_SOURCES) -o ../po/kcminput.pot
