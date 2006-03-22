kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_keys_PART_SRCS 
   shortcuts.cpp 
   modifiers.cpp 
   main.cpp 
   khotkeys.cpp 
   treeview.cpp )

kde4_automoc(${kcm_keys_PART_SRCS})

kde4_add_plugin(kcm_keys ${kcm_keys_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_keys )

target_link_libraries(kcm_keys  ${KDE4_KDECORE_LIBS} kdeui kio ${QT_QT3SUPPORT_LIBRARY} ${QT_QTGUI_LIBRARY} ${X11_X11_LIB})

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_keys )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES keys.desktop )
install_files( ${DATA_INSTALL_DIR}/kcmkeys FILES wm3.kksrc kde3.kksrc kde4.kksrc win3.kksrc win4.kksrc mac4.kksrc unix3.kksrc )
install_files( ${KCONF_UPDATE_INSTALL_DIR} FILES kaccel.upd )
install_files( ${KCONF_UPDATE_INSTALL_DIR} FILES convertShortcuts.pl )
kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_keys.la
#
#kcm_keys_la_SOURCES =  shortcuts.cpp modifiers.cpp main.cpp khotkeys.cpp treeview.cpp
#kcm_keys_la_LIBADD   = -lkdeui -lkio
#kcm_keys_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#
#AM_CPPFLAGS= $(all_includes)
#
#METASOURCES = AUTO
#
#messages:
#	customkeys=`grep "^.include .\.\." keyconfig.cpp | sed -e "s#.*\"\(.*\)\"#\1#"` ;\
#	$(XGETTEXT) *.cpp $$customkeys -o $(podir)/kcmkeys.pot
#
#update_DATA = kaccel.upd
#update_SCRIPTS = convertShortcuts.pl
#updatedir = $(kde_datadir)/kconf_update
#
#xdg_apps_DATA = keys.desktop
#
#scheme_DATA = wm3.kksrc kde3.kksrc kde4.kksrc win3.kksrc win4.kksrc mac4.kksrc unix3.kksrc
#schemedir = $(kde_datadir)/kcmkeys
#KDE_ICON=AUTO
