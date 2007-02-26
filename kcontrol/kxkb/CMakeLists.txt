add_subdirectory( pics ) 
#add_subdirectory( tests ) 

OPTION(USE_XKLAVIER "Use libxklavier to get keyboard layouts configuration" ON)

MESSAGE(STATUS "option: USE_XKLAVIER  <${USE_XKLAVIER}>")

if( USE_XKLAVIER STREQUAL ON )
  macro_optional_find_package(LibXKlavier)
  macro_optional_find_package(GLIB2)
  macro_optional_find_package(GObject)

  if(LIBXKLAVIER_FOUND AND GLIB2_FOUND AND GOBJECT_FOUND)

#MESSAGE("xklav:: ${LIBXKLAVIER_DEFINITIONS}")
#MESSAGE("xklav:: ${LIBXKLAVIER_LIBRARIES}")
	SET(CMAKE_REQUIRED_DEFINITIONS ${LIBXKLAVIER_DEFINITIONS})
	SET(CMAKE_REQUIRED_LIBRARIES ${LIBXKLAVIER_LIBRARIES})
	CHECK_FUNCTION_EXISTS(xkl_config_registry_set_custom_charset HAVE_SET_CUSTOM_CHARSET)

	SET(XKB_SUPPORT_SRC xklavier_adaptor.cpp)
	SET(XKB_SUPPORT_LIB ${LIBXKLAVIER_LIBRARIES} ${GOBJECT_LIBRARIES})
	ADD_DEFINITIONS(-DHAVE_XKLAVIER=1 -DHAVE_SET_CUSTOM_CHARSET=${HAVE_SET_CUSTOM_CHARSET} ${LIBXKLAVIER_DEFINITIONS} ${_LibGLIB2Cflags})
	include_directories(${GLIB2_INCLUDE_DIR})
  else(LIBXKLAVIER_FOUND AND GLIB2_FOUND AND GOBJECT_FOUND)
	MESSAGE ("Could not find glib, gobject or libxklavier >= 2.91 - old xkbfile code will be used to get keyboard layout configuration!")
  endif(LIBXKLAVIER_FOUND AND GLIB2_FOUND AND GOBJECT_FOUND)
endif( USE_XKLAVIER STREQUAL ON )

########### common sources ############

set(kxkb_COMMON_SRCS 
  rules.cpp
  kxkbconfig.cpp
  extension.cpp
  x11helper.cpp
  pixmap.cpp
  layoutmap.cpp
  ${XKB_SUPPORT_SRC}
)

########### KXKB kdeinit ###############

set(kxkb_KDEINIT_SRCS 
	${kxkb_COMMON_SRCS}
  kxkbcore.cpp
  kxkbapp.cpp
  kxkbwidget.cpp
  kxkb_adaptor.cpp
)

kde4_automoc(${kxkb_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kxkb ${kxkb_KDEINIT_SRCS})

target_link_libraries(kdeinit_kxkb ${X11_X11_LIB} ${X11_Xext_LIB} ${KDE4_KDEUI_LIBS} xkbfile ${XKB_SUPPORT_LIB})

install(TARGETS kdeinit_kxkb  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kxkb kdeinit_kxkb )
install(TARGETS kxkb DESTINATION ${BIN_INSTALL_DIR})

########### KCM ###############

set(kcm_keyboard_PART_SRCS 
  kcmlayout.cpp
  kcmmisc.cpp
  ${kxkb_COMMON_SRCS}
)

kde4_automoc(${kcm_keyboard_PART_SRCS})

kde4_add_ui_files(kcm_keyboard_PART_SRCS kcmlayoutwidget.ui kcmmiscwidget.ui )

kde4_add_plugin(kcm_keyboard ${kcm_keyboard_PART_SRCS})


target_link_libraries(kcm_keyboard
    ${KDE4_KIO_LIBS} xkbfile 
    ${XKB_SUPPORT_LIB} ${X11_XTest_LIB}
)

install(TARGETS kcm_keyboard  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### KXKB Applet ###############

set(kxkb_panelapplet_PART_SRCS
	kxkb_applet.cpp
	kxkbwidget.cpp
	kxkbcore.cpp
	${kxkb_COMMON_SRCS})

kde4_automoc(${kxkb_panelapplet_PART_SRCS})

kde4_add_plugin(kxkb_panelapplet ${kxkb_panelapplet_PART_SRCS})

target_link_libraries(kxkb_panelapplet kworkspace kickermain  
    ${KDE4_KIO_LIBS} xkbfile 
    ${XKB_SUPPORT_LIB} ${X11_XTest_LIB}
)

install(TARGETS kxkb_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES kxkb.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets)

########### install files ###############

install( FILES keyboard.desktop keyboard_layout.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kxkb.desktop  DESTINATION  ${XDG_APPS_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

if(NOT LIBXKLAVIER_FOUND OR NOT GLIB2_FOUND )
  install(FILES kxkb_groups DESTINATION ${CONFIG_INSTALL_DIR} )
endif(NOT LIBXKLAVIER_FOUND OR NOT GLIB2_FOUND)

