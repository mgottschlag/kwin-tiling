lib_LTLIBRARIES = libkcm_keys.la

libkcm_keys_la_SOURCES =  keyconfig.cpp savescm.cpp
libkcm_keys_la_LIBADD   = -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET)
libkcm_keys_la_LDFLAGS  = $(all_libraries) -module -avoid-version 

INCLUDES= $(all_includes)

libkcm_keys_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmkeys_SOURCES) ../../kwm/kwmbindings.cpp ./stdbindings.cpp -o ../../po/libkcm_keys.pot

$(srcdir)/keyconfig.cpp: stdbindings.cpp

