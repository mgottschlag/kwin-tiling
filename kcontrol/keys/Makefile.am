lib_LTLIBRARIES = libkcm_keys.la

libkcm_keys_la_SOURCES =  keyconfig.cpp savescm.cpp
libkcm_keys_la_LIBADD   = -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET)
libkcm_keys_la_LDFLAGS  = $(all_libraries) -module -avoid-version 

INCLUDES= $(all_includes)

libkcm_keys_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) $(libkcm_keys_la_SOURCES) -o $(podir)/kcmkeys.pot

