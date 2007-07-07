include_directories(${CMAKE_SOURCE_DIR}/workspace/libs)

set(solidnotifier_engine_SRCS
    solidnotifierengine.cpp
)

kde4_automoc(${solidnotifier_engine_SRCS})
kde4_add_plugin(plasma_engine_solidnotifier ${solidnotifier_engine_SRCS})
target_link_libraries(plasma_engine_solidnotifier ${KDE4_KDECORE_LIBS} plasma )

install(TARGETS plasma_engine_solidnotifier DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-solidnotifierengine.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES test-solid-notifier.desktop DESTINATION ${DATA_INSTALL_DIR}/solid/actions )