kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(kcm_nic_PART_SRCS nic.cpp )

kde4_automoc(${kcm_nic_PART_SRCS})

kde4_add_plugin(kcm_nic ${kcm_nic_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_nic )

target_link_libraries(kcm_nic  ${KDE4_KDECORE_LIBS} kdeui kio ${QT_QT3SUPPORT_LIBRARY} ${QT_QTGUI_LIBRARY})

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_nic )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES nic.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_nic.la
#
#kcm_nic_la_SOURCES = nic.cpp
#
#kcm_nic_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_nic_la_LIBADD = -lkdeui $(LIB_KIO) $(LIBSOCKET)
#
#AM_CPPFLAGS= $(all_includes)
#
#METASOURCES = AUTO
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kcmnic.pot
#
#xdg_apps_DATA = nic.desktop
