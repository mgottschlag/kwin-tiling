
########### next target ###############

set(solid-network_SRCS solid-network.cpp)

kde4_add_executable(solid-network ${solid-network_SRCS})

target_link_libraries(solid-network ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} solidcontrol)

install(TARGETS solid-network ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############



