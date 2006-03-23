kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(kcm_kicker_PART_SRCS 
   positiontab_impl.cpp 
   hidingtab_impl.cpp 
   lookandfeeltab_impl.cpp 
   menutab_impl.cpp 
   extensionInfo.cpp 
   main.cpp 
   advancedDialog.cpp 
   positionconfig.cpp 
   hidingconfig.cpp 
   menuconfig.cpp 
   lookandfeelconfig.cpp )

kde4_automoc(${kcm_kicker_PART_SRCS})

kde4_add_ui3_files(kcm_kicker_PART_SRCS 
   positiontab.ui 
   hidingtab.ui 
   lookandfeeltab.ui 
   menutab.ui 
   advancedOptions.ui )

kde4_add_dcop_skels(kcm_kicker_PART_SRCS main.h )

kde4_add_plugin(kcm_kicker ${kcm_kicker_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kicker )

target_link_libraries(kcm_kicker  ${KDE4_KDECORE_LIBS} kickermain bgnd kio kutils )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kicker )


########### install files ###############

install_files( ${APPLNK_INSTALL_DIR}/.hidden FILES kicker_config.desktop                  kicker_config_arrangement.desktop kicker_config_hiding.desktop                  kicker_config_menus.desktop kicker_config_appearance.desktop )
install_files( ${XDG_APPS_DIR} FILES panel.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )

install(FILES uninstall.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}${APPLNK_INSTALL_DIR}/Settings/LookNFeel/ RENAME panel.desktop)
install(FILES uninstall.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}${APPLNK_INSTALL_DIR}/Settings/LookNFeel/ RENAME panel_appearance.desktop)
install(FILES uninstall.desktop DESTINATION ${XDG_APPS_DIR} RENAME panel_appearance.desktop)

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_kicker.la
#
#kcm_kicker_la_SOURCES = positiontab.ui hidingtab.ui lookandfeeltab.ui menutab.ui \
#			positiontab_impl.cpp hidingtab_impl.cpp lookandfeeltab_impl.cpp \
#			menutab_impl.cpp extensionInfo.cpp main.cpp main.skel \
#                        advancedOptions.ui advancedDialog.cpp \
#                        positionconfig.cpp hidingconfig.cpp menuconfig.cpp lookandfeelconfig.cpp
#
#kcm_kicker_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_kicker_la_LIBADD = $(top_builddir)/kicker/libkicker/libkickermain.la ../background/libbgnd.la $(LIB_KIO) $(LIB_KUTILS)
#AM_CPPFLAGS = -I$(top_srcdir)/kicker/kicker/core -I$(top_srcdir)/kicker/libkicker \
#	-I$(top_builddir)/kicker/libkicker -I$(srcdir)/../background $(all_includes)
#
#METASOURCES = AUTO
#
#noinst_HEADERS = positiontab_impl.h hidingtab_impl.h lookandfeeltab_impl.h menutab_impl.h \
#		extensionInfo.h main.h  advancedDialog.h
#
## Translation of tiles is used by lookandfeeltab_impl.cpp
## Tile names are transformed to words with title case
#### FIXME: Where are the tiles?
#messages: rc.cpp
#	true || (cd ../../../kicker/data/tiles ; ls *_tiny_up.png) | perl -p -e \
#	  's/(.*)_tiny_up\.png/i18n\(\"\u$$1\"\)\;/; s/[_ ]+(.)/ \u$$1/g' >> rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmkicker.pot
#
#xdg_apps_DATA = panel.desktop
#EXTRA_DIST = $(xdg_apps_DATA)
#
#kcmkicker_data3_DATA = kicker_config.desktop \
#                kicker_config_arrangement.desktop kicker_config_hiding.desktop \
#                kicker_config_menus.desktop kicker_config_appearance.desktop
#kcmkicker_data3dir = $(kde_appsdir)/.hidden
#
#install-data-local: uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel/panel.desktop
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel/panel_appearance.desktop
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(xdg_appsdir)/panel_appearance.desktop
#
#extensionInfo.lo: ../../kicker/libkicker/kickerSettings.h
#KDE_ICON=AUTO
