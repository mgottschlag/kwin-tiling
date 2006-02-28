kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfontinst_SRCS 
   CompressedFile.cpp 
   Main.cpp 
   Encodings.cpp 
   FontEngine.cpp 
   Fontmap.cpp 
   GetPid.c 
   XConfig.cpp )

kde4_automoc(${kfontinst_SRCS})

kde4_add_executable(kfontinst ${kfontinst_SRCS})

target_link_libraries(kfontinst  ${QT_AND_KDECORE_LIBS} kxftconfig kio kfontinst )

install_targets(/bin kfontinst )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#bin_PROGRAMS = kfontinst
#kfontinst_SOURCES = \
#CompressedFile.cpp \
#Main.cpp \
#Encodings.cpp \
#FontEngine.cpp \
#Fontmap.cpp \
#GetPid.c \
#XConfig.cpp
#
#noinst_HEADERS= \
#CompressedFile.h \
#Encodings.h \
#FontEngine.h \
#Fontmap.h \
#XConfig.h
#
#kfontinst_LDADD = ../../fonts/libkxftconfig.la $(LIBFONTCONFIG_LIBS) $(LIBFREETYPE_LIBS) $(LIB_FONT_ENC) $(LIBZ) $(LIB_KIO) ../lib/libkfontinst.la
#kfontinst_LDFLAGS = $(all_libraries) $(LIBFONTCONFIG_RPATH) $(LIBFREETYPE_RPATH) $(KDE_RPATH)
#AM_CPPFLAGS= -DOS_$(UNAME) -I$(srcdir)/../lib -I$(srcdir)/../../fonts $(all_includes) $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS)
