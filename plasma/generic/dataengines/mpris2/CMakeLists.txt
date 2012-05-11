cmake_minimum_required(VERSION 2.6)

project(plasma-dataengine-mpris2)

set(QT_MIN_VERSION "4.6.0")
set(KDE_MIN_VERSION "4.6.0")
find_package(KDE4 ${KDE_MIN_VERSION} REQUIRED)
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

set(mpris2_engine_SRCS
    mpris2engine.cpp
    multiplexer.cpp
    multiplexedservice.cpp
    playercontrol.cpp
    playeractionjob.cpp
    playercontainer.cpp
)

kde4_add_plugin(plasma_engine_mpris2 ${mpris2_engine_SRCS})
target_link_libraries(plasma_engine_mpris2
   ${KDE4_PLASMA_LIBS}
)

install(TARGETS plasma_engine_mpris2 DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-mpris2.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES mpris2.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

