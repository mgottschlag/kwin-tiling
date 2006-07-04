
project(kworkspacelib)



########### next target ###############

set(kworkspace_LIB_SRCS kworkspace.cpp )

kde4_automoc(${kworkspace_LIB_SRCS})

kde4_add_library(kworkspace SHARED ${kworkspace_LIB_SRCS})

target_link_libraries(kworkspace  ${KDE4_KIO_LIBS} ${X11_LIBRARIES} )

set_target_properties(kworkspace PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kworkspace  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install_files( /include/libkworkspace FILES kworkspace.h )




#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#lib_LTLIBRARIES = libkworkspace.la
#
#libkworkspace_la_SOURCES = kworkspace.cpp
#
#libkworkspace_la_LDFLAGS = $(all_libraries) -version-info 1:0:0
#libkworkspace_la_LIBADD  = $(LIB_KDECORE)
#    
#
#METASOURCES = AUTO
#
#kworkspaceincludedir = $(includedir)/libkworkspace
#kworkspaceinclude_HEADERS = kworkspace.h
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/libkworkspace.pot
#
#config.lo: configbase.h
