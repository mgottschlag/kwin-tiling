


########### next target ###############

set(kcm_ioslaveinfo_PART_SRCS kcmioslaveinfo.cpp )

kde4_automoc(${kcm_ioslaveinfo_PART_SRCS})

kde4_add_plugin(kcm_ioslaveinfo ${kcm_ioslaveinfo_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_ioslaveinfo )

target_link_libraries(kcm_ioslaveinfo  ${KDE4_KIO_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kcm_ioslaveinfo  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES ioslaveinfo.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_ioslaveinfo.la
#
#kcm_ioslaveinfo_la_SOURCES = kcmioslaveinfo.cpp
#
#kcm_ioslaveinfo_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_ioslaveinfo_la_LIBADD = -lkdeui $(LIB_KIO)
#
#AM_CPPFLAGS= $(all_includes)
#
#METASOURCES = AUTO
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kcmioslaveinfo.pot
#
#xdg_apps_DATA = ioslaveinfo.desktop
#
#install-data-local: uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Settings/Information
#
##	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Settings/Network/ioslaveinfo.desktop
#
