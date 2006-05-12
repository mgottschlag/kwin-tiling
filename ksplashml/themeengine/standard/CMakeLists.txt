
add_subdirectory( pics ) 
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_INCLUDES} )


########### next target ###############


########### next target ###############

set(themestandard_PART_SRCS themestandard.cpp wndicon.cpp wndstatus.cpp)

set(ksplashstandard_PART_SRCS themestandardlib.cpp ${themestandard_PART_SRCS})

kde4_automoc(${ksplashstandard_PART_SRCS})

kde4_add_plugin(ksplashstandard ${ksplashstandard_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} ksplashstandard )

target_link_libraries(ksplashstandard  ${KDE4_KDEUI_LIBS} ksplashthemes kdefx )

install_targets(${PLUGIN_INSTALL_DIR} ksplashstandard )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/ksplash/Themes/Standard FILES Theme.rc )
install_files( ${SERVICES_INSTALL_DIR} FILES ksplashstandard.desktop )




#original Makefile.am contents follow:

#SUBDIRS = pics
#
#INCLUDES =  -I$(top_srcdir)/ksplashml/themeengine $(all_includes)
#
#noinst_LTLIBRARIES = libthemestandard.la
#libthemestandard_la_SOURCES = themestandard.cpp wndicon.cpp wndstatus.cpp
#libthemestandard_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#libthemestandard_la_LIBADD = $(LIB_KDEUI) ../libksplashthemes.la
#
#kde_module_LTLIBRARIES = ksplashstandard.la
#
#ksplashstandard_la_SOURCES = themestandardlib.cpp
#ksplashstandard_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(KDE_PLUGIN) -module
#ksplashstandard_la_LIBADD = $(LIB_KDEUI) ../libksplashthemes.la libthemestandard.la
#METASOURCES = AUTO
#
#noinst_HEADERS = themestandard.h wndicon.h wndstatus.h
#
#servicesdir = $(kde_servicesdir)
#services_DATA = ksplashstandard.desktop
#
#themedir = $(kde_datadir)/ksplash/Themes/Standard
#theme_DATA = Theme.rc
