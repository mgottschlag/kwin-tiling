kde4_header()

add_subdirectory( pics ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_ksplashthemes_PART_SRCS installer.cpp main.cpp )

kde4_automoc(${kcm_ksplashthemes_PART_SRCS})

kde4_add_plugin(kcm_ksplashthemes ${kcm_ksplashthemes_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_ksplashthemes )

target_link_libraries(kcm_ksplashthemes  ${LIB_KDE3SUPPORT} )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_ksplashthemes )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES ksplashthememgr.desktop )

kde4_footer()



#original Makefile.am contents follow:

## Makefile.am for kdebase/ksplashml/kcmksplash
#SUBDIRS=pics
#
#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = kcm_ksplashthemes.la
#
#kcm_ksplashthemes_la_SOURCES = installer.cpp main.cpp
#noinst_HEADERS = installer.h
#
#kcm_ksplashthemes_la_LIBADD = $(LIB_KIO)  $(LIB_KDE3SUPPORT)
#kcm_ksplashthemes_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
#
#xdg_apps_DATA = ksplashthememgr.desktop
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/ksplashthemes.pot
