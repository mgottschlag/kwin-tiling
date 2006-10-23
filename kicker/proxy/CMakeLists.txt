



########### next target ###############

set(extensiondebugger_SRCS extensiondebugger.cpp )

kde4_automoc(${extensiondebugger_SRCS})

kde4_add_executable(extensiondebugger ${extensiondebugger_SRCS})

target_link_libraries(extensiondebugger  ${KDE4_KDECORE_LIBS} kickermain )


########### next target ###############

set(appletproxy_KDEINIT_SRCS appletproxy.cpp )

kde4_automoc(${appletproxy_KDEINIT_SRCS})

kde4_add_kdeinit_executable( appletproxy ${appletproxy_KDEINIT_SRCS})

target_link_libraries(kdeinit_appletproxy  ${KDE4_KDEUI_LIBS} kickermain )

install(TARGETS kdeinit_appletproxy  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( appletproxy kdeinit_appletproxy )
install(TARGETS appletproxy DESTINATION ${BIN_INSTALL_DIR})

########### install files ###############





#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/../core -I$(srcdir)/../libkicker -I../libkicker  $(all_includes)
#
#bin_PROGRAMS = 
#noinst_PROGRAMS = extensiondebugger
#lib_LTLIBRARIES =
#kdeinit_LTLIBRARIES = appletproxy.la
#
#METASOURCES = AUTO
#
#appletproxy_la_LIBADD = $(LIB_KDEUI) ../libkicker/libkickermain.la
#appletproxy_la_SOURCES = appletproxy.cpp
#appletproxy_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module -avoid-version
#
#
#extensiondebugger_SOURCES = extensiondebugger.cpp
#extensiondebugger_LDADD = ../libkicker/libkickermain.la
#extensiondebugger_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#
#messages:
#	$(XGETTEXT) appletproxy.cpp -o $(podir)/appletproxy.pot
#	$(XGETTEXT) extensiondebugger.cpp -o $(podir)/extensionproxy.pot
