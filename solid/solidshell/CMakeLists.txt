kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_SOURCE_DIR} )


########### next target ###############

set(solidshell_SRCS main.cpp )

kde4_automoc(${solidshell_SRCS})

kde4_add_executable(solidshell ${solidshell_SRCS})

target_link_libraries(solidshell ${KDE4_KDECORE_LIBS} kdehw )

install_targets(/bin solidshell )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/../../kdehw $(all_includes)
#
#bin_PROGRAMS = solidshell
#
#solidshell_SOURCES = main.cpp
#solidshell_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#solidshell_LDADD = ../../kdehw/libkdehw.la
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/solidshell.pot
#
#METASOURCES = AUTO
#
