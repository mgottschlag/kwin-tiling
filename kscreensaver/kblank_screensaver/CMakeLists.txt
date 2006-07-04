
project(kblankscrn)
include_directories( ${CMAKE_SOURCE_DIR}/workspace/kscreensaver/libkscreensaver   )


########### next target ###############

set(kblankscrn.kss_SRCS blankscrn.cpp )

kde4_automoc(${kblankscrn.kss_SRCS})

kde4_add_executable(kblankscrn.kss ${kblankscrn.kss_SRCS})

target_link_libraries(kblankscrn.kss  ${KDE4_KDEUI_LIBS} kscreensaver m )

install(TARGETS kblankscrn.kss DESTINATION bin)


########### install files ###############

install( FILES kblank.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/ScreenSavers )
