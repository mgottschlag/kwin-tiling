########### next target ###############

set(kcminit_KDEINIT_SRCS main.cpp)


kde4_add_kdeinit_executable( kcminit ${kcminit_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcminit  ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES})

install(TARGETS kdeinit_kcminit ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS kcminit         ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### next target ###############

# TODO might be simpler to make <whatever>_startup to be a symlink to <whatever>

set(kcminit_startup_KDEINIT_SRCS main.cpp)


kde4_add_kdeinit_executable( kcminit_startup ${kcminit_startup_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcminit_startup  ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES})

install(TARGETS kdeinit_kcminit_startup ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS kcminit_startup         ${INSTALL_TARGETS_DEFAULT_ARGS} )

