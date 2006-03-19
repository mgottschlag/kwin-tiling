kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_taskbar_PART_SRCS kcmtaskbar.cpp )

kde4_automoc(${kcm_taskbar_PART_SRCS})

kde4_add_ui_files(kcm_taskbar_PART_SRCS kcmtaskbarui.ui )

kde4_add_plugin(kcm_taskbar ${kcm_taskbar_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_taskbar )

target_link_libraries(kcm_taskbar  ${QT_AND_KDECORE_LIBS} taskbar kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_taskbar )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kcmtaskbar.desktop )


install(FILES uninstall.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}${APPLNK_INSTALL_DIR}/Settings/LookNFeel/ RENAME kcmtaskbar.desktop)
kde4_footer()



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
