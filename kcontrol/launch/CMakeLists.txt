


########### next target ###############

set(kcm_launch_PART_SRCS kcmlaunch.cpp )

kde4_automoc(${kcm_launch_PART_SRCS})

kde4_add_plugin(kcm_launch ${kcm_launch_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_launch )

target_link_libraries(kcm_launch  ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kcm_launch  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmlaunch.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




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
