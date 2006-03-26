kde4_header()

project(ksystraycmd)
include_directories(  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} )


########### next target ###############

set(ksystraycmd_SRCS ksystraycmd.cpp main.cpp )

kde4_automoc(${ksystraycmd_SRCS})

kde4_add_executable(ksystraycmd ${ksystraycmd_SRCS})

target_link_libraries(ksystraycmd  ${KDE4_KDEUI_LIBS} )

install_targets(/bin ksystraycmd )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

######## Fiddle here
#
#INCLUDES = $(all_includes)
#LDADD    = $(LIB_KDEUI)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
########	Files
#
#bin_PROGRAMS	= ksystraycmd
#METASOURCES	= ksystraycmd.moc
#noinst_HEADERS  = ksystraycmd.h
#
#ksystraycmd_SOURCES = ksystraycmd.cpp main.cpp
#ksystraycmd_LDFLAGS  = $(all_libraries) $(KDE_RPATH)
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/ksystraycmd.pot
#
#
