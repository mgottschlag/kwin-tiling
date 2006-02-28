kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfontinst_LIB_SRCS Misc.cpp FcEngine.cpp )

kde4_automoc(${kfontinst_LIB_SRCS})

kde4_add_library(kfontinst SHARED ${kfontinst_LIB_SRCS})

target_link_libraries(kfontinst  ${QT_AND_KDECORE_LIBS} kio )

set_target_properties(kfontinst PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kfontinst )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#lib_LTLIBRARIES = libkfontinst.la
#
#libkfontinst_la_SOURCES = \
#Misc.cpp \
#FcEngine.cpp
#
#noinst_HEADERS = \
#Misc.h \
#FcEngine.h \
#KfiConstants.h
#
#libkfontinst_la_LIBADD = $(LIB_KDECORE) $(LIBFONTCONFIG_LIBS) $(LIBFREETYPE_LIBS) $(LIB_KIO) $(LIBXFT_LIB)
#libkfontinst_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIBFONTCONFIG_RPATH) $(LIBFREETYPE_RPATH) -no-undefined
#AM_CPPFLAGS= $(all_includes) $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS) -D_LARGEFILE64_SOURCE
