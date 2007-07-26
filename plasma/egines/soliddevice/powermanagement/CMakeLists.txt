include_directories(${CMAKE_SOURCE_DIR}/workspace/lib ${PLASMA_INCLUDE_DIR})

set(powermanagement_engine_SRCS
    powermanagementengine.cpp
)

kde4_add_plugin(plasma_engine_powermanagement ${powermanagement_engine_SRCS})
target_link_libraries(plasma_engine_powermanagement ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} ${PLASMA_LIBS} )

install(TARGETS plasma_engine_powermanagement DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-powermanagement.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

