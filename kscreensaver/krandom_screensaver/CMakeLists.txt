
include_directories( ${KDEBASE_WORKSPACE_SOURCE_DIR}/kscreensaver/libkscreensaver   )


########### next target ###############

set(krandom.kss_SRCS random.cpp )


kde4_add_executable(krandom.kss ${krandom.kss_SRCS})

target_link_libraries(krandom.kss  ${KDE4_KDEUI_LIBS}  m )

install(TARGETS krandom.kss ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES krandom.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/ScreenSavers )

