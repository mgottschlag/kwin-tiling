project(plasma-clock)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/workspace/libs)

set(clock_SRCS
    clock.cpp)

kde4_automoc(${clock_SRCS})
kde4_add_plugin(plasma_applet_clock ${clock_SRCS})
target_link_libraries(plasma_applet_clock plasma ${KDE4_KIO_LIBS})
set_target_properties(plasma_applet_clock PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS plasma_applet_clock DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-clock-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
