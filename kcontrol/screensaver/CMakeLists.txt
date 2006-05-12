kde4_header()



########### next target ###############

set(kcm_screensaver_PART_SRCS 
   scrnsave.cpp 
   testwin.cpp 
   saverlist.cpp 
   saverconfig.cpp 
   advanceddialog.cpp 
   kswidget.cpp )

kde4_automoc(${kcm_screensaver_PART_SRCS})

kde4_add_ui3_files(kcm_screensaver_PART_SRCS advanceddialogimpl.ui )

kde4_add_plugin(kcm_screensaver ${kcm_screensaver_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_screensaver )

target_link_libraries(kcm_screensaver  ${KDE4_KDE3SUPPORT_LIBS})

install_targets(${PLUGIN_INSTALL_DIR} kcm_screensaver )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES screensaver.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#
#AM_CPPFLAGS = $(GLINC) $(all_includes)
#
#kde_module_LTLIBRARIES = kcm_screensaver.la
#
#kcm_screensaver_la_SOURCES = advanceddialogimpl.ui scrnsave.cpp testwin.cpp saverlist.cpp \
#        saverconfig.cpp advanceddialog.cpp kswidget.cpp
#kcm_screensaver_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
#kcm_screensaver_la_LIBADD = $(LIB_KSYCOCA) $(DPMSLIB) $(GLLIB)
#METASOURCES = AUTO
#
#noinst_HEADERS = scrnsave.h
#
#messages: rc.cpp
#	sed 's/X-KDE-Category=\(.*\)/i18n\("Screen saver category","\1"\);/' < category_list > category_list.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmscreensaver.pot
#	-rm -f category_list.cpp
#
#xdg_apps_DATA = screensaver.desktop
#
#KDE_ICON=AUTO
