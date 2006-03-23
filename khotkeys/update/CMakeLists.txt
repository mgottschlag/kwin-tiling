kde4_header()
include_directories( ${CMAKE_SOURCE_DIR}/khotkeys/shared/ ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

set(khotkeys_update_SRCS update.cpp ${libshared_SRCS})

kde4_automoc(${khotkeys_update_SRCS})

kde4_add_executable( khotkeys_update ${khotkeys_update_SRCS})

target_link_libraries( khotkeys_update ${KDE4_KIO_LIBS} ${X11_XTest_LIB})

install_targets(/lib/kconf_update_bin/ khotkeys_update )

kde4_footer()



#original Makefile.am contents follow:

#kconf_PROGRAMS = khotkeys_update
#kconfdir = $(libdir)/kconf_update_bin
#
#khotkeys_update_SOURCES = update.cpp
#
#khotkeys_update_LDADD   = ../shared/libshared.la $(LIB_KDECORE) $(KDE_RPATH)
#khotkeys_update_LDFLAGS = $(all_libraries)
#
#INCLUDES = -I$(srcdir)/../shared $(all_includes)
#
#METASOURCES = AUTO
