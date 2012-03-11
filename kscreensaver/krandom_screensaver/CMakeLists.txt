
include_directories( ${KDEBASE_WORKSPACE_SOURCE_DIR}/kscreensaver/libkscreensaver   )


########### next target ###############

set(krandom.kss_SRCS random.cpp)


kde4_add_executable(krandom.kss ${krandom.kss_SRCS})

if (WIN32)
  target_link_libraries(krandom.kss ${KDE4_KDEUI_LIBS})
else (WIN32)
  target_link_libraries(krandom.kss ${KDE4_KDEUI_LIBS} m)
endif (WIN32)

if (Q_WS_X11)
  target_link_libraries(krandom.kss ${X11_X11_LIB})
endif (Q_WS_X11)

install(TARGETS krandom.kss ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES krandom.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/ScreenSavers )

