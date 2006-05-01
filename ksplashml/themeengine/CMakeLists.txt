kde4_header()

add_subdirectory( default ) 
add_subdirectory( standard ) 
add_subdirectory( redmond ) 
add_subdirectory( simple ) 
include_directories( ${KDE4_INCLUDES} )


########### next target ###############

set(ksplashthemes_LIB_SRCS themeengine.cpp objkstheme.cpp )

kde4_automoc(${ksplashthemes_LIB_SRCS})

kde4_add_library(ksplashthemes SHARED ${ksplashthemes_LIB_SRCS} ) 

target_link_libraries(ksplashthemes  ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${X11_X11_LIB})
if(X11_Xcursor_FOUND)
  target_link_libraries(ksplashthemes ${X11_Xcursor_LIB})
endif(X11_Xcursor_FOUND)

set_target_properties(ksplashthemes PROPERTIES VERSION 0.0.0 SOVERSION 0 )
install_targets(${LIB_INSTALL_DIR} ksplashthemes )


########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES ksplashplugins.desktop )
install_files( /include/ksplash FILES themeengine.h objkstheme.h )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = . default standard redmond simple
#
#INCLUDES = -I$(srcdir)/.. $(all_includes)
#
#lib_LTLIBRARIES = libksplashthemes.la
#
#libksplashthemes_la_SOURCES = themeengine.cpp objkstheme.cpp
#libksplashthemes_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#libksplashthemes_la_LIBADD = $(LIB_KDEUI) $(LIB_XCURSOR)
#METASOURCES = AUTO
#
## install this header for theme developers.
#pkgincludedir = $(includedir)/ksplash
#pkginclude_HEADERS=themeengine.h objkstheme.h
#
#servicetypesdir = $(kde_servicetypesdir)
#servicetypes_DATA = ksplashplugins.desktop
#
