kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_bell_PART_SRCS bell.cpp )

kde4_automoc(${kcm_bell_PART_SRCS})

kde4_add_plugin(kcm_bell ${kcm_bell_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_bell )

target_link_libraries(kcm_bell  ${QT_AND_KDECORE_LIBS} kdeui ${QT_QTGUI_LIBRARY})

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_bell )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES bell.desktop )

kde4_install_icons( hicolor crystalsvg )

kde4_footer()



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
