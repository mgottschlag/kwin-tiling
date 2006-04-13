kde4_header()
include_directories(
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/lib/ 
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/fonts/
	${KDE4_INCLUDES} )


########### next target ###############

set(kfontinst_bin_SRCS ${libkxftconfig_SRCS}
   CompressedFile.cpp 
   Main.cpp 
   Encodings.cpp 
   FontEngine.cpp 
   Fontmap.cpp 
   GetPid.c 
   XConfig.cpp )

kde4_automoc(${kfontinst_bin_SRCS})

kde4_add_executable(kfontinst_bin ${kfontinst_bin_SRCS})
set_target_properties(kfontinst_bin PROPERTIES OUTPUT_NAME kfontinst)

target_link_libraries(kfontinst_bin  ${KDE4_KIO_LIBS} kfontinst fontenc )

install_targets(/bin kfontinst_bin )


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
