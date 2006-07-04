include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_INCLUDES} )


########### next target ###############


########### next target ###############


set(ksplashdefault_PART_SRCS main.cpp ${themedefault_PART_SRCS})


kde4_automoc(${ksplashdefault_PART_SRCS})

kde4_add_plugin(ksplashdefault ${ksplashdefault_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} ksplashdefault )

target_link_libraries(ksplashdefault  ${KDE4_KDEUI_LIBS} ksplashthemes )

install(TARGETS ksplashdefault  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES ksplashdefault.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




#original Makefile.am contents follow:

#
#INCLUDES =  -I$(srcdir)/.. -I$(srcdir)/../.. $(all_includes)
#
#noinst_LTLIBRARIES = libthemedefault.la
#libthemedefault_la_SOURCES = themelegacy.cpp
#libthemedefault_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#libthemedefault_la_LIBADD = $(LIB_KDEUI) ../libksplashthemes.la
#
#kde_module_LTLIBRARIES = ksplashdefault.la
#
#ksplashdefault_la_SOURCES = main.cpp
#ksplashdefault_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(KDE_PLUGIN) -module
#ksplashdefault_la_LIBADD = $(LIB_KDEUI) ../libksplashthemes.la libthemedefault.la
#METASOURCES = AUTO
#
#servicesdir = $(kde_servicesdir)
#services_DATA = ksplashdefault.desktop
