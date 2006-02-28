kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_launch_PART_SRCS kcmlaunch.cpp )

kde4_automoc(${kcm_launch_PART_SRCS})

kde4_add_plugin(kcm_launch ${kcm_launch_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_launch )

target_link_libraries(kcm_launch  ${QT_AND_KDECORE_LIBS} kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_launch )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kcmlaunch.desktop )

kde4_footer()



#original Makefile.am contents follow:

#
#kde_module_LTLIBRARIES        = kcm_launch.la
#AM_CPPFLAGS                      = $(all_includes) 
#kcm_launch_la_SOURCES      = kcmlaunch.cpp
#kcm_launch_la_LDFLAGS      = $(all_libraries) $(KDE_PLUGIN) -module
#kcm_launch_la_LIBADD       = $(LIB_KDEUI)
#METASOURCES  = AUTO
#noinst_HEADERS                = kcmlaunch.h
#xdg_apps_DATA = kcmlaunch.desktop
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmlaunch.pot
#
