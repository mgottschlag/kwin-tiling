add_subdirectory( pics ) 
#add_subdirectory( tests ) 

OPTION(USE_XKLAVIER "Use libxklavier to get keyboard layouts configuration" ON)

MESSAGE(STATUS "option: USE_XKLAVIER  <${USE_XKLAVIER}>")

if( USE_XKLAVIER STREQUAL ON )
  macro_optional_find_package(LibXKlavier)
  macro_optional_find_package(GLIB2)

  if(LIBXKLAVIER_FOUND AND GLIB2_FOUND)

#MESSAGE("xklav:: ${LIBXKLAVIER_DEFINITIONS}")
#MESSAGE("xklav:: ${LIBXKLAVIER_LIBRARIES}")
	SET(CMAKE_REQUIRED_DEFINITIONS ${LIBXKLAVIER_DEFINITIONS})
	SET(CMAKE_REQUIRED_LIBRARIES ${LIBXKLAVIER_LIBRARIES})
	CHECK_FUNCTION_EXISTS(xkl_config_registry_set_custom_charset HAVE_SET_CUSTOM_CHARSET)

	SET(XKB_SUPPORT_SRC xklavier_adaptor.cpp)
	SET(XKB_SUPPORT_LIB ${LIBXKLAVIER_LIBRARIES} gobject-2.0)
	ADD_DEFINITIONS(-DHAVE_XKLAVIER=1 -DHAVE_SET_CUSTOM_CHARSET=${HAVE_SET_CUSTOM_CHARSET} ${LIBXKLAVIER_DEFINITIONS} ${_LibGLIB2Cflags})
	include_directories(${GLIB2_INCLUDE_DIR})
  else(LIBXKLAVIER_FOUND AND GLIB2_FOUND)
	MESSAGE ("Could not find glib or libxklavier >= 2.91 - old xkbfile code will be used to get keyboard layout configuration!")
  endif(LIBXKLAVIER_FOUND AND GLIB2_FOUND)
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
  kxkbwidget.cpp
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

target_link_libraries(kcm_keyboard
    ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} xkbfile 
    ${XKB_SUPPORT_LIB} ${X11_XTest_LIB}
)

install(TARGETS kcm_keyboard  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES keyboard.desktop keyboard_layout.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kxkb.desktop  DESTINATION  ${XDG_APPS_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )


if(NOT LIBXKLAVIER_FOUND OR NOT GLIB2_FOUND )
  install(FILES kxkb_groups DESTINATION ${CONFIG_INSTALL_DIR} )
endif(NOT LIBXKLAVIER_FOUND OR NOT GLIB2_FOUND)

