include_directories( ${X11_Xrandr_INCLUDE_PATH}  )


########### next target ###############

set(randrinternal_PART_SRCS ktimerdialog.cpp randr.cpp)

set(kcm_randr_PART_SRCS krandrmodule.cpp ${randrinternal_PART_SRCS})

kde4_automoc(${kcm_randr_PART_SRCS})

kde4_add_plugin(kcm_randr ${kcm_randr_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_randr )

target_link_libraries(kcm_randr  ${KDE4_KDEUI_LIBS} ${X11_Xrandr_LIB} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})

install_targets(${PLUGIN_INSTALL_DIR} kcm_randr )


########### next target ###############


########### next target ###############

set(krandrtray_SRCS 
   main.cpp 
   krandrtray.cpp 
   krandrapp.cpp 
   krandrpassivepopup.cpp ${randrinternal_PART_SRCS})

kde4_automoc(${krandrtray_SRCS})

kde4_add_executable(krandrtray ${krandrtray_SRCS})

target_link_libraries(krandrtray  ${KDE4_KDECORE_LIBS} kutils ${X11_Xrandr_LIB} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS krandrtray DESTINATION bin)


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES krandrtray.desktop )
install_files( ${APPLNK_INSTALL_DIR}/.hidden FILES randr.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )




#original Makefile.am contents follow:

#AM_CPPFLAGS  = $(all_includes)
#
#lib_LTLIBRARIES = 
#kde_module_LTLIBRARIES = kcm_randr.la
#
#noinst_LTLIBRARIES = librandrinternal.la
#
#librandrinternal_la_SOURCES = ktimerdialog.cpp randr.cpp
#METASOURCES = AUTO 
#
#kcm_randr_la_SOURCES = krandrmodule.cpp
#kcm_randr_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
#kcm_randr_la_LIBADD = librandrinternal.la $(LIB_KDEUI) $(LIB_XRANDR)
#
#noinst_HEADERS = randr.h krandrmodule.h krandrtray.h krandrapp.h ktimerdialog.h \
#    krandrpassivepopup.h
#
#xdg_apps_DATA = krandrtray.desktop
#
#krandr_data_DATA = randr.desktop
#krandr_datadir = $(kde_appsdir)/.hidden
#
#KDE_ICON=AUTO
#
##install-data-local: uninstall.desktop
##	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Settings/Desktop
##	$(INSTALL_DATA) $(srcdir)/uninstall.desktop 
##	$(DESTDIR)$(kde_appsdir)/Settings/Desktop/krandrmodule.desktop
#
#bin_PROGRAMS = krandrtray
#
#krandrtray_SOURCES = main.cpp krandrtray.cpp krandrapp.cpp krandrpassivepopup.cpp
#krandrtray_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#krandrtray_LDADD = librandrinternal.la $(LIB_KFILE) $(LIB_KUTILS) $(LIB_XRANDR)
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/krandr.pot
