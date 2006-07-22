
project(ksplash)

set(themedefault_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/themeengine/default/themelegacy.cpp )


add_subdirectory( themeengine ) 
add_subdirectory( pics ) 
add_subdirectory( kcmksplash ) 
include_directories(  ${CMAKE_CURRENT_SOURCE_DIR}/themeengine ${CMAKE_CURRENT_SOURCE_DIR}/themeengine/default/ ${KDE4_INCLUDES} )


########### next target ###############

set(ksplash_SRCS wndmain.cpp main.cpp ${themedefault_PART_SRCS} )

qt4_add_dbus_adaptor( ksplash_SRCS org.kde.KSplash.xml wndmain.h KSplash )

kde4_automoc(${ksplash_SRCS})

kde4_add_executable(ksplash ${ksplash_SRCS})

target_link_libraries(ksplash  ${KDE4_KDEUI_LIBS} ksplashthemes ${KDE4_KIO_LIBS} )

install(TARGETS ksplash DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES ksplash.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




#original Makefile.am contents follow:

#SUBDIRS = themeengine pics kcmksplash
#
#INCLUDES = -I$(srcdir)/themeengine -I$(srcdir)/themeengine/default $(all_includes)
#
#METASOURCES=AUTO
#
#bin_PROGRAMS = ksplash
#ksplash_SOURCES = wndmain.cpp ksplashiface.skel main.cpp
#ksplash_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#ksplash_LDADD = $(LIB_KDEUI) themeengine/default/libthemedefault.la themeengine/libksplashthemes.la $(LIB_KIO)
#
#noinst_HEADERS = ksplashiface.h wndmain.h
#
#servicesdir = $(kde_servicesdir)
#services_DATA = ksplash.desktop
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp themeengine/*.cpp -o $(podir)/ksplash.pot
