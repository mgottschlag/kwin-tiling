kde4_header()
include_directories( ${CMAKE_SOURCE_DIR}/workspace/plasma/lib  ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(pdesktop_SRCS desktop.cpp main.cpp )

kde4_automoc(${pdesktop_SRCS})

kde4_add_executable(pdesktop ${pdesktop_SRCS})

target_link_libraries(pdesktop  ${KDE4_KDECORE_LIBS} plasma kio )

install_targets(/bin pdesktop )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

### Makefile.am of Plasma Desktop
#
#INCLUDES = -I$(top_srcdir)/workspace/plasma/lib $(all_includes)
#
#bin_PROGRAMS = pdesktop
#pdesktop_LDFLAGS  = $(all_libraries) $(KDE_RPATH)
#pdesktop_LDADD    = ../lib/libplasma.la $(LIB_KIO) $(LIB_QTOPENGL)
#
######## Files
#
#pdesktop_SOURCES = desktop.cpp main.cpp
#
#METASOURCES = AUTO
