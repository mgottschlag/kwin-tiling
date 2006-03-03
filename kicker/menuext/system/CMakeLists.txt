kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kicker/libkicker ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kickermenu_systemmenu_PART_SRCS systemmenu.cpp )

kde4_automoc(${kickermenu_systemmenu_PART_SRCS})

kde4_add_plugin(kickermenu_systemmenu ${kickermenu_systemmenu_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kickermenu_systemmenu )

target_link_libraries(kickermenu_systemmenu  ${QT_AND_KDECORE_LIBS} kdeui ${QT_QTGUI_LIBRARY} kio)

install_targets(${LIB_INSTALL_DIR}/kde4 kickermenu_systemmenu )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/menuext FILES systemmenu.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes) -I$(top_srcdir)/kicker/libkicker
#
#kde_module_LTLIBRARIES = kickermenu_systemmenu.la
#
#kickermenu_systemmenu_la_SOURCES = systemmenu.cpp
#kickermenu_systemmenu_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kickermenu_systemmenu_la_LIBADD = $(LIB_KDEUI) 
#
#kickermenu_systemmenu_la_METASOURCES = AUTO
#
#desktopmenu_DATA = systemmenu.desktop
#desktopmenudir = $(kde_datadir)/kicker/menuext
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_systemmenu.pot
