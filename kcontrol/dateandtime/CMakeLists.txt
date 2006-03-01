kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_clock_PART_SRCS dtime.cpp tzone.cpp main.cpp )

kde4_automoc(${kcm_clock_PART_SRCS})

kde4_add_plugin(kcm_clock ${kcm_clock_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_clock )

target_link_libraries(kcm_clock  ${QT_AND_KDECORE_LIBS} kio kdeui ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} DCOP)

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_clock )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES clock.desktop )

kde4_install_icons( hicolor crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_clock.la
#
#kcm_clock_la_SOURCES = dtime.cpp tzone.cpp main.cpp 
#
#kcm_clock_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kcm_clock_la_LIBADD = $(LIB_KIO)
#
#AM_CPPFLAGS = $(all_includes)
# 
#METASOURCES = AUTO
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kcmkclock.pot
#
#xdg_apps_DATA = clock.desktop
#
#KDE_ICON=AUTO
