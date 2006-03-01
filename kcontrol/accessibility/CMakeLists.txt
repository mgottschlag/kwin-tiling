kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_accessibility_PART_SRCS accessibility.cpp )

kde4_automoc(${kcm_accessibility_PART_SRCS})

kde4_add_ui3_files(kcm_accessibility_PART_SRCS accessibilityconfigwidget.ui )

kde4_add_plugin(kcm_accessibility ${kcm_accessibility_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_accessibility )

target_link_libraries(kcm_accessibility  ${QT_AND_KDECORE_LIBS} kio ${QT_QTGUI_LIBRARY} kdeui ${QT_QT3SUPPORT_LIBRARY})

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_accessibility )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES accessibility.desktop )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = $(all_includes)
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = kcm_accessibility.la
#
#kcm_accessibility_la_SOURCES = accessibility.cpp accessibilityconfigwidget.ui
#kcm_accessibility_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_accessibility_la_LIBADD = $(LIB_KIO)
#
#noinst_HEADERS = accessibility.h
#
#xdg_apps_DATA = accessibility.desktop
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmaccessibility.pot
