kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/workspace/kicker/libkicker ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kickermenu_konqueror_PART_SRCS konqy_menu.cpp )

kde4_automoc(${kickermenu_konqueror_PART_SRCS})

kde4_add_plugin(kickermenu_konqueror ${kickermenu_konqueror_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kickermenu_konqueror )

target_link_libraries(kickermenu_konqueror  ${QT_AND_KDECORE_LIBS} kdeui kio kickermain )

install_targets(${LIB_INSTALL_DIR}/kde4 kickermenu_konqueror )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/menuext FILES konquerormenu.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes) -I$(top_srcdir)/workspace/kicker/libkicker
#
#kde_module_LTLIBRARIES = kickermenu_konqueror.la
#
#kickermenu_konqueror_la_SOURCES = konqy_menu.cpp
#kickermenu_konqueror_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kickermenu_konqueror_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO) $(top_builddir)/workspace/kicker/libkicker/libkickermain.la
#
#kickermenu_konqueror_la_METASOURCES = AUTO
#
#desktopmenu_DATA = konquerormenu.desktop
#desktopmenudir = $(kde_datadir)/kicker/menuext
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_konqueror.pot
