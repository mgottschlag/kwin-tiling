
########### next target ###############

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../control )

set(solid-network_SRCS solid-network.cpp)
set(solid-bluetooth_SRCS solid-bluetooth.cpp)
set(solid-powermanagement_SRCS solid-powermanagement.cpp)

kde4_add_executable(solid-network ${solid-network_SRCS})
kde4_add_executable(solid-bluetooth ${solid-bluetooth_SRCS})
kde4_add_executable(solid-powermanagement ${solid-powermanagement_SRCS})

target_link_libraries(solid-network ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} solidcontrol)
target_link_libraries(solid-bluetooth ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} solidcontrol)
target_link_libraries(solid-powermanagement ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} solidcontrol)

install(TARGETS solid-network DESTINATION ${BIN_INSTALL_DIR})
install(TARGETS solid-bluetooth DESTINATION ${BIN_INSTALL_DIR})
install(TARGETS solid-powermanagement DESTINATION ${BIN_INSTALL_DIR})

########### install files ###############



