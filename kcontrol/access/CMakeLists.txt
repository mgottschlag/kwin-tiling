


########### next target ###############

set(kcm_access_PART_SRCS kcmaccess.cpp )

kde4_automoc(${kcm_access_PART_SRCS})

kde4_add_plugin(kcm_access ${kcm_access_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_access )

target_link_libraries(kcm_access  ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_PHONONCORE_LIBRARY} )

install(TARGETS kcm_access  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kaccess_KDEINIT_SRCS kaccess.cpp main.cpp )

kde4_automoc(${kaccess_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kaccess ${kaccess_KDEINIT_SRCS})

target_link_libraries(kdeinit_kaccess  ${KDE4_KDEUI_LIBS} ${KDE4_PHONONCORE_LIBRARY}  )

install(TARGETS kdeinit_kaccess  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kaccess kdeinit_kaccess )
install(TARGETS kaccess DESTINATION ${BIN_INSTALL_DIR})

########### install files ###############

install( FILES kcmaccess.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kaccess.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES eventsrc  DESTINATION  ${DATA_INSTALL_DIR}/kaccess/ )

kde4_install_icons( ${ICON_INSTALL_DIR}  )




#original Makefile.am contents follow:

#
#AM_CPPFLAGS= $(all_includes)
#METASOURCES = AUTO
#
#bin_PROGRAMS =
#lib_LTLIBRARIES =
#kde_module_LTLIBRARIES = kcm_access.la
#kdeinit_LTLIBRARIES = kaccess.la
#
#kaccess_la_SOURCES = kaccess.cpp main.cpp
#kaccess_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kaccess_la_LIBADD = $(LIB_KDEUI)
#
#kcm_access_la_SOURCES = kcmaccess.cpp 
#kcm_access_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_access_la_LIBADD = $(LIB_KIO)
#
#noinst_HEADERS = kcmaccess.h
#
#xdg_apps_DATA = kcmaccess.desktop
#
#services_DATA = kaccess.desktop
#servicesdir = $(kde_servicesdir)
#
#eventsrcdir = $(kde_datadir)/kaccess/
#eventsrc_DATA = eventsrc
#
#### TODO: why do we need 2 POT files for a single directory?
#messages:
#	$(XGETTEXT) kaccess.cpp main.cpp -o $(podir)/kaccess.pot
#	$(XGETTEXT) kcmaccess.cpp -o $(podir)/kcmaccess.pot
#
#KDE_ICON=AUTO
