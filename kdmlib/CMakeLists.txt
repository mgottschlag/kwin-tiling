kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/workspace/lib ${CMAKE_SOURCE_DIR}/kdm/kfrontend ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kgreet_classic_PART_SRCS kgreet_classic.cpp )

kde4_automoc(${kgreet_classic_PART_SRCS})

kde4_add_plugin(kgreet_classic ${kgreet_classic_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kgreet_classic )

target_link_libraries(kgreet_classic  ${LIB_KDEUI})

install_targets(${LIB_INSTALL_DIR}/kde4 kgreet_classic )


########### next target ###############

set(kgreet_winbind_PART_SRCS kgreet_winbind.cpp )

kde4_automoc(${kgreet_winbind_PART_SRCS})

kde4_add_plugin(kgreet_winbind ${kgreet_winbind_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kgreet_winbind )

target_link_libraries(kgreet_winbind  ${LIB_KDEUI})

install_targets(${LIB_INSTALL_DIR}/kde4 kgreet_winbind )


########### next target ###############


########### install files ###############

install_files( /include FILES kgreeterplugin.h )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/workspace/lib -I$(top_srcdir)/kdm/kfrontend $(all_includes)
#
#kde_module_LTLIBRARIES = kgreet_classic.la kgreet_winbind.la
#
#kgreet_classic_la_SOURCES = kgreet_classic.cpp
#kgreet_classic_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kgreet_classic_la_LIBADD = $(LIB_KDEUI)
#
#kgreet_winbind_la_SOURCES = kgreet_winbind.cpp
#kgreet_winbind_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kgreet_winbind_la_LIBADD = $(LIB_KDEUI)
#
#noinst_LTLIBRARIES = libdmctl.la
#libdmctl_la_SOURCES = dmctl.cpp
#libdmctl_la_LDFLAGS = $(all_libraries) -no-undefined
#libdmctl_la_LIBADD = $(LIB_KDECORE) -lXau
#
#METASOURCES = AUTO
#
#noinst_HEADERS = dmctl.h kgreet_classic.h kgreet_winbind.h
#include_HEADERS = kgreeterplugin.h
#
#### TODO: why do we need 3 POT files for one directory?
#messages:
#	$(XGETTEXT) kgreet_classic.cpp -o $(podir)/kgreet_classic.pot
#	$(XGETTEXT) kgreet_winbind.cpp -o $(podir)/kgreet_winbind.pot
#	$(XGETTEXT) dmctl.cpp -o $(podir)/libdmctl.pot
