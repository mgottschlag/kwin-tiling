lib_LTLIBRARIES = libkcm_bell.la

libkcm_bell_la_SOURCES = bell.cpp

libkcm_bell_la_LIBADD   = -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET)
libkcm_bell_la_LDFLAGS  = $(all_libraries) -module -avoid-version 

INCLUDES= $(all_includes)

libkcm_bell_la_METASOURCES = AUTO

DISTCLEANFILES = $(lib_bell_METASOURCES)

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(libkcm_bell_la_SOURCES) -o ../po/libkcm_bell.pot
