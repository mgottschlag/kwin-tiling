



########### next target ###############

set(kcminit_KDEINIT_SRCS main.cpp)


kde4_add_kdeinit_executable( kcminit ${kcminit_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcminit  ${KDE4_KUTILS_LIBS} )

install(TARGETS kdeinit_kcminit ${INSTALL_TARGETS_DEFAULT_ARGS} )

target_link_libraries( kcminit kdeinit_kcminit )
install(TARGETS kcminit DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

# TODO might be simpler to make <whatever>_startup to be a symlink to <whatever>

set(kcminit_startup_KDEINIT_SRCS main.cpp)


kde4_add_kdeinit_executable( kcminit_startup ${kcminit_startup_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcminit_startup  ${KDE4_KUTILS_LIBS} )

install(TARGETS kdeinit_kcminit_startup ${INSTALL_TARGETS_DEFAULT_ARGS} )

target_link_libraries( kcminit_startup kdeinit_kcminit_startup )
install(TARGETS kcminit_startup DESTINATION ${BIN_INSTALL_DIR})

