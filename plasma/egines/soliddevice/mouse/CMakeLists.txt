include_directories(${CMAKE_SOURCE_DIR}/workspace/libs)

set(mouse_engine_SRCS
    mouseengine.cpp
)

kde4_add_plugin(plasma_engine_mouse ${mouse_engine_SRCS})
target_link_libraries(plasma_engine_mouse ${KDE4_KDEUI_LIBS} plasma)

install(TARGETS plasma_engine_mouse DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-mouse.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

