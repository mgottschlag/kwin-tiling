kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_knotify_PART_SRCS knotify.cpp )

kde4_automoc(${kcm_knotify_PART_SRCS})

kde4_add_ui_files(kcm_knotify_PART_SRCS playersettings.ui )

kde4_add_plugin(kcm_knotify ${kcm_knotify_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_knotify )

target_link_libraries(kcm_knotify  ${KDE4_KDECORE_LIBS} kio knotifyconfig ${QT_QTGUI_LIBRARY} kdeui)

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_knotify )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kcmnotify.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#
#kde_module_LTLIBRARIES = kcm_knotify.la
#
#kcm_knotify_la_SOURCES = knotify.cpp playersettings.ui
#
#kcm_knotify_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kcm_knotify_la_LIBADD = $(LIB_KIO)  -lknotifyconfig
#
#AM_CPPFLAGS = $(all_includes)
# 
#METASOURCES = AUTO 
#
#noinst_HEADERS = knotify.h
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmnotify.pot
#
#xdg_apps_DATA = kcmnotify.desktop
#
#
## the following should be gone at release time.
#install-data-local:
#	rm -f $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel/knotify.desktop
#	rm -f $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel/kcmnotify.desktop
#
#KDE_ICON=AUTO
#
