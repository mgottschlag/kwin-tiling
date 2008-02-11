add_subdirectory( pics )
#add_subdirectory( tests ) 
KDE4_NO_ENABLE_FINAL(kxkb)

OPTION(USE_XKLAVIER "Use libxklavier to get keyboard layouts configuration" ON)

MESSAGE(STATUS "option: USE_XKLAVIER  <${USE_XKLAVIER}>")

if( USE_XKLAVIER STREQUAL ON )

  macro_optional_find_package(LibXKlavier)
  macro_optional_find_package(GLIB2)
  macro_optional_find_package(GObject)

  if(LIBXKLAVIER_FOUND AND GLIB2_FOUND AND GOBJECT_FOUND)
    SET(HAVE_XKLAVIER TRUE)

    SET(CMAKE_REQUIRED_DEFINITIONS ${LIBXKLAVIER_DEFINITIONS})
    SET(CMAKE_REQUIRED_LIBRARIES ${LIBXKLAVIER_LIBRARIES})

    SET(XKB_SUPPORT_SRC xklavier_adaptor.cpp)
    SET(XKB_SUPPORT_LIB ${LIBXKLAVIER_LIBRARIES} ${GOBJECT_LIBRARIES})
    ADD_DEFINITIONS(-DHAVE_XKLAVIER=1 ${LIBXKLAVIER_DEFINITIONS} ${_LibGLIB2Cflags})
    include_directories(${GLIB2_INCLUDE_DIR})

  else(LIBXKLAVIER_FOUND AND GLIB2_FOUND AND GOBJECT_FOUND)
    MESSAGE (STATUS "kxkb: Could not find libxklavier >= 3.0, glib or gobject, will try xkbfile...")
  endif(LIBXKLAVIER_FOUND AND GLIB2_FOUND AND GOBJECT_FOUND)

endif( USE_XKLAVIER STREQUAL ON )

if( NOT HAVE_XKLAVIER )

    if (NOT X11_Xkbfile_FOUND)
        MESSAGE (STATUS "Could not find xkbfile - kxkb won't be built!")
        macro_log_feature (FALSE "libxklavier" "Could not find libxklavier or xkbfile - kxkb won't be built!" "http://freedesktop.org/wiki/Software/LibXklavier")
    else (NOT X11_Xkbfile_FOUND)
        SET(XKB_SUPPORT_LIB ${X11_Xkbfile_LIB})
        MESSAGE (STATUS "Using xkbfile for getting keyboard layouts is not well maintained, please consider installing libxklavier.")
        macro_log_feature (TRUE "libxklavier" "Using xkbfile for getting keyboard layouts is not well maintained, please consider installing libxklavier." "http://freedesktop.org/wiki/Software/LibXklavier")
    endif (NOT X11_Xkbfile_FOUND)

endif( NOT HAVE_XKLAVIER )


if (X11_Xkbfile_FOUND OR HAVE_XKLAVIER)

#MESSAGE("xkb support libs:: ${XKB_SUPPORT_LIB}")



########### common sources ############

set(kxkb_COMMON_SRCS 
  rules.cpp
  kxkbconfig.cpp
  extension.cpp
  x11helper.cpp
  pixmap.cpp
  ${XKB_SUPPORT_SRC}
)


########### KCM ###############

set(kcm_keyboard_layout_PART_SRCS 
  kcmlayout.cpp
  ${kxkb_COMMON_SRCS}
)

kde4_add_ui_files(kcm_keyboard_layout_PART_SRCS kcmlayoutwidget.ui )

kde4_add_plugin(kcm_keyboard_layout ${kcm_keyboard_layout_PART_SRCS})

target_link_libraries(kcm_keyboard_layout ${KDE4_KIO_LIBS}
    ${XKB_SUPPORT_LIB}
)

if(X11_XTest_FOUND)
  target_link_libraries(kcm_keyboard_layout ${X11_XTest_LIB})
endif(X11_XTest_FOUND)

install(TARGETS kcm_keyboard_layout DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES keyboard_layout.desktop DESTINATION  ${SERVICES_INSTALL_DIR} )


########### KXKB kdeinit ###############

set(kxkb_KDEINIT_SRCS ${kxkb_COMMON_SRCS}
  kxkbcore.cpp
  layoutmap.cpp
  kxkbapp.cpp
  kxkbwidget.cpp
  kxkb_adaptor.cpp
  kxkb_part.cpp
)

kde4_add_kdeinit_executable( kxkb ${kxkb_KDEINIT_SRCS})

target_link_libraries(kdeinit_kxkb ${X11_X11_LIB} ${X11_Xext_LIB} ${KDE4_KDEUI_LIBS}
     ${XKB_SUPPORT_LIB})

install(TARGETS kdeinit_kxkb  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kxkb kdeinit_kxkb )
install(TARGETS kxkb DESTINATION ${BIN_INSTALL_DIR})
#install(FILES kxkb_component.h DESTINATION ${INCLUDE_INSTALL_DIR})


########### KXKB Plasma Applet ###############

if( FALSE )     # don't generate applet - it's not finished yet

set(plasma_applet_kxkb_SRCS kxkb_applet.cpp ${kxkb_COMMON_SRCS}
     kxkbcore.cpp
     layoutmap.cpp
     kxkbwidget.cpp
)

kde4_add_plugin(plasma_applet_kxkb ${plasma_applet_kxkb_SRCS})

target_link_libraries(plasma_applet_kxkb plasma
    ${KDE4_KDEUI_LIBS} ${XKB_SUPPORT_LIB})
        
if(X11_XTest_FOUND)
    target_link_libraries(plasma_applet_kxkb ${X11_XTest_LIB})
endif(X11_XTest_FOUND)

install(TARGETS plasma_applet_kxkb  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES plasma-applet-kxkb.desktop  DESTINATION  ${SERVICES_INSTALL_DIR})

ADD_DEPENDENCIES(plasma_applet_kxkb kdeinit_kxkb)

endif (FALSE)

########### KXKB KPart ###############
# we're not using KPart for now - it's too heavy
#
#set(kxkb_part_PART_SRCS kxkb_part.cpp)
#
#kde4_add_plugin(kxkb_part ${kxkb_part_PART_SRCS})
#
#target_link_libraries(kxkb_part kdeinit_kxkb
#    ${KDE4_KIO_LIBS}
#    ${XKB_SUPPORT_LIB}
#)
#if(X11_XTest_FOUND)
#  target_link_libraries(kxkb_part ${X11_XTest_LIB})
#endif(X11_XTest_FOUND)
#
#install( TARGETS kxkb_part DESTINATION ${PLUGIN_INSTALL_DIR} )
#install( FILES kxkb_part.desktop DESTINATION  ${SERVICES_INSTALL_DIR} )
##install( FILES kxkb_part.rc DESTINATION  ${DATA_INSTALL_DIR}/kxkb_part )

endif (X11_Xkbfile_FOUND OR HAVE_XKLAVIER)
