


########### next target ###############

########### install files ###############





#original Makefile.am contents follow:

#noinst_LTLIBRARIES = libshared.la
#
#libshared_la_SOURCES = action_data.cpp khotkeysglobal.cpp triggers.cpp windows.cpp \
#    settings.cpp actions.cpp input.cpp conditions.cpp khlistview.cpp khlistbox.cpp \
#    gestures.cpp
#libshared_la_LIBADD  = $(LIB_KIO) $(XTESTLIB) -lXext -lX11 $(X_PRE_LIBS)
#libshared_la_LDFLAGS = $(all_libraries) $(X_LDFLAGS) -no-undefined
#
#INCLUDES = $(all_includes)
#
#noinst_HEADERS = action_data.h khotkeysglobal.h triggers.h windows.h actions.h input.h \
#    settings.h conditions.h khlistview.h khlistbox.h gestures.h
#
#libshared_la_METASOURCES = AUTO
