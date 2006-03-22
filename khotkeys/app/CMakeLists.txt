kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  ${CMAKE_SOURCE_DIR}/khotkeys/shared/ )


########### next target ###############

set(khotkeys_KDEINIT_SRCS app.cpp ${libshared_SRCS})

kde4_automoc(${khotkeys_KDEINIT_SRCS})

kde4_add_dcop_skels(khotkeys_KDEINIT_SRCS app.h )

kde4_add_kdeinit_executable( khotkeys ${khotkeys_KDEINIT_SRCS})

target_link_libraries(kdeinit_khotkeys  ${KDE4_KDE3SUPPORT_LIBS} ${X11_XTest_LIB} )

install_targets(${LIB_INSTALL_DIR} kdeinit_khotkeys )

target_link_libraries( khotkeys kdeinit_khotkeys )
install_targets(/bin khotkeys )

########### next target ###############

set(kded_khotkeys_PART_SRCS kded.cpp ${libshared_SRCS})

kde4_automoc(${kded_khotkeys_PART_SRCS})

kde4_add_dcop_skels(kded_khotkeys_PART_SRCS kded.h )

kde4_add_plugin(kded_khotkeys ${kded_khotkeys_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kded_khotkeys )

target_link_libraries(kded_khotkeys  ${KDE4_KDE3SUPPORT_LIBS} ${X11_XTest_LIB} )

install_targets(${LIB_INSTALL_DIR}/kde4 kded_khotkeys )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kded FILES khotkeys.desktop )

kde4_footer()



#original Makefile.am contents follow:

#bin_PROGRAMS =
#lib_LTLIBRARIES =
#kdeinit_LTLIBRARIES = khotkeys.la
#kde_module_LTLIBRARIES = kded_khotkeys.la
#
#khotkeys_la_SOURCES = app.cpp app.skel
#khotkeys_la_LIBADD   = ../shared/libshared.la $(LIB_KDECORE)
#khotkeys_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#
#kded_khotkeys_la_SOURCES = kded.cpp kded.skel
#kded_khotkeys_la_LIBADD = ../shared/libshared.la $(LIB_KDECORE)
#kded_khotkeys_la_LDFLAGS = $(all_libraries) -module -avoid-version
#
#noinst_HEADERS = app.h kded.h
#
#INCLUDES = -I$(srcdir)/../shared $(all_includes)
#
#METASOURCES = AUTO
#
#servicesdir = $(kde_servicesdir)/kded
#services_DATA = khotkeys.desktop
