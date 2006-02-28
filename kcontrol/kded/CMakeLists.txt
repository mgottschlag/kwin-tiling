kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_kded_PART_SRCS kcmkded.cpp )

kde4_automoc(${kcm_kded_PART_SRCS})

kde4_add_plugin(kcm_kded ${kcm_kded_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kded )

target_link_libraries(kcm_kded  ${QT_AND_KDECORE_LIBS} kio ${QT_QT3SUPPORT_LIBRARY} kdeui ${QT_QTGUI_LIBRARY} DCOP)

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kded )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kcmkded.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_kded.la
#
#kcm_kded_la_SOURCES = kcmkded.cpp
#
#kcm_kded_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kcm_kded_la_LIBADD =  $(LIB_KIO)
#
#AM_CPPFLAGS= $(all_includes)
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmkded.pot
#
#xdg_apps_DATA = kcmkded.desktop
