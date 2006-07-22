add_subdirectory( pics ) 
#add_subdirectory( tests ) 

OPTION(USE_XKLAVIER "Use libxklavier to get keyboard layouts configuration" ON)

MESSAGE(STATUS "option: USE_XKLAVIER  <${USE_XKLAVIER}>")

if( USE_XKLAVIER STREQUAL ON )
  macro_optional_find_package(LibXKlavier)
  #macro_optional_find_package(Glib2)

  if(LIBXKLAVIER_FOUND)

#MESSAGE("xklav:: ${LIBXKLAVIER_DEFINITIONS}")
#MESSAGE("xklav:: ${LIBXKLAVIER_LIBRARIES}")

	SET(XKB_SUPPORT_SRC xklavier_adaptor.cpp)
	SET(XKB_SUPPORT_LIB ${LIBXKLAVIER_LIBRARIES} gobject-2.0)
	ADD_DEFINITIONS(-DHAVE_XKLAVIER=1 ${LIBXKLAVIER_DEFINITIONS})
  else(LIBXKLAVIER_FOUND)
	MESSAGE ("Could not find glib or libxklavier - old xkbfile code will be used to get keyboard layout configuration!")
  endif(LIBXKLAVIER_FOUND)
endif( USE_XKLAVIER STREQUAL ON )


########### next target ###############

set(kxkb_KDEINIT_SRCS 
  extension.cpp
  x11helper.cpp
  rules.cpp
  kxkbconfig.cpp
  pixmap.cpp
  layoutmap.cpp
  kxkb.cpp
  kxkb_adaptor.cpp
  kxkbtraywindow.cpp
  ${XKB_SUPPORT_SRC}
)

kde4_automoc(${kxkb_KDEINIT_SRCS})


kde4_add_kdeinit_executable( kxkb ${kxkb_KDEINIT_SRCS})

target_link_libraries(kdeinit_kxkb  ${KDE4_KDECORE_LIBS} X11 Xext ${KDE4_KDEUI_LIBS} xkbfile ${XKB_SUPPORT_LIB})

install(TARGETS kdeinit_kxkb  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kxkb kdeinit_kxkb )
install(TARGETS kxkb DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############


set(kcm_keyboard_PART_SRCS 
  extension.cpp
  x11helper.cpp
  rules.cpp
  kxkbconfig.cpp
  pixmap.cpp
  kcmlayout.cpp
  kcmmisc.cpp
  ${XKB_SUPPORT_SRC}
)


kde4_automoc(${kcm_keyboard_PART_SRCS})

kde4_add_ui_files(kcm_keyboard_PART_SRCS kcmlayoutwidget.ui kcmmiscwidget.ui )

kde4_add_plugin(kcm_keyboard ${kcm_keyboard_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_keyboard )

target_link_libraries(kcm_keyboard  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} xkbfile ${XKB_SUPPORT_LIB})

install(TARGETS kcm_keyboard  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES keyboard.desktop keyboard_layout.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kxkb.desktop  DESTINATION  ${XDG_APPS_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )


if(NOT LIBXKLAVIER_FOUND)
  install(FILES kxkb_groups DESTINATION ${CONFIG_INSTALL_DIR} )
endif(NOT LIBXKLAVIER_FOUND)



#original Makefile.am contents follow:

#INCLUDES= $(all_includes)
#METASOURCES = AUTO
#SUBDIRS = pics
#
## it's basicly impossible to get this done with all the X headers used ;(
#KDE_OPTIONS = nofinal
#
#bin_PROGRAMS = 
#lib_LTLIBRARIES = 
#kdeinit_LTLIBRARIES = kxkb.la
#kde_module_LTLIBRARIES = kcm_keyboard.la
#
#kcm_keyboard_la_SOURCES = extension.cpp rules.cpp kcmlayout.cpp pixmap.cpp kcmlayoutwidget.ui kcmmisc.cpp kcmmiscwidget.ui
#kcm_keyboard_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_keyboard_la_LIBADD = -lxkbfile $(XTESTLIB) $(LIB_KIO)
#
##kxkb_la_includedir = .
##kxkb_la_include_HEADERS = kxkb.h
#kxkb_la_SOURCES = kxkb.cpp rules.cpp extension.cpp pixmap.cpp kxkb.skel
#kxkb_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kxkb_la_LIBADD = -lxkbfile -lX11 -lXext $(LIB_KDEUI)
#
#xdg_apps_DATA = keyboard.desktop keyboard_layout.desktop
#
#install-data-local: 
#	$(mkinstalldirs) $(DESTDIR)$(kde_confdir)
#	$(INSTALL_DATA) $(srcdir)/kxkb_groups $(DESTDIR)$(kde_confdir)/kxkb_groups
#
#uninstall-local:
#	-rm -f $(DESTDIR)$(kde_confdir)/kxkb_groups
#
#servicesdir = $(kde_servicesdir)
#services_DATA = kxkb.desktop
#
#messages: rc.cpp
#	$(XGETTEXT) kxkb.cpp rules.cpp extension.cpp pixmap.cpp kxkbbindings.cpp -o $(podir)/kxkb.pot
#	$(XGETTEXT) rules.cpp kcmlayout.cpp pixmap.cpp kcmmisc.cpp rc.cpp kxkbbindings.cpp -o $(podir)/kcmlayout.pot 
#
#KDE_ICON=AUTO
