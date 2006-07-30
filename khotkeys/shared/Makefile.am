lib_LTLIBRARIES = libkhotkeys_shared.la

libkhotkeys_shared_la_SOURCES = action_data.cpp khotkeysglobal.cpp triggers.cpp windows.cpp \
    settings.cpp actions.cpp input.cpp conditions.cpp khlistview.cpp khlistbox.cpp \
    gestures.cpp voices.cpp voicesignature.cpp sound.cpp soundrecorder.cpp
libkhotkeys_shared_la_LIBADD  = $(LIB_KIO) $(XTESTLIB) -lXext -lX11 $(X_PRE_LIBS)
libkhotkeys_shared_la_LDFLAGS = $(all_libraries) $(X_LDFLAGS) -version-info 1:0:0 -no-undefined

INCLUDES = $(all_includes)

noinst_HEADERS = action_data.h khotkeysglobal.h triggers.h windows.h actions.h input.h \
    settings.h conditions.h khlistview.h khlistbox.h gestures.h voices.h \
    voicesignature.h sound.h soundrecorder.h

METASOURCES = AUTO
