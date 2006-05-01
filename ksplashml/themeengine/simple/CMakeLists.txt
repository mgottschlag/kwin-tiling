kde4_header()
include_directories(  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} )


########### next target ###############

set(ksplashsimple_SRCS main.cpp )

kde4_automoc(${ksplashsimple_SRCS})

kde4_add_executable(ksplashsimple ${ksplashsimple_SRCS})

target_link_libraries(ksplashsimple  ${KDE4_KDECORE_LIBS} )

if (X11_Xinerama_FOUND)
   target_link_libraries(ksplashsimple  ${X11_Xinerama_LIB} )
endif (X11_Xinerama_FOUND)


install_targets(/bin ksplashsimple )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES = $(all_includes)
#
#bin_PROGRAMS = ksplashsimple
#ksplashsimple_SOURCES = main.cpp
#ksplashsimple_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#ksplashsimple_LDADD = $(LIB_XINERAMA) $(LIB_X11)
