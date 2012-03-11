
include_directories( ${KDEBASE_WORKSPACE_SOURCE_DIR}/kscreensaver/libkscreensaver   )


########### next target ###############

set(kblankscrn.kss_SRCS blankscrn.cpp )


kde4_add_executable(kblankscrn.kss ${kblankscrn.kss_SRCS})

if (WIN32)
  target_link_libraries(kblankscrn.kss  ${KDE4_KDEUI_LIBS} kscreensaver )
else (WIN32)
  target_link_libraries(kblankscrn.kss  ${KDE4_KDEUI_LIBS} kscreensaver m )
endif (WIN32)

install(TARGETS kblankscrn.kss ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES kblank.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/ScreenSavers )
