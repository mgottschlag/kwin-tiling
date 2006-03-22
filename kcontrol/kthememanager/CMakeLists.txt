kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_kthememanager_PART_SRCS knewthemedlg.cpp ktheme.cpp kthememanager.cpp )

kde4_automoc(${kcm_kthememanager_PART_SRCS})

kde4_add_ui3_files(kcm_kthememanager_PART_SRCS kthemedlg.ui newthemewidget.ui )

kde4_add_plugin(kcm_kthememanager ${kcm_kthememanager_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kthememanager )

target_link_libraries(kcm_kthememanager  ${KDE4_KDECORE_LIBS} kio kde3support  ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} kdeui ${QT_QTXML_LIBRARY} DCOP)

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kthememanager )


########### install files ###############

install_files( ${MIME_INSTALL_DIR}/application FILES x-ktheme.desktop )
install_files( ${XDG_APPS_DIR} FILES kthememanager.desktop installktheme.desktop )
install_files( /bin FILES kdeinstallktheme )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS    = $(all_includes)
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = kcm_kthememanager.la
#
#kcm_kthememanager_la_SOURCES = kthemedlg.ui newthemewidget.ui knewthemedlg.cpp ktheme.cpp kthememanager.cpp
#kcm_kthememanager_la_LIBADD = $(LIB_KIO) $(LIB_KDE3SUPPORT)
#kcm_kthememanager_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
#
#noinst_HEADERS = kthememanager.h ktheme.h knewthemedlg.h kthemedlg.ui.h
#
#xdg_apps_DATA = kthememanager.desktop installktheme.desktop
#
## MIME type
#kthememime_DATA = x-ktheme.desktop
#kthememimedir = $(kde_mimedir)/application
#
#bin_SCRIPTS = kdeinstallktheme
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kthememanager.pot
#KDE_ICON=AUTO
