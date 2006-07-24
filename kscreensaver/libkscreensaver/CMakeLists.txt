
project(libkscreensaver)



########### next target ###############

set(kscreensaver_LIB_SRCS main.cpp kscreensaver.cpp)

kde4_automoc(${kscreensaver_LIB_SRCS})

kde4_add_library(kscreensaver SHARED ${kscreensaver_LIB_SRCS})

target_link_libraries(kscreensaver  ${KDE4_KIO_LIBS} ${X11_X11_LIB})

set_target_properties(kscreensaver PROPERTIES VERSION 4.2.0 SOVERSION 4)
install(TARGETS kscreensaver  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES kscreensaver.h kscreensaver_vroot.h DESTINATION ${INCLUDE_INSTALL_DIR} )




#original Makefile.am contents follow:

#### TODO KDE4: define a macro or template for the main() code to avoid undefined symbols
#
#lib_LTLIBRARIES = libkscreensaver.la
#libkscreensaver_la_SOURCES = main.cpp kscreensaver.cpp
#libkscreensaver_la_LIBADD  = $(LIB_KIO)
#if undefined_symbols_allowed
#libkscreensaver_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 6:0:2
#else
#libkscreensaver_la_LDFLAGS = $(all_libraries) -static
#endif 
#
#include_HEADERS = kscreensaver.h kscreensaver_vroot.h
#
#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/libkscreensaver.pot 
#
#include $(top_srcdir)/admin/Doxyfile.am
