
CPPFLAGS = -DSRCDIR="\"$(shell (cd $(srcdir) && pwd))\""

lib_LTLIBRARIES = libkscreensaver.la
libkscreensaver_la_SOURCES = main.cpp kscreensaver.cpp
libkscreensaver_la_LIBADD  = $(LIB_KSYCOCA)
libkscreensaver_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:1

include_HEADERS = kscreensaver.h

INCLUDES = $(all_includes)
METASOURCES = AUTO


messages:
#	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/libkss.pot 

