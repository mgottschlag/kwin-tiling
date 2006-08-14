


########### next target ###############

set(kcm_bell_PART_SRCS bell.cpp )

kde4_automoc(${kcm_bell_PART_SRCS})

kde4_add_plugin(kcm_bell ${kcm_bell_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_bell )

target_link_libraries(kcm_bell  ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${X11_X11_LIB})

install(TARGETS kcm_bell  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES bell.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )




#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_bell.la
#
#kcm_bell_la_SOURCES = bell.cpp
#
#kcm_bell_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kcm_bell_la_LIBADD = -lkdeui
#
#AM_CPPFLAGS= $(all_includes)
#
#METASOURCES = bell.moc
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kcmbell.pot
#
#xdg_apps_DATA = bell.desktop
#
#KDE_ICON=AUTO
