


########### next target ###############

set(kcm_taskbar_PART_SRCS kcmtaskbar.cpp )

kde4_automoc(${kcm_taskbar_PART_SRCS})

kde4_add_ui_files(kcm_taskbar_PART_SRCS kcmtaskbarui.ui )

kde4_add_plugin(kcm_taskbar ${kcm_taskbar_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_taskbar )

target_link_libraries(kcm_taskbar  ${KDE4_KDECORE_LIBS} taskbar kdeui )

install(TARGETS kcm_taskbar  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmtaskbar.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_taskbar.la
#
#kcm_taskbar_la_SOURCES = kcmtaskbarui.ui kcmtaskbar.cpp
#kcm_taskbar_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_taskbar_la_LIBADD = $(top_builddir)/kicker/taskbar/libtaskbar.la -lkdeui
#AM_CPPFLAGS = -I$(top_builddir)/kicker/taskbar $(all_includes) 
#
#METASOURCES = AUTO
#
#noinst_HEADERS = kcmtaskbar.h
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmtaskbar.pot
#
#xdg_apps_DATA = kcmtaskbar.desktop
#
#install-data-local: uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel/kcmtaskbar.desktop
#
#kcmtaskbar.lo: ../../kicker/taskbar/taskbarsettings.h
