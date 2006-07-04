
add_subdirectory( pics ) 
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_INCLUDES} )


########### next target ###############

set(ksplashredmond_PART_SRCS previewredmond.cpp themeredmond.cpp ${ksplashthemes_LIB_SRCS})

kde4_automoc(${ksplashredmond_PART_SRCS})

kde4_add_plugin(ksplashredmond ${ksplashredmond_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} ksplashredmond )

target_link_libraries(ksplashredmond  ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kdefx ksplashthemes )

install(TARGETS ksplashredmond  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES Theme.rc  DESTINATION  ${DATA_INSTALL_DIR}/ksplash/Themes/Redmond )
install( FILES ksplashredmond.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




#original Makefile.am contents follow:

#SUBDIRS = pics
#
#INCLUDES =  -I$(top_srcdir)/ksplashml/themeengine $(all_includes)
#
#ksplashredmond_la_SOURCES = previewredmond.cpp themeredmond.cpp
#ksplashredmond_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module $(KDE_PLUGIN)
#ksplashredmond_la_LIBADD  = $(LIB_KDEUI) $(LIB_KIO) ../libksplashthemes.la
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = ksplashredmond.la
#
#noinst_HEADERS 	= themeredmond.h previewredmond.h
#
#servicesdir = $(kde_servicesdir)
#services_DATA = ksplashredmond.desktop
#
#themedir = $(kde_datadir)/ksplash/Themes/Redmond
#theme_DATA = Theme.rc
