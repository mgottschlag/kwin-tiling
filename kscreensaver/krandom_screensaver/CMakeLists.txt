
project(krandom)
include_directories( ${CMAKE_SOURCE_DIR}/workspace/kscreensaver/libkscreensaver   )


########### next target ###############

set(krandom.kss_SRCS random.cpp )

kde4_automoc(${krandom.kss_SRCS})

kde4_add_executable(krandom.kss ${krandom.kss_SRCS})

target_link_libraries(krandom.kss  ${KDE4_KDEUI_LIBS}  m )

install(TARGETS krandom.kss DESTINATION bin)


########### install files ###############

install( FILES krandom.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/ScreenSavers )

