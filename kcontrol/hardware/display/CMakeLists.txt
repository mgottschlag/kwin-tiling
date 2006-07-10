


########### next target ###############

set(kcm_display_PART_SRCS display.cpp )

kde4_automoc(${kcm_display_PART_SRCS})

kde4_add_plugin(kcm_display ${kcm_display_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_display )

target_link_libraries(kcm_display  ${KDE4_KDECORE_LIBS} kutils ${QT_QTGUI_LIBRARY} kdeui)

install(TARGETS kcm_display  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES display.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_display.la
#
#kcm_display_la_SOURCES = display.cpp
#
#kcm_display_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_display_la_LIBADD = $(LIB_KUTILS)
#AM_CPPFLAGS = $(all_includes) 
#
#METASOURCES = AUTO
#
#noinst_HEADERS = display.h
#
#xdg_apps_DATA = display.desktop
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/display.pot
