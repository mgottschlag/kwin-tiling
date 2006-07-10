


########### next target ###############

set(kcm_nic_PART_SRCS nic.cpp )

kde4_automoc(${kcm_nic_PART_SRCS})

kde4_add_plugin(kcm_nic ${kcm_nic_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_nic )

target_link_libraries(kcm_nic  ${KDE4_KIO_LIBS} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTGUI_LIBRARY})

install(TARGETS kcm_nic  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES nic.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




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
